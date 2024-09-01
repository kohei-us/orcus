/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xls_xml_auto_filter_context.hpp"
#include "xls_xml_namespace_types.hpp"
#include "xls_xml_token_constants.hpp"

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/spreadsheet/import_interface_auto_filter.hpp>
#include <orcus/measurement.hpp>

#include <mdds/sorted_string_map.hpp>

#include <iostream>
#include <optional>

namespace ss = orcus::spreadsheet;

namespace orcus {

namespace {

namespace column_type {

using map_type = mdds::sorted_string_map<xls_xml_auto_filter_context::filter_column_type>;

// Keys must be sorted.
constexpr map_type::entry_type entries[] = {
    { "All", xls_xml_auto_filter_context::filter_column_type::all },
    { "Blanks", xls_xml_auto_filter_context::filter_column_type::blanks },
    { "Bottom", xls_xml_auto_filter_context::filter_column_type::bottom },
    { "BottomPercent", xls_xml_auto_filter_context::filter_column_type::bottom_percent },
    { "Custom", xls_xml_auto_filter_context::filter_column_type::custom },
    { "NonBlanks", xls_xml_auto_filter_context::filter_column_type::non_blanks },
    { "Top", xls_xml_auto_filter_context::filter_column_type::top },
    { "TopPercent", xls_xml_auto_filter_context::filter_column_type::top_percent },
};

const map_type& get()
{
    static const map_type mt(entries, std::size(entries), xls_xml_auto_filter_context::filter_column_type::all);
    return mt;
}

} // namespace column_type

namespace op_type {

using map_type = mdds::sorted_string_map<ss::auto_filter_op_t>;

// Keys must be sorted.
constexpr map_type::entry_type entries[] = {
    { "DoesNotEqual", ss::auto_filter_op_t::not_equal },
    { "Equals", ss::auto_filter_op_t::equal },
    { "GreaterThan", ss::auto_filter_op_t::greater },
    { "GreaterThanOrEqual", ss::auto_filter_op_t::greater_equal },
    { "LessThan", ss::auto_filter_op_t::less },
    { "LessThanOrEqual", ss::auto_filter_op_t::less_equal },
};

const map_type& get()
{
    static const map_type mt(entries, std::size(entries), ss::auto_filter_op_t::unspecified);
    return mt;
}

} // namespace op_type

} // anonymous namespace

xls_xml_auto_filter_context::xls_xml_auto_filter_context(
    session_context& session_cxt, const tokens& tokens, ss::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory)
{
    assert(mp_factory);

    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_xls_xml_x, XML_AutoFilter }, // root element
        { NS_xls_xml_x, XML_AutoFilter, NS_xls_xml_x, XML_AutoFilterColumn },
        { NS_xls_xml_x, XML_AutoFilterColumn, NS_xls_xml_x, XML_AutoFilterCondition },
        { NS_xls_xml_x, XML_AutoFilterColumn, NS_xls_xml_x, XML_AutoFilterAnd },
        { NS_xls_xml_x, XML_AutoFilterColumn, NS_xls_xml_x, XML_AutoFilterOr },
        { NS_xls_xml_x, XML_AutoFilterAnd, NS_xls_xml_x, XML_AutoFilterCondition },
        { NS_xls_xml_x, XML_AutoFilterOr, NS_xls_xml_x, XML_AutoFilterCondition },
    };

    init_element_validator(rules, std::size(rules));
}

xls_xml_auto_filter_context::~xls_xml_auto_filter_context() = default;

