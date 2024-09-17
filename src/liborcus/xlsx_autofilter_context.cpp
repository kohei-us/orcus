/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xlsx_autofilter_context.hpp"
#include "xml_context_global.hpp"
#include "ooxml_namespace_types.hpp"
#include "ooxml_token_constants.hpp"
#include "impl_utils.hpp"

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/spreadsheet/import_interface_auto_filter.hpp>
#include <orcus/measurement.hpp>

#include <mdds/sorted_string_map.hpp>

#include <iostream>

namespace ss = orcus::spreadsheet;

namespace orcus {

namespace {

namespace filterop {

using map_type = mdds::sorted_string_map<ss::auto_filter_op_t>;

// Keys must be sorted.
constexpr map_type::entry_type entries[] = {
    { "equal", ss::auto_filter_op_t::equal },
    { "greaterThan", ss::auto_filter_op_t::greater },
    { "greaterThanOrEqual", ss::auto_filter_op_t::greater_equal },
    { "lessThan", ss::auto_filter_op_t::less },
    { "lessThanOrEqual", ss::auto_filter_op_t::less_equal },
    { "notEqual", ss::auto_filter_op_t::not_equal },
};

const map_type& get()
{
    static const map_type mt(entries, std::size(entries), ss::auto_filter_op_t::unspecified);
    return mt;
}

} // namespace filterop

}

xlsx_autofilter_context::xlsx_autofilter_context(
    session_context& session_cxt, const tokens& tokens,
    spreadsheet::iface::import_reference_resolver& resolver) :
    xml_context_base(session_cxt, tokens),
    m_resolver(resolver),
    m_cur_col(-1)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_ooxml_xlsx, XML_autoFilter }, // root element
        { NS_ooxml_xlsx, XML_autoFilter, NS_ooxml_xlsx, XML_filterColumn },
        { NS_ooxml_xlsx, XML_filterColumn, NS_ooxml_xlsx, XML_filters },
        { NS_ooxml_xlsx, XML_filterColumn, NS_ooxml_xlsx, XML_customFilters },
        { NS_ooxml_xlsx, XML_filterColumn, NS_ooxml_xlsx, XML_top10 },
        { NS_ooxml_xlsx, XML_filterColumn, NS_ooxml_xlsx, XML_dynamicFilter },
        { NS_ooxml_xlsx, XML_customFilters, NS_ooxml_xlsx, XML_customFilter },
        { NS_ooxml_xlsx, XML_filters, NS_ooxml_xlsx, XML_filter },
    };

    init_element_validator(rules, std::size(rules));
}

xlsx_autofilter_context::~xlsx_autofilter_context() = default;

void xlsx_autofilter_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
{
    push_stack(ns, name);

    if (ns != NS_ooxml_xlsx)
        return;

    switch (name)
    {
        case XML_autoFilter:
        {
            start_auto_filter(attrs);
            break;
        }
        case XML_filterColumn:
        {
            start_filter_column(attrs);
            break;
        }
        case XML_customFilters:
        {
            start_custom_filters(attrs);
            break;
        }
        case XML_customFilter:
        {
            start_custom_filter(attrs);
            break;
        }
        case XML_filters:
            // ignore this
            break;
        case XML_filter:
        {
            warn("TODO: handle this");
            break;
        }
        default:
            warn_unhandled();
    }
}

bool xlsx_autofilter_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_ooxml_xlsx)
    {
        switch (name)
        {
            case XML_autoFilter:
            {
                end_auto_filter();
                break;
            }
            case XML_filterColumn:
            {
                end_filter_column();
                break;
            }
            case XML_customFilters:
            {
                end_custom_filters();
                break;
            }
            case XML_customFilter:
            {
                end_custom_filter();
                break;
            }
            default:
                ;
        }
    }
    return pop_stack(ns, name);
}

void xlsx_autofilter_context::reset(iface_factory_type factory)
{
    m_factory = std::move(factory);
    mp_auto_filter = nullptr;

    m_node_stack.clear();
    m_cur_col = -1;
}

