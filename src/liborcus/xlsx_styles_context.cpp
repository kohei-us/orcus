/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xlsx_styles_context.hpp"
#include "impl_utils.hpp"
#include "ooxml_namespace_types.hpp"
#include "ooxml_token_constants.hpp"
#include "xlsx_helper.hpp"
#include "xml_context_global.hpp"

#include <orcus/tokens.hpp>
#include <orcus/measurement.hpp>
#include <orcus/spreadsheet/import_interface_styles.hpp>

#include <mdds/sorted_string_map.hpp>
#include <mdds/global.hpp>

#include <optional>

namespace ss = orcus::spreadsheet;

namespace orcus {

namespace {

namespace border_style {

using map_type = mdds::sorted_string_map<ss::border_style_t, mdds::string_view_map_entry>;

// Keys must be sorted.
constexpr map_type::entry entries[] = {
    { "dashDot",          ss::border_style_t::dash_dot            },
    { "dashDotDot",       ss::border_style_t::dash_dot_dot        },
    { "dashed",           ss::border_style_t::dashed              },
    { "dotted",           ss::border_style_t::dotted              },
    { "double",           ss::border_style_t::double_border       },
    { "hair",             ss::border_style_t::hair                },
    { "medium",           ss::border_style_t::medium              },
    { "mediumDashDot",    ss::border_style_t::medium_dash_dot     },
    { "mediumDashDotDot", ss::border_style_t::medium_dash_dot_dot },
    { "mediumDashed",     ss::border_style_t::medium_dashed       },
    { "none",             ss::border_style_t::none                },
    { "slantDashDot",     ss::border_style_t::slant_dash_dot      },
    { "thick",            ss::border_style_t::thick               },
    { "thin",             ss::border_style_t::thin                }
};

const map_type& get()
{
    static const map_type mt(entries, std::size(entries), ss::border_style_t::none);
    return mt;
}

} // border_style namespace

namespace fill_pattern {

using map_type = mdds::sorted_string_map<ss::fill_pattern_t, mdds::string_view_map_entry>;

// Keys must be sorted.
constexpr map_type::entry entries[] = {
    { "darkDown",        ss::fill_pattern_t::dark_down        },
    { "darkGray",        ss::fill_pattern_t::dark_gray        },
    { "darkGrid",        ss::fill_pattern_t::dark_grid        },
    { "darkHorizontal",  ss::fill_pattern_t::dark_horizontal  },
    { "darkTrellis",     ss::fill_pattern_t::dark_trellis     },
    { "darkUp",          ss::fill_pattern_t::dark_up          },
    { "darkVertical",    ss::fill_pattern_t::dark_vertical    },
    { "gray0625",        ss::fill_pattern_t::gray_0625        },
    { "gray125",         ss::fill_pattern_t::gray_125         },
    { "lightDown",       ss::fill_pattern_t::light_down       },
    { "lightGray",       ss::fill_pattern_t::light_gray       },
    { "lightGrid",       ss::fill_pattern_t::light_grid       },
    { "lightHorizontal", ss::fill_pattern_t::light_horizontal },
    { "lightTrellis",    ss::fill_pattern_t::light_trellis    },
    { "lightUp",         ss::fill_pattern_t::light_up         },
    { "lightVertical",   ss::fill_pattern_t::light_vertical   },
    { "mediumGray",      ss::fill_pattern_t::medium_gray      },
    { "none",            ss::fill_pattern_t::none             },
    { "solid",           ss::fill_pattern_t::solid            },
};

const map_type& get()
{
    static const map_type mt(entries, std::size(entries), ss::fill_pattern_t::none);
    return mt;
}

} // fill_pattern namespace

namespace underline {

using map_type = mdds::sorted_string_map<ss::underline_t, mdds::string_view_map_entry>;

// Keys must be sorted.
constexpr map_type::entry entries[] = {
    { "double", ss::underline_t::double_line },
    { "doubleAccounting", ss::underline_t::double_accounting },
    { "none", ss::underline_t::none },
    { "single", ss::underline_t::single_line },
    { "singleAccounting", ss::underline_t::single_accounting },
};

const map_type& get()
{
    static const map_type mt(entries, std::size(entries), ss::underline_t::none);
    return mt;
}

} // namespace underline

class border_attr_parser
{
    ss::border_direction_t m_dir;
    ss::iface::import_border_style& m_border_style;
public:
    border_attr_parser(ss::border_direction_t dir, ss::iface::import_border_style& style) :
        m_dir(dir), m_border_style(style) {}

