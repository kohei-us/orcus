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

ss::fill_pattern_t to_fill_pattern(std::size_t v)
{
    constexpr ss::fill_pattern_t patterns[] = {
        ss::fill_pattern_t::none,
        ss::fill_pattern_t::solid,
        // TODO: add more as we establish more mapping rules
    };

    return (v < std::size(patterns)) ? patterns[v] : ss::fill_pattern_t::none;
}

gnumeric_style::border_type parse_border_attributes(const xml_token_attrs_t& attrs)
{
    gnumeric_style::border_type ret;

    for (const auto& attr : attrs)
    {
        if (attr.ns == XMLNS_UNKNOWN_ID)
        {
            switch (attr.name)
            {
                case XML_Style:
                {
                    const char* p_end = nullptr;
                    long v = to_long(attr.value, &p_end);
                    if (attr.value.data() < p_end)
                        ret.style = static_cast<gnumeric_border_type>(v);
                    break;
                }
                case XML_Color:
                {
                    ret.color = parse_gnumeric_rgb(attr.value);
                    break;
                }
            }
        }
    }

    return ret;
}

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
        { NS_gnumeric_gnm, XML_Style, NS_gnumeric_gnm, XML_Font },
        { NS_gnumeric_gnm, XML_Style, NS_gnumeric_gnm, XML_StyleBorder },
        { NS_gnumeric_gnm, XML_StyleBorder, NS_gnumeric_gnm, XML_Bottom },
        { NS_gnumeric_gnm, XML_StyleBorder, NS_gnumeric_gnm, XML_Diagonal },
        { NS_gnumeric_gnm, XML_StyleBorder, NS_gnumeric_gnm, XML_Left },
        { NS_gnumeric_gnm, XML_StyleBorder, NS_gnumeric_gnm, XML_Rev_Diagonal },
        { NS_gnumeric_gnm, XML_StyleBorder, NS_gnumeric_gnm, XML_Right },
        { NS_gnumeric_gnm, XML_StyleBorder, NS_gnumeric_gnm, XML_Top },
        { NS_gnumeric_gnm, XML_StyleRegion, NS_gnumeric_gnm, XML_Style },
        { NS_gnumeric_gnm, XML_Styles, NS_gnumeric_gnm, XML_StyleRegion },
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
            case XML_StyleBorder:
                break;
            case XML_Style:
                start_style(attrs);
                break;
            case XML_Top:
            {
                m_current_style.border_top = parse_border_attributes(attrs);
                break;
            }
            case XML_Bottom:
            {
                m_current_style.border_bottom = parse_border_attributes(attrs);
                break;
            }
            case XML_Left:
            {
                m_current_style.border_left = parse_border_attributes(attrs);
                break;
            }
            case XML_Right:
            {
                m_current_style.border_right = parse_border_attributes(attrs);
                break;
            }
            case XML_Diagonal:
            {
                m_current_style.border_bl_tr = parse_border_attributes(attrs);
                break;
            }
            case XML_Rev_Diagonal:
            {
                m_current_style.border_br_tl = parse_border_attributes(attrs);
                break;
            }
            case XML_Font:
                start_font(attrs);
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

void gnumeric_styles_context::characters(std::string_view str, bool transient)
{
    const auto [ns, name] = get_current_element();

    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_Font:
            {
                if (transient)
                    str = intern(str);
                m_current_style.font_name = str;
                break;
            }
        }
    }
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
            case XML_Fore:
                m_current_style.fore = parse_gnumeric_rgb(attr.value);
                break;
            case XML_Back:
                m_current_style.back = parse_gnumeric_rgb(attr.value);
                break;
            case XML_PatternColor:
                m_current_style.pattern_color = parse_gnumeric_rgb(attr.value);
                break;
            case XML_Shade:
                m_current_style.pattern = to_fill_pattern(to_long(attr.value));
                break;
        }
    }
}

void gnumeric_styles_context::start_font(const std::vector<xml_token_attr_t>& attrs)
{
    for (const auto& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_Unit:
            {
                const char* p_end = nullptr;
                double v = to_double(attr.value, &p_end);
                if (attr.value.data() < p_end)
                    m_current_style.font_unit = v;
                break;
            }
            case XML_Bold:
                m_current_style.bold = to_bool(attr.value);
                break;
            case XML_Italic:
                m_current_style.italic = to_bool(attr.value);
                break;
            case XML_Underline:
                m_current_style.underline = to_bool(attr.value);
                break;
            case XML_StrikeThrough:
                m_current_style.strikethrough = to_bool(attr.value);
                break;
            case XML_Script:
            {
                auto v = to_long(attr.value);
                m_current_style.script = static_cast<gnumeric_script_type>(v);
                break;
            }
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