void xls_xml_auto_filter_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
{
    push_stack(ns, name);

    if (ns == NS_xls_xml_x)
    {
        switch (name)
        {
            case XML_AutoFilter:
                start_auto_filter(attrs);
                break;
            case XML_AutoFilterColumn:
                start_column(attrs);
                break;
            case XML_AutoFilterCondition:
                start_condition(attrs);
                break;
            case XML_AutoFilterAnd:
                start_filter_node(ss::auto_filter_node_op_t::op_and);
                break;
            case XML_AutoFilterOr:
                start_filter_node(ss::auto_filter_node_op_t::op_or);
                break;
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool xls_xml_auto_filter_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_xls_xml_x)
    {
        switch (name)
        {
            case XML_AutoFilter:
                end_auto_filter();
                break;
            case XML_AutoFilterColumn:
                end_column();
                break;
            case XML_AutoFilterAnd:
            case XML_AutoFilterOr:
                end_filter_node();
                break;
        }
    }

    return pop_stack(ns, name);
}

void xls_xml_auto_filter_context::reset(spreadsheet::iface::import_sheet* parent_sheet)
{
    mp_sheet = parent_sheet;
    mp_auto_filter = nullptr;
    m_filter_node_stack.clear();
}

void xls_xml_auto_filter_context::start_auto_filter(const xml_token_attrs_t& attrs)
{
    if (!mp_sheet)
        return;

    auto* resolver = mp_factory->get_reference_resolver(ss::formula_ref_context_t::global);
    if (!resolver)
        return;

    std::optional<ss::range_t> range;

    for (const auto& attr : attrs)
    {
        if (attr.ns == NS_xls_xml_x)
        {
            switch (attr.name)
            {
                case XML_Range:
                {
                    if (resolver)
                        range = to_rc_range(resolver->resolve_range(attr.value));
                    break;
                }
            }
        }
    }

    if (range)
        mp_auto_filter = mp_sheet->start_auto_filter(*range);
    else
        warn("range value did not get picked up in auto-filter context, skipping import");
}

void xls_xml_auto_filter_context::end_auto_filter()
{
    if (!mp_auto_filter)
        return;

    assert(m_filter_node_stack.size() == 1u); // root node
    m_filter_node_stack.back()->commit();
    m_filter_node_stack.pop_back();
    mp_auto_filter->commit();
}

void xls_xml_auto_filter_context::start_column(const xml_token_attrs_t& attrs)
{
    if (!mp_auto_filter)
        return;

    m_column.reset();

    for (const auto& attr : attrs)
    {
        if (attr.ns == NS_xls_xml_x)
        {
            switch (attr.name)
            {
                case XML_Index:
                {
                    if (auto v = to_long_checked(attr.value); v)
                        m_column.index = *v - 1; // 1-based to 0-based
                    break;
                }
                case XML_Type:
                {
                    m_column.type = column_type::get().find(attr.value);
                    break;
                }
                case XML_Value:
                {
                    auto v = to_double_checked(attr.value);
                    if (v)
                        m_column.value = *v;
                    else
                    {
                        std::ostringstream os;
                        os << "failed to convert '" << attr.value << "' to a double for a column filter rule";
                        warn(os.str());
                    }
                }
            }
        }
    }

    if (m_filter_node_stack.empty())
    {
        // create a root node before the first field
        auto* root = mp_auto_filter->start_node(ss::auto_filter_node_op_t::op_and);
        m_filter_node_stack.push_back(root);
    }
}

void xls_xml_auto_filter_context::end_column()
{
    if (!mp_auto_filter)
        return;

    auto to_dest_type = [](filter_column_type src) -> std::optional<ss::auto_filter_op_t>
    {
        switch (src)
        {
            case filter_column_type::blanks:
                return ss::auto_filter_op_t::empty;
            case filter_column_type::non_blanks:
                return ss::auto_filter_op_t::not_empty;
            case filter_column_type::top:
                return ss::auto_filter_op_t::top;
            case filter_column_type::top_percent:
                return ss::auto_filter_op_t::top_percent;
            case filter_column_type::bottom:
                return ss::auto_filter_op_t::bottom;
            case filter_column_type::bottom_percent:
                return ss::auto_filter_op_t::bottom_percent;
            default:;
        }
        return {};
    };

    switch (m_column.type)
    {
        case filter_column_type::all:
            // nothing to filter
            break;
        case filter_column_type::blanks:
        case filter_column_type::non_blanks:
        case filter_column_type::top:
        case filter_column_type::top_percent:
        case filter_column_type::bottom:
        case filter_column_type::bottom_percent:
        {
            auto op = to_dest_type(m_column.type);
            assert(op.has_value());
            assert(m_filter_node_stack.size() == 1u);
            auto* node = m_filter_node_stack.back()->start_node(ss::auto_filter_node_op_t::op_and);
            node->append_item(m_column.index, *op, m_column.value);
            node->commit();
            break;
        }
        case filter_column_type::custom:
        {
            if (m_filter_node_stack.size() == 2u)
                end_filter_node();
            break;
        }
    }

    assert(m_filter_node_stack.size() == 1u); // root node should still exist
    m_column.reset();
}

void xls_xml_auto_filter_context::start_condition(const xml_token_attrs_t& attrs)
{
    if (!mp_auto_filter)
        return;

    if (m_column.node_op == ss::auto_filter_node_op_t::unspecified)
    {
        // if not explicitly under <x:AutoFilterAnd> or <x:AutoFilterOr>, it's
        // equivalent of being under <x:AutoFilterAnd>
        start_filter_node(ss::auto_filter_node_op_t::op_and);
    }

    std::optional<ss::auto_filter_op_t> op;
    std::optional<std::string_view> value;

    for (const auto& attr : attrs)
    {
        if (attr.ns == NS_xls_xml_x)
        {
            switch (attr.name)
            {
                case XML_Operator:
                {
                    op = op_type::get().find(attr.value);
                    break;
                }
                case XML_Value:
                {
                    value = attr.value;
                    break;
                }
            }
        }
    }

    if (!op || !value)
    {
        warn("either the x:Operator or x:Value attribute is missing, or both");
        return;
    }

    switch (*op)
    {
        case ss::auto_filter_op_t::equal:
        case ss::auto_filter_op_t::not_equal:
        {
            // since the value type is not specified, try converting it to a numeric
            // value and if that succeeds, import it as a numeric value.

            assert(!m_filter_node_stack.empty());
            auto* node = m_filter_node_stack.back();
            if (auto v = to_double_checked(*value); v)
                node->append_item(m_column.index, *op, *v); // numeric
            else
                node->append_item(m_column.index, *op, *value); // text

            break;
        }
        case ss::auto_filter_op_t::greater:
        case ss::auto_filter_op_t::greater_equal:
        case ss::auto_filter_op_t::less:
        case ss::auto_filter_op_t::less_equal:
        {
            // these operators expect a numeric rhs operand
            auto v = to_double_checked(*value);
            if (!v)
            {
                std::ostringstream os;
                os << "numeric value was expected for operator '" << op_type::get().find_key(*op) << "', but '"
                    << *value << "' cannot be interpreted as numeric";
                warn(os.str());
                return;
            }

            assert(!m_filter_node_stack.empty());
            auto* node = m_filter_node_stack.back();
            node->append_item(m_column.index, *op, *v);
            break;
        }
        default:
        {
            std::ostringstream os;
            os << "unexpected auto-filter condition operator '" << op_type::get().find_key(*op) << "'";
            warn(os.str());
        }
    }
}

void xls_xml_auto_filter_context::start_filter_node(ss::auto_filter_node_op_t op)
{
    m_column.node_op = op;

    assert(!m_filter_node_stack.empty());
    auto* node = m_filter_node_stack.back()->start_node(m_column.node_op);
    m_filter_node_stack.push_back(node);
}

void xls_xml_auto_filter_context::end_filter_node()
{
    assert(!m_filter_node_stack.empty());
    m_filter_node_stack.back()->commit();
    m_filter_node_stack.pop_back();
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