    void operator() (const xml_token_attr_t& attr)
    {
        switch (attr.name)
        {
            case XML_style:
            {
                m_border_style.set_style(m_dir,
                    border_style::get().find(attr.value.data(), attr.value.size()));
                break;
            }
        }
    }
};

std::optional<std::size_t> extract_count(const xml_token_attrs_t& attrs)
{
    std::optional<std::size_t> count;

    for (const auto& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_count:
            {
                const char* p_end = nullptr;
                long v = to_long(attr.value, &p_end);
                if (attr.value.data() < p_end && v >= 0)
                    count = v;
                break;
            }
        }
    }

    return count;
}

} // anonymous namespace

xlsx_styles_context::xlsx_styles_context(session_context& session_cxt, const tokens& tokens, ss::iface::import_styles* styles) :
    xml_context_base(session_cxt, tokens),
    mp_styles(styles),
    m_diagonal_up(false), m_diagonal_down(false),
    m_cur_border_dir(ss::border_direction_t::unknown),
    m_cell_style_xf(false)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_ooxml_xlsx, XML_styleSheet }, // root element
        { NS_ooxml_xlsx, XML_border, NS_ooxml_xlsx, XML_bottom },
        { NS_ooxml_xlsx, XML_border, NS_ooxml_xlsx, XML_diagonal },
        { NS_ooxml_xlsx, XML_border, NS_ooxml_xlsx, XML_left },
        { NS_ooxml_xlsx, XML_border, NS_ooxml_xlsx, XML_right },
        { NS_ooxml_xlsx, XML_border, NS_ooxml_xlsx, XML_top },
        { NS_ooxml_xlsx, XML_borders, NS_ooxml_xlsx, XML_border },
        { NS_ooxml_xlsx, XML_bottom, NS_ooxml_xlsx, XML_color },
        { NS_ooxml_xlsx, XML_cellStyleXfs, NS_ooxml_xlsx, XML_xf },
        { NS_ooxml_xlsx, XML_cellStyles, NS_ooxml_xlsx, XML_cellStyle },
        { NS_ooxml_xlsx, XML_cellXfs, NS_ooxml_xlsx, XML_xf },
        { NS_ooxml_xlsx, XML_diagonal, NS_ooxml_xlsx, XML_color },
        { NS_ooxml_xlsx, XML_dx, NS_ooxml_xlsx, XML_fill },
        { NS_ooxml_xlsx, XML_dxf, NS_ooxml_xlsx, XML_alignment },
        { NS_ooxml_xlsx, XML_dxf, NS_ooxml_xlsx, XML_border },
        { NS_ooxml_xlsx, XML_dxf, NS_ooxml_xlsx, XML_font },
        { NS_ooxml_xlsx, XML_dxf, NS_ooxml_xlsx, XML_numFmt },
        { NS_ooxml_xlsx, XML_dxf, NS_ooxml_xlsx, XML_protection },
        { NS_ooxml_xlsx, XML_end, NS_ooxml_xlsx, XML_color },
        { NS_ooxml_xlsx, XML_fill, NS_ooxml_xlsx, XML_patternFill },
        { NS_ooxml_xlsx, XML_fills, NS_ooxml_xlsx, XML_fill },
        { NS_ooxml_xlsx, XML_font, NS_ooxml_xlsx, XML_b },
        { NS_ooxml_xlsx, XML_font, NS_ooxml_xlsx, XML_color },
        { NS_ooxml_xlsx, XML_font, NS_ooxml_xlsx, XML_family },
        { NS_ooxml_xlsx, XML_font, NS_ooxml_xlsx, XML_i },
        { NS_ooxml_xlsx, XML_font, NS_ooxml_xlsx, XML_name },
        { NS_ooxml_xlsx, XML_font, NS_ooxml_xlsx, XML_scheme },
        { NS_ooxml_xlsx, XML_font, NS_ooxml_xlsx, XML_sz },
        { NS_ooxml_xlsx, XML_font, NS_ooxml_xlsx, XML_u },
        { NS_ooxml_xlsx, XML_fonts, NS_ooxml_xlsx, XML_font },
        { NS_ooxml_xlsx, XML_horizontal, NS_ooxml_xlsx, XML_color },
        { NS_ooxml_xlsx, XML_left, NS_ooxml_xlsx, XML_color },
        { NS_ooxml_xlsx, XML_mruColors, NS_ooxml_xlsx, XML_color },
        { NS_ooxml_xlsx, XML_numFmts, NS_ooxml_xlsx, XML_numFmt },
        { NS_ooxml_xlsx, XML_patternFill, NS_ooxml_xlsx, XML_bgColor },
        { NS_ooxml_xlsx, XML_patternFill, NS_ooxml_xlsx, XML_fgColor },
        { NS_ooxml_xlsx, XML_right, NS_ooxml_xlsx, XML_color },
        { NS_ooxml_xlsx, XML_start, NS_ooxml_xlsx, XML_color },
        { NS_ooxml_xlsx, XML_stop, NS_ooxml_xlsx, XML_color },
        { NS_ooxml_xlsx, XML_styleSheet, NS_ooxml_xlsx, XML_borders },
        { NS_ooxml_xlsx, XML_styleSheet, NS_ooxml_xlsx, XML_cellStyleXfs },
        { NS_ooxml_xlsx, XML_styleSheet, NS_ooxml_xlsx, XML_cellStyles },
        { NS_ooxml_xlsx, XML_styleSheet, NS_ooxml_xlsx, XML_cellXfs },
        { NS_ooxml_xlsx, XML_styleSheet, NS_ooxml_xlsx, XML_dxfs },
        { NS_ooxml_xlsx, XML_styleSheet, NS_ooxml_xlsx, XML_fills },
        { NS_ooxml_xlsx, XML_styleSheet, NS_ooxml_xlsx, XML_fonts },
        { NS_ooxml_xlsx, XML_styleSheet, NS_ooxml_xlsx, XML_numFmts },
        { NS_ooxml_xlsx, XML_top, NS_ooxml_xlsx, XML_color },
        { NS_ooxml_xlsx, XML_vertical, NS_ooxml_xlsx, XML_color },
        { NS_ooxml_xlsx, XML_xf, NS_ooxml_xlsx, XML_alignment },
        { NS_ooxml_xlsx, XML_xf, NS_ooxml_xlsx, XML_protection },
    };

    init_element_validator(rules, std::size(rules));
}

