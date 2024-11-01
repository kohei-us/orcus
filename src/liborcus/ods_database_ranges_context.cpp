/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ods_database_ranges_context.hpp"
#include "odf_token_constants.hpp"
#include "odf_namespace_types.hpp"
#include "impl_utils.hpp"

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/spreadsheet/import_interface_auto_filter.hpp>
#include <orcus/measurement.hpp>
#include <mdds/sorted_string_map.hpp>

#include <sstream>

namespace ss = orcus::spreadsheet;

namespace orcus {

namespace {

namespace field_op {

using map_type = mdds::sorted_string_map<ss::auto_filter_op_t>;

// Keys must be sorted.
constexpr map_type::entry_type entries[] = {
    { "!=", ss::auto_filter_op_t::not_equal },
    { "!begins", ss::auto_filter_op_t::not_begin_with },
    { "!contains", ss::auto_filter_op_t::not_contain },
    { "!empty", ss::auto_filter_op_t::not_empty },
    { "!ends", ss::auto_filter_op_t::not_end_with },
    { "<", ss::auto_filter_op_t::less },
    { "<=", ss::auto_filter_op_t::less_equal },
    { "=", ss::auto_filter_op_t::equal },
    { ">", ss::auto_filter_op_t::greater },
    { ">=", ss::auto_filter_op_t::greater_equal },
    { "begins", ss::auto_filter_op_t::begin_with },
    { "bottom percent", ss::auto_filter_op_t::bottom_percent },
    { "bottom values", ss::auto_filter_op_t::bottom },
    { "contains", ss::auto_filter_op_t::contain },
    { "empty", ss::auto_filter_op_t::empty },
    { "ends", ss::auto_filter_op_t::end_with },
    { "top percent", ss::auto_filter_op_t::top_percent },
    { "top values", ss::auto_filter_op_t::top },
};

const map_type& get()
{
    static const map_type mt(entries, std::size(entries), ss::auto_filter_op_t::unspecified);
    return mt;
}

} // namespace field_op

} // anonymous namespace

ods_database_ranges_context::ods_database_ranges_context(
    session_context& session_cxt, const tokens& tokens, ss::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_odf_table, XML_database_ranges }, // root element
        { NS_odf_table, XML_database_range, NS_odf_table, XML_filter },
        { NS_odf_table, XML_database_ranges, NS_odf_table, XML_database_range },
        { NS_odf_table, XML_filter, NS_odf_table, XML_filter_and },
        { NS_odf_table, XML_filter, NS_odf_table, XML_filter_or },
        { NS_odf_table, XML_filter_and, NS_odf_table, XML_filter_condition },
        { NS_odf_table, XML_filter_and, NS_odf_table, XML_filter_or },
        { NS_odf_table, XML_filter_or, NS_odf_table, XML_filter_and },
        { NS_odf_table, XML_filter_or, NS_odf_table, XML_filter_condition },
    };

    init_element_validator(rules, std::size(rules));
}

ods_database_ranges_context::~ods_database_ranges_context() = default;

void ods_database_ranges_context::start_element(
    xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
{
    push_stack(ns, name);

    if (ns != NS_odf_table)
        warn_unexpected();

    switch (name)
    {
        case XML_database_ranges:
            break;
        case XML_database_range:
            start_database_range(attrs);
            break;
        case XML_filter:
            start_filter(attrs);
            break;
        case XML_filter_or:
            start_filter_node(ss::auto_filter_node_op_t::op_or);
            break;
        case XML_filter_and:
            start_filter_node(ss::auto_filter_node_op_t::op_and);
            break;
        case XML_filter_condition:
            start_filter_condition(attrs);
            break;
        default:
            warn_unexpected();
    }
}

bool ods_database_ranges_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_odf_table)
    {
        switch (name)
        {
            case XML_database_range:
                end_database_range();
                break;
            case XML_filter:
                end_filter();
                break;
            case XML_filter_or:
                end_filter_node();
                break;
            case XML_filter_and:
                end_filter_node();
                break;
        }
    }
    return pop_stack(ns, name);
}

void ods_database_ranges_context::reset()
{
    mp_table = nullptr;
    mp_filter = nullptr;

    m_target_range.reset();
    m_filter_node_stack.clear();
}

