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

#include <iostream>
#include <optional>

namespace ss = orcus::spreadsheet;

namespace orcus {

namespace {

enum gnumeric_filter_field_type_t
{
    filter_expr,
    filter_blanks,
    filter_nonblanks,
    filter_type_invalid
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
    mp_node = nullptr;
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

    mp_node = mp_auto_filter->start_node(ss::auto_filter_node_op_t::op_and);
    ENSURE_INTERFACE(mp_node, import_auto_filter_node);
}

void gnumeric_filter_context::start_field(const xml_token_attrs_t& attrs)
{
    if (!mp_node)
        return;

    gnumeric_filter_field_type_t filter_field_type = filter_type_invalid;
    ss::auto_filter_op_t filter_op = ss::auto_filter_op_t::unspecified;

    ss::col_t col = -1;
    // NB: due to a bug in gnumeric, value and value type attributes are swapped
    std::optional<long> filter_value_type;
    std::string_view filter_value;

    for (const xml_token_attr_t& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_Index:
            {
                col = to_long(attr.value.data());
                break;
            }
            case XML_Type:
            {
                if (attr.value == "expr")
                    filter_field_type = filter_expr;
                else if (attr.value == "blanks")
                    filter_field_type = filter_blanks;
                else if (attr.value == "nonblanks")
                    filter_field_type = filter_nonblanks;
                break;
            }
            case XML_Op0:
            {
                if (attr.value == "eq")
                    filter_op = ss::auto_filter_op_t::equal;
                else if (attr.value == "gt")
                    filter_op = ss::auto_filter_op_t::greater;
                else if (attr.value == "lt")
                    filter_op = ss::auto_filter_op_t::less;
                else if (attr.value == "gte")
                    filter_op = ss::auto_filter_op_t::greater_equal;
                else if (attr.value == "lte")
                    filter_op = ss::auto_filter_op_t::less_equal;
                else if (attr.value == "ne")
                    filter_op = ss::auto_filter_op_t::not_equal;
                break;
            }
            case XML_Value0:
            {
                filter_value_type = to_long_checked(attr.value);
                break;
            }
            case XML_ValueType0:
            {
                filter_value = attr.value;
                break;
            }
        }
    }

    if (col < 0)
    {
        warn("valid field index value was not found in the 'Index' attribute of 'Filter' element");
        return;
    }

    if (!filter_value_type)
    {
        warn("valid filter value type was not found");
        return;
    }

    switch (filter_field_type)
    {
        case filter_expr:
        {
            // see GnmValueType in gnumeric code for these magic values

            switch (*filter_value_type)
            {
                case 10:
                    // empty
                    warn("empty filter value type is not yet handled");
                    break;
                case 20:
                {
                    // boolean
                    bool v = to_bool(filter_value);
                    mp_node->append_item(col, filter_op, v ? 1 : 0);
                    break;
                }
                case 40:
                {
                    // float
                    auto v = to_double_checked(filter_value);
                    if (!v)
                    {
                        std::ostringstream os;
                        os << "numeric filter value was expected but failed to convert to numeric value: " << filter_value;
                        warn(os.str());
                        break;
                    }
                    mp_node->append_item(col, filter_op, *v);
                    break;
                }
                case 50:
                    // error
                    warn("error filter value type is not yet handled");
                    break;
                case 60:
                    // string
                    mp_node->append_item(col, filter_op, filter_value, false);
                    break;
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
                    os << "unhandled fitler value type (" << *filter_value_type << ")";
                    warn(os.str());
                }
            }
            break;
        }
        case filter_blanks:
            mp_node->append_item(col, ss::auto_filter_op_t::empty, 0);
            break;
        case filter_nonblanks:
            mp_node->append_item(col, ss::auto_filter_op_t::not_empty, 0);
            break;
        case filter_type_invalid:
            break;
    }
}

void gnumeric_filter_context::end_filter()
{
    if (mp_node)
        mp_node->commit();

    mp_node = nullptr;

    if (mp_auto_filter)
        mp_auto_filter->commit();

    mp_auto_filter = nullptr;
}

void gnumeric_filter_context::end_field()
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