xlsx_styles_context::~xlsx_styles_context() = default;

void xlsx_styles_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);

    if (ns == NS_ooxml_xlsx)
    {
        switch (name)
        {
            case XML_styleSheet:
            {
                // root element
                if (get_config().debug)
                    print_attrs(get_tokens(), attrs);
                break;
            }
            case XML_fonts:
            {
                std::string_view ps = for_each(
                    attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_count)).get_value();
                size_t font_count = to_long(ps);
                mp_styles->set_font_count(font_count);
                m_font_ids.reserve(font_count);
                break;
            }
            case XML_font:
            {
                mp_font = mp_styles->start_font_style();
                ENSURE_INTERFACE(mp_font, import_font_style);
                break;
            }
            case XML_b:
                assert(mp_font);
                mp_font->set_bold(true);
                break;
            case XML_i:
                assert(mp_font);
                mp_font->set_italic(true);
                break;
            case XML_u:
            {
                assert(mp_font);
                ss::underline_t v = ss::underline_t::single_line; // default value

                for (const auto& attr : attrs)
                {
                    switch (name)
                    {
                        case XML_val:
                            v = underline::get().find(attr.value);
                            break;
                    }
                }

                mp_font->set_underline(v);
                break;
            }
            case XML_sz:
            {
                assert(mp_font);
                std::optional<double> font_size;

                for (const auto& attr : attrs)
                {
                    switch (attr.name)
                    {
                        case XML_val:
                        {
                            const char* p_end = nullptr;
                            double v = to_double(attr.value, &p_end);
                            if (attr.value.data() < p_end)
                                font_size = v;
                            break;
                        }
                    }
                }

                if (font_size)
                    mp_font->set_size(*font_size);

                break;
            }
            case XML_color:
            {
                if (parent.first == NS_ooxml_xlsx)
                {
                    switch (parent.second)
                    {
                        case XML_top:
                        case XML_bottom:
                        case XML_left:
                        case XML_right:
                        case XML_diagonal:
                            // This color is for a border.
                            start_border_color(attrs);
                        break;
                        case XML_font:
                            start_font_color(attrs);
                        default:
                            ;
                    }
                }
                break;
            }
            case XML_name:
            {
                std::string_view ps = for_each(
                    attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_val)).get_value();
                mp_font->set_name(ps);
                break;
            }
            case XML_fills:
            {
                std::string_view ps = for_each(
                    attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_count)).get_value();
                size_t fill_count = to_long(ps);
                mp_styles->set_fill_count(fill_count);
                m_fill_ids.reserve(fill_count);
                break;
            }
            case XML_fill:
            {
                mp_fill = mp_styles->start_fill_style();
                ENSURE_INTERFACE(mp_fill, import_fill_style);

                break;
            }
            case XML_patternFill:
            {
                std::string_view ps = for_each(
                    attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_patternType)).get_value();
                assert(mp_fill);
                mp_fill->set_pattern_type(fill_pattern::get().find(ps.data(), ps.size()));
                break;
            }
            case XML_fgColor:
            {
                assert(mp_fill);

                for (const xml_token_attr_t& attr : attrs)
                {
                    switch (attr.name)
                    {
                        case XML_rgb:
                        {
                            ss::color_elem_t alpha;
                            ss::color_elem_t red;
                            ss::color_elem_t green;
                            ss::color_elem_t blue;
                            if (!to_rgb(attr.value, alpha, red, green, blue))
                                // invalid RGB color format.
                                continue;

                            mp_fill->set_fg_color(alpha, red, green, blue);
                            break;
                        }
                        case XML_indexed:
                            break;
                        default:
                            if (get_config().debug)
                                std::cerr << "warning: unknown attribute [ " << get_tokens().get_token_name(attr.name) << " ]" << std::endl;
                    }
                }

                break;
            }
            case XML_bgColor:
            {
                for (const xml_token_attr_t& attr : attrs)
                {
                    switch (attr.name)
                    {
                        case XML_rgb:
                        {
                            ss::color_elem_t alpha;
                            ss::color_elem_t red;
                            ss::color_elem_t green;
                            ss::color_elem_t blue;
                            if (!to_rgb(attr.value, alpha, red, green, blue))
                                // invalid RGB color format.
                                continue;

                            mp_fill->set_bg_color(alpha, red, green, blue);
                            break;
                        }
                        case XML_indexed:
                            break;
                        default:
                            if (get_config().debug)
                                std::cerr << "warning: unknown attribute [ " << get_tokens().get_token_name(attr.name) << " ]" << std::endl;
                    }
                }

                break;
            }
            case XML_borders:
            {
                std::string_view ps = for_each(
                    attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_count)).get_value();
                size_t border_count = to_long(ps);
                mp_styles->set_border_count(border_count);
                m_border_ids.reserve(border_count);
                break;
            }
            case XML_border:
            {
                start_element_border(attrs);

                mp_border = mp_styles->start_border_style();
                ENSURE_INTERFACE(mp_border, import_border_style);

                break;
            }
            case XML_top:
            {
                assert(mp_border);
                m_cur_border_dir = ss::border_direction_t::top;
                border_attr_parser func(ss::border_direction_t::top, *mp_border);
                for_each(attrs.begin(), attrs.end(), func);
                break;
            }
            case XML_bottom:
            {
                assert(mp_border);
                m_cur_border_dir = ss::border_direction_t::bottom;
                border_attr_parser func(ss::border_direction_t::bottom, *mp_border);
                for_each(attrs.begin(), attrs.end(), func);
                break;
            }
            case XML_left:
            {
                assert(mp_border);
                m_cur_border_dir = ss::border_direction_t::left;
                border_attr_parser func(ss::border_direction_t::left, *mp_border);
                for_each(attrs.begin(), attrs.end(), func);
                break;
            }
            case XML_right:
            {
                assert(mp_border);
                m_cur_border_dir = ss::border_direction_t::right;
                border_attr_parser func(ss::border_direction_t::right, *mp_border);
                for_each(attrs.begin(), attrs.end(), func);
                break;
            }
            case XML_diagonal:
            {
                start_element_diagonal(attrs);
                break;
            }
            case XML_cellStyleXfs:
            {
                if (std::optional<std::size_t> count = extract_count(attrs); count)
                {
                    mp_styles->set_xf_count(ss::xf_category_t::cell_style, *count);
                    m_cell_style_xf_ids.reserve(*count);
                }

                m_cell_style_xf = true;
                mp_xf = mp_styles->start_xf(ss::xf_category_t::cell_style);
                ENSURE_INTERFACE(mp_xf, import_xf);
                m_xf_type = ss::xf_category_t::cell_style;
                break;
            }
            case XML_cellXfs:
            {
                if (std::optional<std::size_t> count = extract_count(attrs); count)
                {
                    mp_styles->set_xf_count(ss::xf_category_t::cell, *count);
                    m_cell_xf_ids.reserve(*count);
                }

                m_cell_style_xf = false;
                mp_xf = mp_styles->start_xf(ss::xf_category_t::cell);
                ENSURE_INTERFACE(mp_xf, import_xf);
                m_xf_type = ss::xf_category_t::cell;
                break;
            }
            case XML_dxfs:
            {
                if (std::optional<std::size_t> count = extract_count(attrs); count)
                {
                    mp_styles->set_xf_count(ss::xf_category_t::differential, *count);
                    m_dxf_ids.reserve(*count);
                }

                mp_xf = mp_styles->start_xf(ss::xf_category_t::differential);
                ENSURE_INTERFACE(mp_xf, import_xf);
                m_xf_type = ss::xf_category_t::differential;
                break;
            }
            case XML_cellStyles:
            {
                std::string_view ps = for_each(
                    attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_count)).get_value();
                if (!ps.empty())
                {
                    size_t n = strtoul(ps.data(), nullptr, 10);
                    mp_styles->set_cell_style_count(n);
                }
                mp_cell_style = mp_styles->start_cell_style();
                ENSURE_INTERFACE(mp_cell_style, import_cell_style);
                break;
            }
            case XML_cellStyle:
            {
                // named cell style, some of which are built-in such as 'Normal'.
                assert(mp_cell_style);

                for (const xml_token_attr_t& attr : attrs)
                {
                    switch (attr.name)
                    {
                        case XML_name:
                            mp_cell_style->set_name(attr.value);
                            break;
                        case XML_xfId:
                        {
                            size_t n = to_long(attr.value);
                            mp_cell_style->set_xf(n);
                            break;
                        }
                        case XML_builtinId:
                        {
                            size_t n = to_long(attr.value);
                            mp_cell_style->set_builtin(n);
                        }
                        break;
                    }
                }
                break;
            }
            case XML_xf:
            {
                start_xf(attrs);
                break;
            }
            case XML_protection:
            {
                mp_protection = mp_styles->start_cell_protection();
                ENSURE_INTERFACE(mp_protection, import_cell_protection);

                for (const auto& attr : attrs)
                {
                    switch (attr.name)
                    {
                        case XML_hidden:
                        {
                            bool b = to_long(attr.value) != 0;
                            mp_protection->set_hidden(b);
                            break;
                        }
                        case XML_locked:
                        {
                            bool b = to_long(attr.value) != 0;
                            mp_protection->set_locked(b);
                            break;
                        }
                    }
                }

                break;
            }
            case XML_alignment:
            {
                assert(mp_xf);

                // NB: default vertical alignment is 'bottom'.
                ss::hor_alignment_t hor_align = ss::hor_alignment_t::unknown;
                ss::ver_alignment_t ver_align = ss::ver_alignment_t::bottom;
                bool wrap_text = false;
                bool shrink_to_fit = false;

                for (const xml_token_attr_t& attr : attrs)
                {
                    switch (attr.name)
                    {
                        case XML_horizontal:
                        {
                            if (attr.value == "center")
                                hor_align = ss::hor_alignment_t::center;
                            else if (attr.value == "right")
                                hor_align = ss::hor_alignment_t::right;
                            else if (attr.value == "left")
                                hor_align = ss::hor_alignment_t::left;
                            else if (attr.value == "justify")
                                hor_align = ss::hor_alignment_t::justified;
                            else if (attr.value == "distributed")
                                hor_align = ss::hor_alignment_t::distributed;
                            break;
                        }
                        case XML_vertical:
                        {
                            if (attr.value == "top")
                                ver_align = ss::ver_alignment_t::top;
                            else if (attr.value == "center")
                                ver_align = ss::ver_alignment_t::middle;
                            else if (attr.value == "bottom")
                                ver_align = ss::ver_alignment_t::bottom;
                            else if (attr.value == "justify")
                                ver_align = ss::ver_alignment_t::justified;
                            else if (attr.value == "distributed")
                                ver_align = ss::ver_alignment_t::distributed;
                            break;
                        }
                        case XML_wrapText:
                            wrap_text = to_bool(attr.value);
                            break;
                        case XML_shrinkToFit:
                            shrink_to_fit = to_bool(attr.value);
                            break;
                    }
                }

                mp_xf->set_horizontal_alignment(hor_align);
                mp_xf->set_vertical_alignment(ver_align);
                mp_xf->set_wrap_text(wrap_text);
                mp_xf->set_shrink_to_fit(shrink_to_fit);
                break;
            }
            case XML_numFmts:
            {
                std::string_view val =
                    for_each(
                        attrs.begin(), attrs.end(),
                        single_attr_getter(m_pool, NS_ooxml_xlsx, XML_count)).get_value();
                if (!val.empty())
                {
                    size_t n = to_long(val);
                    mp_styles->set_number_format_count(n);
                }
                break;
            }
            case XML_numFmt:
                start_element_number_format(attrs);
                break;
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool xlsx_styles_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    switch (name)
    {
        case XML_font:
        {
            assert(mp_font);
            std::size_t id = mp_font->commit();
            m_font_ids.push_back(id);
            mp_font = nullptr;
            break;
        }
        case XML_fill:
        {
            assert(mp_fill);
            std::size_t id = mp_fill->commit();
            m_fill_ids.push_back(id);
            mp_fill = nullptr;
            break;
        }
        case XML_border:
        {
            assert(mp_border);
            std::size_t id = mp_border->commit();
            m_border_ids.push_back(id);
            mp_border = nullptr;
            break;
        }
        case XML_cellStyle:
            assert(mp_cell_style);
            mp_cell_style->commit();
            break;
        case XML_cellStyles:
            assert(mp_cell_style);
            mp_cell_style = nullptr;
            break;
        case XML_cellStyleXfs:
        case XML_cellXfs:
        case XML_dxfs:
            assert(mp_xf);
            mp_xf = nullptr;
            m_xf_type = ss::xf_category_t::unknown;
            break;
        case XML_xf:
        case XML_dxf:
        {
            assert(mp_xf);
            std::size_t id = mp_xf->commit();
            switch (m_xf_type)
            {
                case ss::xf_category_t::cell:
                    m_cell_xf_ids.push_back(id);
                    break;
                case ss::xf_category_t::cell_style:
                    m_cell_style_xf_ids.push_back(id);
                    break;
                case ss::xf_category_t::differential:
                    m_dxf_ids.push_back(id);
                    break;
                case ss::xf_category_t::unknown:
                    warn("xf entry committed while the current xf category is unknown");
                    break;
            }
            break;
        }
        case XML_protection:
        {
            assert(mp_protection);
            size_t prot_id = mp_protection->commit();
            assert(mp_xf);
            mp_xf->set_protection(prot_id);
            break;
        }
        case XML_numFmt:
            end_element_number_format();
            break;
    }
    return pop_stack(ns, name);
}