void xlsx_autofilter_context::start_auto_filter(const xml_token_attrs_t& attrs)
{
    for (const auto& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_ref:
            {
                auto range = to_rc_range(m_resolver.resolve_range(attr.value));
                mp_auto_filter = m_factory(range);
                break;
            }
        }
    }

    if (!mp_auto_filter)
        return;

    auto* node = mp_auto_filter->start_node(ss::auto_filter_node_op_t::op_and);
    m_node_stack.push_back(node);
}

void xlsx_autofilter_context::end_auto_filter()
{
    if (!mp_auto_filter)
        return;

    if (m_node_stack.size() != 1u)
    {
        std::ostringstream os;
        os << "end_auto_filter: node-stack stack size was expected to be one, but is " << m_node_stack.size();
        throw xml_structure_error(os.str());
    }

    m_node_stack.back()->commit();
    m_node_stack.pop_back();
    mp_auto_filter->commit();
    mp_auto_filter = nullptr;
}

void xlsx_autofilter_context::start_custom_filters(const xml_token_attrs_t& attrs)
{
    if (!mp_auto_filter)
        return;

    if (m_node_stack.empty())
        return;

    auto node_op = ss::auto_filter_node_op_t::op_and;

    for (const auto& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_and:
            {
                node_op = to_bool(attr.value) ? ss::auto_filter_node_op_t::op_and : ss::auto_filter_node_op_t::op_or;
                break;
            }
        }
    }

    auto* node = m_node_stack.back()->start_node(node_op);
    ENSURE_INTERFACE(node, import_auto_filter_node);
    m_node_stack.push_back(node);
}

void xlsx_autofilter_context::end_custom_filters()
{
    if (!mp_auto_filter)
        return;

    if (m_node_stack.empty())
        return;

    m_node_stack.back()->commit();
    m_node_stack.pop_back();
}

void xlsx_autofilter_context::start_custom_filter(const xml_token_attrs_t& attrs)
{
    if (!mp_auto_filter)
        return;

    if (m_node_stack.empty())
        return;

    auto op = ss::auto_filter_op_t::unspecified;
    std::string_view val;

    for (const auto& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_operator:
                op = filterop::get().find(attr.value);
                break;
            case XML_val:
                val = attr.value;
                break;
        }
    }

    switch (op)
    {
        case ss::auto_filter_op_t::equal:
        case ss::auto_filter_op_t::not_equal:
        {
            // always treat rhs as text
            auto res = m_value_parser.parse(op, val);
            m_node_stack.back()->append_item(m_cur_col, res.op, res.value, res.regex);
            break;
        }
        case ss::auto_filter_op_t::greater:
        case ss::auto_filter_op_t::greater_equal:
        case ss::auto_filter_op_t::less:
        case ss::auto_filter_op_t::less_equal:
        {
            // these operators expect a numeric rhs operand
            auto v = to_double_checked(val);
            if (!v)
            {
                std::ostringstream os;
                os << "numeric value was expected for operator '" << filterop::get().find_key(op) << "', but '"
                    << val << "' cannot be interpreted as numeric";
                warn(os.str());
                return;
            }

            m_node_stack.back()->append_item(m_cur_col, op, *v);
            break;
        }
        default:
        {
            std::ostringstream os;
            os << "unexpected auto-filter condition operator '" << filterop::get().find_key(op) << "'";
            warn(os.str());
        }
    }
}

void xlsx_autofilter_context::end_custom_filter()
{
    if (!mp_auto_filter)
        return;

    if (m_node_stack.empty())
        return;
}

void xlsx_autofilter_context::start_filter_column(const xml_token_attrs_t& attrs)
{
    if (!mp_auto_filter)
        return;

    for (const auto& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_colId:
            {
                m_cur_col = -1;
                auto col = to_long_checked(attr.value);
                if (!col)
                    throw xml_structure_error("failed to parse a column id (colId) from filterColumn");

                m_cur_col = *col;
                break;
            }
        }
    }
}

void xlsx_autofilter_context::end_filter_column()
{
    if (!mp_auto_filter)
        return;

    m_cur_col = -1;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
