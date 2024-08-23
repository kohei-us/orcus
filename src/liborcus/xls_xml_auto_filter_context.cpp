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

}

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
            case XML_AutoFilterColumn:
                end_column();
                break;
        }
    }

    return pop_stack(ns, name);
}

void xls_xml_auto_filter_context::reset(spreadsheet::iface::import_sheet* parent_sheet)
{
    mp_sheet = parent_sheet;
    mp_auto_filter = nullptr;
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

void xls_xml_auto_filter_context::start_column(const xml_token_attrs_t& attrs)
{
    if (!mp_sheet)
        return;

    m_column.index = 0; // column offset from the left-most column, defaults to 1
    m_column.type = filter_column_type::all;

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
            }
        }
    }
}

void xls_xml_auto_filter_context::end_column()
{
    m_column.reset();
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