void xlsx_styles_context::characters(std::string_view /*str*/, bool /*transient*/)
{
    // not used in the styles.xml part.
}

void xlsx_styles_context::start_element_number_format(const xml_token_attrs_t& attrs)
{
    if (!mp_styles)
        return;

    mp_numfmt = mp_styles->start_number_format();
    ENSURE_INTERFACE(mp_numfmt, import_number_format);

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns && attr.ns != NS_ooxml_xlsx)
            continue;

        switch (attr.name)
        {
            case XML_numFmtId:
            {
                long id = to_long(attr.value);
                mp_numfmt->set_identifier(id);
                break;
            }
            case XML_formatCode:
            {
                mp_numfmt->set_code(attr.value);
                break;
            }
        }
    }
}

void xlsx_styles_context::start_element_border(const xml_token_attrs_t& attrs)
{
    bool diagonal_up = false;
    bool diagonal_down = false;

    std::for_each(attrs.begin(), attrs.end(),
        [&diagonal_up,&diagonal_down](const xml_token_attr_t& attr)
        {
            if (attr.ns && attr.ns != NS_ooxml_xlsx)
                return;

            switch (attr.name)
            {
                case XML_diagonalDown:
                    // top-left to bottom-right.
                    diagonal_down = to_long(attr.value) != 0;
                    break;
                case XML_diagonalUp:
                    // bottom-left to top-right.
                    diagonal_up = to_long(attr.value) != 0;
                    break;
                default:
                    ;
            }
        }
    );

    m_diagonal_up = diagonal_up;
    m_diagonal_down = diagonal_down;
}

