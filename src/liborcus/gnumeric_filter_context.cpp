/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gnumeric_filter_context.hpp"
#include "gnumeric_token_constants.hpp"
#include "gnumeric_namespace_types.hpp"
#include "impl_utils.hpp"

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/spreadsheet/import_interface_auto_filter.hpp>
#include <orcus/measurement.hpp>

#include <mdds/sorted_string_map.hpp>

#include <iostream>
#include <optional>

namespace ss = orcus::spreadsheet;

namespace orcus {

namespace {

enum class gnumeric_filter_field_type_t
{
    invalid,
    expr,
    blanks,
    noblanks,
    bucket,
};

namespace field_type {

using map_type = mdds::sorted_string_map<gnumeric_filter_field_type_t>;

// Keys must be sorted.
constexpr map_type::entry_type entries[] = {
    { "blanks", gnumeric_filter_field_type_t::blanks },
    { "bucket", gnumeric_filter_field_type_t::bucket },
    { "expr", gnumeric_filter_field_type_t::expr },
    { "noblanks", gnumeric_filter_field_type_t::noblanks },
};

const map_type& get()
{
    static const map_type mt(entries, std::size(entries), gnumeric_filter_field_type_t::invalid);
    return mt;
}

} // field_type namespace

namespace op {

using map_type = mdds::sorted_string_map<ss::auto_filter_op_t>;

// Keys must be sorted.
constexpr map_type::entry_type entries[] = {
    { "eq", ss::auto_filter_op_t::equal },
    { "gt", ss::auto_filter_op_t::greater },
    { "gte", ss::auto_filter_op_t::greater_equal },
    { "lt", ss::auto_filter_op_t::less },
    { "lte", ss::auto_filter_op_t::less_equal },
    { "ne", ss::auto_filter_op_t::not_equal },
};

const map_type& get()
{
    static const map_type mt(entries, std::size(entries), ss::auto_filter_op_t::unspecified);
    return mt;
}

} // op namespace

class scoped_child_node
{
    std::vector<ss::iface::import_auto_filter_node*>& m_node_stack;

public:
    scoped_child_node(
        std::vector<ss::iface::import_auto_filter_node*>& node_stack,
        ss::auto_filter_node_op_t connector) :
        m_node_stack(node_stack)
    {
        auto* node = m_node_stack.back()->start_node(connector);
        ENSURE_INTERFACE(node, import_auto_filter_node);
        m_node_stack.push_back(node);
    }

    ~scoped_child_node()
    {
        assert(!m_node_stack.empty());
        m_node_stack.back()->commit();
        m_node_stack.pop_back();
    }
};

} // anonymous namespace

gnumeric_filter_context::gnumeric_filter_context(
    session_context& session_cxt, const tokens& tokens,
    ss::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_gnumeric_gnm, XML_Filter }, // root element
        { NS_gnumeric_gnm, XML_Filter, NS_gnumeric_gnm, XML_Field },
    };

    init_element_validator(rules, std::size(rules));
}

gnumeric_filter_context::~gnumeric_filter_context() = default;

void gnumeric_filter_context::start_element(
    xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs)
{
    push_stack(ns, name);

    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_Filter:
            {
                start_filter(attrs);
                break;
            }
            case XML_Field:
            {
                start_field(attrs);
                break;
            }
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool gnumeric_filter_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_Filter:
                end_filter();
                break;
            case XML_Field:
                end_field();
                break;
        }
    }

    return pop_stack(ns, name);
}

void gnumeric_filter_context::reset(spreadsheet::iface::import_sheet* sheet)
{
    mp_sheet = sheet;
    mp_auto_filter = nullptr;
    m_node_stack.clear();
}

void gnumeric_filter_context::start_filter(const xml_token_attrs_t& attrs)
{
    if (!mp_sheet)
        return;

    ss::iface::import_reference_resolver* resolver =
        mp_factory->get_reference_resolver(ss::formula_ref_context_t::global);

    if (!resolver)
        return;

    std::optional<spreadsheet::range_t> area;

    for (const xml_token_attr_t& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_Area:
                area = to_rc_range(resolver->resolve_range(attr.value));
                break;
            default:
                ;
        }
    }

    if (!area)
        return;

    mp_auto_filter = mp_sheet->start_auto_filter(*area);
    if (!mp_auto_filter)
        return;

    auto node = mp_auto_filter->start_node(ss::auto_filter_node_op_t::op_and);
    ENSURE_INTERFACE(node, import_auto_filter_node);
    m_node_stack.push_back(node);
}