void ods_database_ranges_context::start_database_range(const xml_token_attrs_t& attrs)
{
    if (!mp_factory)
        return;

    auto resolver = mp_factory->get_reference_resolver(ss::formula_ref_context_t::table_range);
    if (!resolver)
        return;

    std::string_view name;
    std::optional<ss::src_range_t> target_range;

    for (const auto& attr : attrs)
    {
        if (attr.ns != NS_odf_table)
            continue;

        switch (attr.name)
        {
            case XML_name:
                name = attr.value;
                break;
            case XML_target_range_address:
                target_range = resolver->resolve_range(attr.value);
                break;
        }
    }

    if (!target_range)
        return;

    if (name.empty())
        return;

    auto* sh = mp_factory->get_sheet(target_range->first.sheet); // Just use the first sheet for now
    if (!sh)
        return;

    if (target_range->last.sheet > target_range->first.sheet)
    {
        std::ostringstream os;
        os << "database range named '" << name << "' spans across multiple sheets, but only the first sheet is used";
        warn(os.str());
    }

    mp_table = sh->start_table();
    if (!mp_table)
        return;

    m_target_range = to_rc_range(*target_range);

    mp_table->set_name(name);
    mp_table->set_range(*m_target_range);
}

void ods_database_ranges_context::end_database_range()
{
    if (!mp_table)
        return;

    mp_table->commit();
    mp_table = nullptr;
}

void ods_database_ranges_context::start_filter(const xml_token_attrs_t& /*attrs*/)
{
    if (!mp_table || !m_target_range)
        return;

    mp_filter = mp_table->start_auto_filter(*m_target_range);
}

void ods_database_ranges_context::end_filter()
{
    if (!mp_filter)
        return;

    assert(m_filter_node_stack.empty());
    mp_filter->commit();
    mp_filter = nullptr;
}

void ods_database_ranges_context::start_filter_condition(const xml_token_attrs_t& attrs)
{
    if (!mp_filter || m_filter_node_stack.empty())
        return;

    auto* node = m_filter_node_stack.back();

    bool numeric_value = true; // probably numeric by default if not specified (?)
    std::string_view value;
    ss::auto_filter_op_t op = ss::auto_filter_op_t::unspecified;
    std::optional<ss::col_t> field;

    for (const auto& attr : attrs)
    {
        if (attr.ns != NS_odf_table)
            continue;

        switch (attr.name)
        {
            case XML_data_type:
            {
                if (attr.value == "number")
                    numeric_value = true;
                else if (attr.value == "text")
                    numeric_value = false;
                else
                {
                    std::ostringstream os;
                    os << "invalid value in 'table:data-type': " << attr.value;
                    warn(os.str());
                }
                break;
            }
            case XML_value:
            {
                value = attr.value;
                break;
            }
            case XML_operator:
            {
                op = field_op::get().find(attr.value);

                if (op == ss::auto_filter_op_t::unspecified)
                {
                    std::ostringstream os;
                    os << "invalid field operator value '" << attr.value << "'";
                    warn(os.str());
                    return;
                }
                break;
            }
            case XML_field_number:
            {
                field = to_long_checked(attr.value);
                if (!field)
                {
                    std::ostringstream os;
                    os << "invalid field value '" << attr.value << "'";
                    warn(os.str());
                    return;
                }
                break;
            }
        }
    }

    if (!field)
    {
        warn("required 'field' attribute was not provided");
        return;
    }

    assert(op != ss::auto_filter_op_t::unspecified);

    switch (op)
    {
        case ss::auto_filter_op_t::greater:
        case ss::auto_filter_op_t::greater_equal:
        case ss::auto_filter_op_t::less:
        case ss::auto_filter_op_t::less_equal:
        {
            if (!numeric_value)
            {
                warn("field operator requires numeric value, but the value is not numeric");
                return;
            }

            auto v = to_double_checked(value);
            if (!v)
            {
                std::ostringstream os;
                os << "failed to convert field value to a numeric value: '" << value << "'";
                warn(os.str());
                return;
            }

            node->append_item(*field, op, *v);
            break;
        }
        default:
            warn("TODO: handle this");
    }
}

void ods_database_ranges_context::start_filter_node(ss::auto_filter_node_op_t node_op)
{
    if (!mp_filter)
        return;

    ss::iface::import_auto_filter_node* node = nullptr;

    if (m_filter_node_stack.empty())
        node = mp_filter->start_node(node_op);
    else
        node = m_filter_node_stack.back()->start_node(node_op);

    ENSURE_INTERFACE(node, import_auto_filter_node);
    m_filter_node_stack.push_back(node);
}

void ods_database_ranges_context::end_filter_node()
{
    if (m_filter_node_stack.empty())
        return;

    m_filter_node_stack.back()->commit();
    m_filter_node_stack.pop_back();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