void xlsx_styles_context::start_element_diagonal(const xml_token_attrs_t& attrs)
{
    assert(mp_border);

    m_cur_border_dir = ss::border_direction_t::unknown;

    if (m_diagonal_up)
    {
        m_cur_border_dir = m_diagonal_down ?
            ss::border_direction_t::diagonal :
            ss::border_direction_t::diagonal_bl_tr;
    }
    else
    {
        m_cur_border_dir = m_diagonal_down ?
            ss::border_direction_t::diagonal_tl_br :
            ss::border_direction_t::unknown;
    }

    if (m_cur_border_dir == ss::border_direction_t::unknown)
        return;

    border_attr_parser func(m_cur_border_dir, *mp_border);
    for_each(attrs.begin(), attrs.end(), func);
}

void xlsx_styles_context::start_border_color(const xml_token_attrs_t& attrs)
{
    assert(mp_border);

    std::optional<std::string_view> rgb;

    for (const xml_token_attr_t& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_rgb:
                rgb = attr.value;
                break;
            case XML_theme:
                // TODO : handle this.
                break;
        }
    }

    if (rgb)
    {
        ss::color_elem_t alpha;
        ss::color_elem_t red;
        ss::color_elem_t green;
        ss::color_elem_t blue;

        if (to_rgb(*rgb, alpha, red, green, blue))
            mp_border->set_color(m_cur_border_dir, alpha, red, green, blue);
    }
}