void gnumeric_filter_context::start_field(const xml_token_attrs_t& attrs)
{
    if (m_node_stack.empty())
        return;

    gnumeric_filter_field_type_t filter_field_type = gnumeric_filter_field_type_t::invalid;

    ss::col_t field = -1;

    // NB: due to a bug in gnumeric, value and value type attributes are swapped
    std::optional<long> filter_value_type0;
    std::string_view filter_value0;
    ss::auto_filter_op_t filter_op0 = ss::auto_filter_op_t::unspecified;

    std::optional<long> filter_value_type1;
    std::string_view filter_value1;
    ss::auto_filter_op_t filter_op1 = ss::auto_filter_op_t::unspecified;

    std::optional<ss::auto_filter_node_op_t> connector;

    std::optional<bool> top;
    std::optional<bool> items;
    std::optional<bool> rel_range;
    std::optional<double> count;

    for (const xml_token_attr_t& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_Index:
            {
                field = to_long(attr.value.data());
                break;
            }
            case XML_Type:
            {
                filter_field_type = field_type::get().find(attr.value);

                if (filter_field_type == gnumeric_filter_field_type_t::invalid)
                {
                    std::ostringstream os;
                    os << "invalid filter field type: " << attr.value;
                    warn(os.str());
                    return;
                }
                break;
            }
            case XML_Op0:
            {
                filter_op0 = op::get().find(attr.value);
                if (filter_op0 == ss::auto_filter_op_t::unspecified)
                {
                    std::ostringstream os;
                    os << "invalid filter operator: '" << attr.value << "'";
                    warn(os.str());
                    return;
                }
                break;
            }
            case XML_Op1:
            {
                filter_op1 = op::get().find(attr.value);
                if (filter_op1 == ss::auto_filter_op_t::unspecified)
                {
                    std::ostringstream os;
                    os << "invalid filter operator: '" << attr.value << "'";
                    warn(os.str());
                    return;
                }
                break;
            }
            case XML_Value0:
            {
                filter_value_type0 = to_long_checked(attr.value);
                break;
            }
            case XML_Value1:
            {
                filter_value_type1 = to_long_checked(attr.value);
                break;
            }
            case XML_ValueType0:
            {
                filter_value0 = attr.value;
                break;
            }
            case XML_ValueType1:
            {
                filter_value1 = attr.value;
                break;
            }
            case XML_IsAnd:
            {
                bool b = to_bool(attr.value);
                connector = b ? ss::auto_filter_node_op_t::op_and : ss::auto_filter_node_op_t::op_or;
                break;
            }
            case XML_top:
            {
                top = to_bool(attr.value);
                break;
            }
            case XML_items:
            {
                items = to_bool(attr.value);
                break;
            }
            case XML_rel_range:
            {
                rel_range = to_bool(attr.value);
                break;
            }
            case XML_count:
            {
                count = to_double_checked(attr.value);
                break;
            }
        }
    }

    if (field < 0)
    {
        warn("valid field index value was not found in the 'Index' attribute of 'Filter' element");
        return;
    }

    switch (filter_field_type)
    {
        case gnumeric_filter_field_type_t::expr:
        {
            // see GnmValueType in gnumeric code for these magic values
            push_expression_field(
                field, filter_op0, filter_value_type0, filter_value0, filter_op1,
                filter_value_type1, filter_value1, connector);
            break;
        }
        case gnumeric_filter_field_type_t::blanks:
            m_node_stack.back()->append_item(field, ss::auto_filter_op_t::empty, 0);
            break;
        case gnumeric_filter_field_type_t::noblanks:
            m_node_stack.back()->append_item(field, ss::auto_filter_op_t::not_empty, 0);
            break;
        case gnumeric_filter_field_type_t::bucket:
        {
            push_bucket_field(field, top, rel_range, items, count);
            break;
        }
        case gnumeric_filter_field_type_t::invalid:
            warn("filter field type is invalid without early bail-out");
            break;
    }
}

