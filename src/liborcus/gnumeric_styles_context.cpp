/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gnumeric_styles_context.hpp"
#include "gnumeric_token_constants.hpp"
#include "gnumeric_namespace_types.hpp"

#include <orcus/measurement.hpp>
#include <mdds/sorted_string_map.hpp>

namespace ss = orcus::spreadsheet;

namespace orcus {

namespace {

namespace hor_align {

using map_type = mdds::sorted_string_map<ss::hor_alignment_t, mdds::string_view_map_entry>;

// Keys must be sorted.
constexpr map_type::entry entries[] = {
    { "GNM_HALIGN_CENTER", ss::hor_alignment_t::center },
    { "GNM_HALIGN_DISTRIBUTED", ss::hor_alignment_t::distributed },
    { "GNM_HALIGN_GENERAL", ss::hor_alignment_t::unknown },
    { "GNM_HALIGN_JUSTIFY", ss::hor_alignment_t::justified },
    { "GNM_HALIGN_LEFT", ss::hor_alignment_t::left },
    { "GNM_HALIGN_RIGHT", ss::hor_alignment_t::right },
};

const map_type& get()
{
    static const map_type mt(entries, std::size(entries), ss::hor_alignment_t::unknown);
    return mt;
}

} // namespace hor_align

namespace ver_align {

using map_type = mdds::sorted_string_map<ss::ver_alignment_t, mdds::string_view_map_entry>;

// Keys must be sorted.
constexpr map_type::entry entries[] = {
    { "GNM_VALIGN_BOTTOM", ss::ver_alignment_t::bottom },
    { "GNM_VALIGN_CENTER", ss::ver_alignment_t::middle },
    { "GNM_VALIGN_DISTRIBUTED", ss::ver_alignment_t::distributed },
    { "GNM_VALIGN_JUSTIFY", ss::ver_alignment_t::justified },
    { "GNM_VALIGN_TOP", ss::ver_alignment_t::top },
};

const map_type& get()
{
    static const map_type mt(entries, std::size(entries), ss::ver_alignment_t::unknown);
    return mt;
}

} // namespace ver_align

} // anonymous namespace

gnumeric_styles_context::gnumeric_styles_context(
    session_context& session_cxt, const tokens& tokens,
    ss::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_gnumeric_gnm, XML_Styles }, // root element
        { NS_gnumeric_gnm, XML_Styles, NS_gnumeric_gnm, XML_StyleRegion },
        { NS_gnumeric_gnm, XML_StyleRegion, NS_gnumeric_gnm, XML_Style },
        { NS_gnumeric_gnm, XML_Style, NS_gnumeric_gnm, XML_Font },
    };

    init_element_validator(rules, std::size(rules));
}

gnumeric_styles_context::~gnumeric_styles_context() = default;

void gnumeric_styles_context::start_element(
    xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs)
{
    push_stack(ns, name);

    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_StyleRegion:
                start_style_region(attrs);
                break;
            case XML_Style:
                start_style(attrs);
                break;
            case XML_Font:
                break;
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool gnumeric_styles_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_StyleRegion:
                end_style_region();
                break;
        }
    }
    return pop_stack(ns, name);
}

void gnumeric_styles_context::reset(ss::sheet_t sheet)
{
    m_sheet = sheet;
    m_styles.clear();
    m_current_style = gnumeric_style{};
}

std::vector<gnumeric_style> gnumeric_styles_context::pop_styles()
{
    return std::move(m_styles);
}

void gnumeric_styles_context::start_style_region(const std::vector<xml_token_attr_t>& attrs)
{
    m_current_style = gnumeric_style{};
    m_current_style.sheet = m_sheet;

    for (const auto& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_startCol:
                m_current_style.region.first.column = to_long(attr.value);
                break;
            case XML_startRow:
                m_current_style.region.first.row = to_long(attr.value);
                break;
            case XML_endCol:
                m_current_style.region.last.column = to_long(attr.value);
                break;
            case XML_endRow:
                m_current_style.region.last.row = to_long(attr.value);
                break;
        }
    }
}

void gnumeric_styles_context::start_style(const std::vector<xml_token_attr_t>& attrs)
{
    for (const auto& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_HAlign:
                m_current_style.hor_align = hor_align::get().find(attr.value);
                break;
            case XML_VAlign:
                m_current_style.ver_align = ver_align::get().find(attr.value);
                break;
            case XML_WrapText:
                m_current_style.wrap_text = to_bool(attr.value);
                break;
        }
    }
}

void gnumeric_styles_context::end_style_region()
{
    if (m_current_style.valid())
        m_styles.push_back(m_current_style);
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