void xlsx_styles_context::start_font_color(const xml_token_attrs_t& attrs)
{
    assert(mp_font);

    std::optional<std::string_view> rgb;

    for (const xml_token_attr_t& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_rgb:
                rgb = attr.value;
                break;
            case XML_theme:
                // TODO : handle this.
                break;
        }
    }

    if (rgb)
    {
        ss::color_elem_t alpha;
        ss::color_elem_t red;
        ss::color_elem_t green;
        ss::color_elem_t blue;
        if (to_rgb(*rgb, alpha, red, green, blue))
            mp_font->set_color(alpha, red, green, blue);
    }
}

void xlsx_styles_context::start_xf(const xml_token_attrs_t& attrs)
{
    assert(mp_xf);

    for (const xml_token_attr_t& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_borderId:
            {
                const char* p_end = nullptr;
                size_t n = to_long(attr.value, &p_end);
                if (attr.value.data() < p_end)
                {
                    if (n < m_border_ids.size())
                        mp_xf->set_border(m_border_ids[n]);
                    else
                    {
                        std::ostringstream os;
                        os << "out-of-bound borderId: id=" << n << "; count=" << m_border_ids.size();
                        warn(os.str());
                    }
                }
                break;
            }
            case XML_fillId:
            {
                const char* p_end = nullptr;
                size_t n = to_long(attr.value, &p_end);
                if (attr.value.data() < p_end)
                {
                    if (n < m_fill_ids.size())
                        mp_xf->set_fill(m_fill_ids[n]);
                    else
                    {
                        std::ostringstream os;
                        os << "out-of-bound fillId: id=" << n << "; count=" << m_fill_ids.size();
                        warn(os.str());
                    }
                }
                break;
            }
            case XML_fontId:
            {
                const char* p_end = nullptr;
                size_t n = to_long(attr.value, &p_end);
                if (attr.value.data() < p_end)
                {
                    if (n < m_font_ids.size())
                        mp_xf->set_font(m_font_ids[n]);
                    else
                    {
                        std::ostringstream os;
                        os << "out-of-bound fontId: id=" << n << "; count=" << m_font_ids.size();
                        warn(os.str());
                    }
                }
                break;
            }
            case XML_numFmtId:
            {
                size_t n = to_long(attr.value);
                mp_xf->set_number_format(n);
                break;
            }
            case XML_xfId:
            {
                size_t n = to_long(attr.value);
                mp_xf->set_style_xf(n);
                break;
            }
            case XML_applyBorder:
                break;
            case XML_applyFill:
                break;
            case XML_applyFont:
                break;
            case XML_applyNumberFormat:
                break;
            case XML_applyAlignment:
            {
                bool b = to_long(attr.value) != 0;
                mp_xf->set_apply_alignment(b);
                break;
            }
        }
    }
}

void xlsx_styles_context::end_element_number_format()
{
    if (!mp_styles)
        return;

    assert(mp_numfmt);
    mp_numfmt->commit();
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