void gnumeric_filter_context::end_filter()
{
    if (!m_node_stack.empty())
    {
        m_node_stack.back()->commit();
        m_node_stack.pop_back();
    }

    if (!m_node_stack.empty())
        warn("node stack was expected to be empty when the Filter scope ends, but it isn't");

    if (mp_auto_filter)
        mp_auto_filter->commit();

    mp_auto_filter = nullptr;
}

void gnumeric_filter_context::end_field()
{
}

void gnumeric_filter_context::push_expression_field(
    ss::col_t field, ss::auto_filter_op_t op0, std::optional<long> value_type0,
    std::string_view value0, ss::auto_filter_op_t op1, std::optional<long> value_type1,
    std::string_view value1, std::optional<ss::auto_filter_node_op_t> connector)
{
    assert(field >= 0);
    assert(!m_node_stack.empty());

    if (op0 == ss::auto_filter_op_t::unspecified)
    {
        warn("no valid operator found in rule 1");
        return;
    }

    if (!value_type0)
    {
        warn("no valid value type found in rule 1");
        return;
    }

    if (!connector)
    {
        // this is a single-rule field
        push_field_rule(field, op0, *value_type0, value0);
        return;
    }

    // This is a compound rule.  Import it as a node with two child rules.
    scoped_child_node node_pop(m_node_stack, *connector);

    push_field_rule(field, op0, *value_type0, value0);

    if (op1 == ss::auto_filter_op_t::unspecified)
    {
        warn("no valid operator found in rule 2");
        return;
    }

    if (!value_type1)
    {
        warn("no valid value type found in rule 2");
        return;
    }

    push_field_rule(field, op1, *value_type1, value1);
}

void gnumeric_filter_context::push_field_rule(
    spreadsheet::col_t field, spreadsheet::auto_filter_op_t op, long value_type,
    std::string_view value)
{
    assert(!m_node_stack.empty());

    switch (value_type)
    {
        case 10:
            // empty
            warn("empty filter value type is not yet handled");
            break;
        case 20:
        {
            // boolean
            bool v = to_bool(value);
            m_node_stack.back()->append_item(field, op, v ? 1 : 0);
            break;
        }
        case 40:
        {
            // float
            auto v = to_double_checked(value);
            if (!v)
            {
                std::ostringstream os;
                os << "numeric filter value was expected but failed to convert to numeric value: " << value;
                warn(os.str());
                break;
            }
            m_node_stack.back()->append_item(field, op, *v);
            break;
        }
        case 50:
            // error
            warn("error filter value type is not yet handled");
            break;
        case 60:
        {
            // string
            auto res = m_value_parser.parse(op, value);
            m_node_stack.back()->append_item(field, res.op, res.value, res.regex);
            break;
        }
        case 70:
            // cell range
            warn("cell-range filter value type is not yet handled");
            break;
        case 80:
            // array
            warn("array filter value type is not yet handled");
            break;
        default:
        {
            std::ostringstream os;
            os << "unhandled filter value type (" << value_type << ")";
            warn(os.str());
        }
    }
}

void gnumeric_filter_context::push_bucket_field(
    ss::col_t field, std::optional<bool> top, std::optional<bool> rel_range,
    std::optional<bool> items, std::optional<double> count)
{
    assert(field >= 0);
    assert(!m_node_stack.empty());

    if (!top)
    {
        warn("bucket filter type with no 'top' boolean attribute given");
        return;
    }

    if (!count)
    {
        warn("bucket filter type with no 'count' numeric attribute given");
        return;
    }

    //  items | rel_range | interpretation
    // -------+-----------+----------------------
    //  true  | -         | top N items
    //  false | true      | top N% of data range
    //  false | false     | top N% of all items

    if (items && *items)
    {
        // top N items
        m_node_stack.back()->append_item(
            field, *top ? ss::auto_filter_op_t::top : ss::auto_filter_op_t::bottom, *count);
        return;
    }

    if (rel_range && *rel_range)
    {
        // top N% of data range
        m_node_stack.back()->append_item(
            field,
            *top ? ss::auto_filter_op_t::top_percent_range : ss::auto_filter_op_t::bottom_percent_range,
            *count);
        return;
    }

    // top N% of items
    m_node_stack.back()->append_item(
        field, *top ? ss::auto_filter_op_t::top_percent : ss::auto_filter_op_t::bottom_percent, *count);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
