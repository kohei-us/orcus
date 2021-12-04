/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xlsx_context.hpp"
#include "ooxml_global.hpp"
#include "ooxml_token_constants.hpp"
#include "ooxml_namespace_types.hpp"
#include "ooxml_types.hpp"
#include "ooxml_schemas.hpp"
#include "xlsx_helper.hpp"
#include "xml_context_global.hpp"

#include "orcus/global.hpp"
#include "orcus/tokens.hpp"
#include "orcus/spreadsheet/import_interface.hpp"
#include "orcus/measurement.hpp"

#include <mdds/sorted_string_map.hpp>
#include <mdds/global.hpp>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>

using namespace std;

namespace orcus {

namespace {

class shared_strings_root_attr_parser
{
public:
    shared_strings_root_attr_parser() : m_count(0), m_unique_count(0) {}

    void operator() (const xml_token_attr_t &attr)
    {
        switch (attr.name)
        {
            case XML_count:
                m_count = to_long(attr.value);
            break;
            case XML_uniqueCount:
                m_unique_count = to_long(attr.value);
            break;
        }
    }

    shared_strings_root_attr_parser& operator= (const shared_strings_root_attr_parser& r)
    {
        m_count = r.m_count;
        m_unique_count = r.m_unique_count;
        return *this;
    }

    size_t get_count() const { return m_count; }
    size_t get_unique_count() const { return m_unique_count; }
private:
    size_t m_count;
    size_t m_unique_count;
};

class color_attr_parser
{
    pstring m_rgb;
public:
    void operator() (const xml_token_attr_t& attr)
    {
        switch (attr.name)
        {
            case XML_rgb:
                m_rgb = attr.value;
            break;
            case XML_theme:
                // TODO : handle this.
            break;
            default:
                ;
        }
    }

    pstring get_rgb() const { return m_rgb; }
};

}

xlsx_shared_strings_context::xlsx_shared_strings_context(session_context& session_cxt, const tokens& tokens, spreadsheet::iface::import_shared_strings* strings) :
    xml_context_base(session_cxt, tokens), mp_strings(strings), m_in_segments(false) {}

xlsx_shared_strings_context::~xlsx_shared_strings_context() {}

xml_context_base* xlsx_shared_strings_context::create_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/)
{
    return nullptr;
}

void xlsx_shared_strings_context::end_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/, xml_context_base* /*child*/)
{
}

void xlsx_shared_strings_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_attrs_t& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);
    switch (name)
    {
        case XML_sst:
        {
            // root element for the shared string part.
            xml_element_expected(parent, XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN);
            if (get_config().debug)
                print_attrs(get_tokens(), attrs);

            shared_strings_root_attr_parser func;
            func = for_each(attrs.begin(), attrs.end(), func);

            if (get_config().debug)
                cout << "count: " << func.get_count() << "  unique count: " << func.get_unique_count() << endl;
        }
        break;
        case XML_si:
            // single shared string entry.
            m_in_segments = false;
            xml_element_expected(parent, NS_ooxml_xlsx, XML_sst);
        break;
        case XML_r:
            // rich text run
            m_in_segments = true;
            xml_element_expected(parent, NS_ooxml_xlsx, XML_si);
        break;
        case XML_rPr:
            // rich text run property
            xml_element_expected(parent, NS_ooxml_xlsx, XML_r);
        break;
        case XML_b:
            // bold
            xml_element_expected(parent, NS_ooxml_xlsx, XML_rPr);
        break;
        case XML_i:
            // italic
            xml_element_expected(parent, NS_ooxml_xlsx, XML_rPr);
        break;
        case XML_sz:
        {
            // font size
            xml_element_expected(parent, NS_ooxml_xlsx, XML_rPr);
            pstring s = for_each(attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_val)).get_value();
            double point = to_double(s);
            mp_strings->set_segment_font_size(point);
        }
        break;
        case XML_color:
        {
            // font color
            xml_element_expected(parent, NS_ooxml_xlsx, XML_rPr);
            color_attr_parser func;
            func = for_each(attrs.begin(), attrs.end(), func);

            spreadsheet::color_elem_t alpha;
            spreadsheet::color_elem_t red;
            spreadsheet::color_elem_t green;
            spreadsheet::color_elem_t blue;
            if (to_rgb(func.get_rgb(), alpha, red, green, blue))
                mp_strings->set_segment_font_color(alpha, red, green, blue);
        }
        break;
        case XML_rFont:
        {
            // font
            xml_element_expected(parent, NS_ooxml_xlsx, XML_rPr);
            std::string_view font = for_each(attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_val)).get_value();
            mp_strings->set_segment_font_name(font);
        }
        break;
        case XML_family:
            // font family
            xml_element_expected(parent, NS_ooxml_xlsx, XML_rPr);
        break;
        case XML_scheme:
            // font scheme
            xml_element_expected(parent, NS_ooxml_xlsx, XML_rPr);
        break;
        case XML_t:
        {
            // actual text stored as its content.
            const xml_elem_set_t expected = {
                { NS_ooxml_xlsx, XML_r },
                { NS_ooxml_xlsx, XML_rPh },
                { NS_ooxml_xlsx, XML_si },
            };
            xml_element_expected(parent, expected);
        }
        break;
        default:
            warn_unhandled();
    }
}

bool xlsx_shared_strings_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    switch (name)
    {
        case XML_t:
        break;
        case XML_b:
            mp_strings->set_segment_bold(true);
        break;
        case XML_i:
            mp_strings->set_segment_italic(true);
        break;
        case XML_r:
            mp_strings->append_segment(m_cur_str);
        break;
        case XML_si:
        {
            if (m_in_segments)
                // commit all formatted segments.
                mp_strings->commit_segments();
            else
            {
                // unformatted text should only have one text segment.
                mp_strings->append(m_cur_str);
            }
        }
        break;
    }
    return pop_stack(ns, name);
}

void xlsx_shared_strings_context::characters(std::string_view str, bool transient)
{
    xml_token_pair_t cur_token = get_current_element();
    if (cur_token.first == NS_ooxml_xlsx && cur_token.second == XML_t)
    {
        m_cur_str = str;

        // In case the string contains carriage returns (CRs), remove them.
        m_cell_buffer.reset();
        const char* p = m_cur_str.data();
        const char* p_end = p + m_cur_str.size();
        const char* p0 = nullptr;

        for (; p != p_end; ++p)
        {
            if (!p0)
                p0 = p;

            if (*p == 0x0D)
            {
                // Append the segment up to this CR, and skip the CR.
                m_cell_buffer.append(p0, std::distance(p0, p));
                p0 = nullptr;
            }
        }

        if (!m_cell_buffer.empty())
        {
            // This string contains at least one CR.

            if (p0)
                // Append the tail end.
                m_cell_buffer.append(p0, std::distance(p0, p));

            m_cur_str = m_pool.intern({m_cell_buffer.get(), m_cell_buffer.size()}).first;
            transient = false;
        }

        if (transient)
            m_cur_str = m_pool.intern(m_cur_str).first;
    }
}

// ============================================================================

namespace {

namespace border_style {

typedef mdds::sorted_string_map<spreadsheet::border_style_t> map_type;

// Keys must be sorted.
const std::vector<map_type::entry> entries =
{
    { ORCUS_ASCII("dashDot"),          spreadsheet::border_style_t::dash_dot            },
    { ORCUS_ASCII("dashDotDot"),       spreadsheet::border_style_t::dash_dot_dot        },
    { ORCUS_ASCII("dashed"),           spreadsheet::border_style_t::dashed              },
    { ORCUS_ASCII("dotted"),           spreadsheet::border_style_t::dotted              },
    { ORCUS_ASCII("double"),           spreadsheet::border_style_t::double_border       },
    { ORCUS_ASCII("hair"),             spreadsheet::border_style_t::hair                },
    { ORCUS_ASCII("medium"),           spreadsheet::border_style_t::medium              },
    { ORCUS_ASCII("mediumDashDot"),    spreadsheet::border_style_t::medium_dash_dot     },
    { ORCUS_ASCII("mediumDashDotDot"), spreadsheet::border_style_t::medium_dash_dot_dot },
    { ORCUS_ASCII("mediumDashed"),     spreadsheet::border_style_t::medium_dashed       },
    { ORCUS_ASCII("none"),             spreadsheet::border_style_t::none                },
    { ORCUS_ASCII("slantDashDot"),     spreadsheet::border_style_t::slant_dash_dot      },
    { ORCUS_ASCII("thick"),            spreadsheet::border_style_t::thick               },
    { ORCUS_ASCII("thin"),             spreadsheet::border_style_t::thin                }
};

const map_type& get()
{
    static map_type mt(entries.data(), entries.size(), spreadsheet::border_style_t::none);
    return mt;
}

}

namespace fill_pattern {

typedef mdds::sorted_string_map<spreadsheet::fill_pattern_t> map_type;

// Keys must be sorted.
const std::vector<map_type::entry> entries =
{
    { ORCUS_ASCII("darkDown"),        spreadsheet::fill_pattern_t::dark_down        },
    { ORCUS_ASCII("darkGray"),        spreadsheet::fill_pattern_t::dark_gray        },
    { ORCUS_ASCII("darkGrid"),        spreadsheet::fill_pattern_t::dark_grid        },
    { ORCUS_ASCII("darkHorizontal"),  spreadsheet::fill_pattern_t::dark_horizontal  },
    { ORCUS_ASCII("darkTrellis"),     spreadsheet::fill_pattern_t::dark_trellis     },
    { ORCUS_ASCII("darkUp"),          spreadsheet::fill_pattern_t::dark_up          },
    { ORCUS_ASCII("darkVertical"),    spreadsheet::fill_pattern_t::dark_vertical    },
    { ORCUS_ASCII("gray0625"),        spreadsheet::fill_pattern_t::gray_0625        },
    { ORCUS_ASCII("gray125"),         spreadsheet::fill_pattern_t::gray_125         },
    { ORCUS_ASCII("lightDown"),       spreadsheet::fill_pattern_t::light_down       },
    { ORCUS_ASCII("lightGray"),       spreadsheet::fill_pattern_t::light_gray       },
    { ORCUS_ASCII("lightGrid"),       spreadsheet::fill_pattern_t::light_grid       },
    { ORCUS_ASCII("lightHorizontal"), spreadsheet::fill_pattern_t::light_horizontal },
    { ORCUS_ASCII("lightTrellis"),    spreadsheet::fill_pattern_t::light_trellis    },
    { ORCUS_ASCII("lightUp"),         spreadsheet::fill_pattern_t::light_up         },
    { ORCUS_ASCII("lightVertical"),   spreadsheet::fill_pattern_t::light_vertical   },
    { ORCUS_ASCII("mediumGray"),      spreadsheet::fill_pattern_t::medium_gray      },
    { ORCUS_ASCII("none"),            spreadsheet::fill_pattern_t::none             },
    { ORCUS_ASCII("solid"),           spreadsheet::fill_pattern_t::solid            },
};

const map_type& get()
{
    static map_type mt(entries.data(), entries.size(), spreadsheet::fill_pattern_t::none);
    return mt;
}

}

class border_attr_parser
{
    spreadsheet::border_direction_t m_dir;
    spreadsheet::iface::import_styles& m_styles;
public:
    border_attr_parser(spreadsheet::border_direction_t dir, spreadsheet::iface::import_styles& styles) :
        m_dir(dir), m_styles(styles) {}

    void operator() (const xml_token_attr_t& attr)
    {
        switch (attr.name)
        {
            case XML_style:
            {
                m_styles.set_border_style(m_dir,
                    border_style::get().find(attr.value.data(), attr.value.size()));
            }
            break;
        }
    }
};

class cell_style_attr_parser
{
    spreadsheet::iface::import_styles& m_styles;
public:
    cell_style_attr_parser(spreadsheet::iface::import_styles& styles) :
        m_styles(styles) {}

    void operator() (const xml_token_attr_t& attr)
    {
        switch (attr.name)
        {
            case XML_name:
                m_styles.set_cell_style_name(attr.value);
            break;
            case XML_xfId:
            {
                size_t n = to_long(attr.value);
                m_styles.set_cell_style_xf(n);
            }
            break;
            case XML_builtinId:
            {
                size_t n = to_long(attr.value);
                m_styles.set_cell_style_builtin(n);
            }
            break;
        }
    }
};

class xf_attr_parser
{
    spreadsheet::iface::import_styles& m_styles;
public:
    xf_attr_parser(spreadsheet::iface::import_styles& styles) :
        m_styles(styles) {}

    void operator() (const xml_token_attr_t& attr)
    {
        switch (attr.name)
        {
            case XML_borderId:
            {
                size_t n = to_long(attr.value);
                m_styles.set_xf_border(n);
            }
            break;
            case XML_fillId:
            {
                size_t n = to_long(attr.value);
                m_styles.set_xf_fill(n);
            }
            break;
            case XML_fontId:
            {
                size_t n = to_long(attr.value);
                m_styles.set_xf_font(n);
            }
            break;
            case XML_numFmtId:
            {
                size_t n = to_long(attr.value);
                m_styles.set_xf_number_format(n);
            }
            break;
            case XML_xfId:
            {
                size_t n = to_long(attr.value);
                m_styles.set_xf_style_xf(n);
            }
            break;
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
                m_styles.set_xf_apply_alignment(b);
            }
            break;
        }
    }
};

class fill_color_attr_parser
{
    spreadsheet::iface::import_styles& m_styles;
    const tokens& m_tokens;
    bool m_foreground;
    bool m_debug;
public:
    fill_color_attr_parser(spreadsheet::iface::import_styles& styles, const tokens& _tokens, bool fg, bool debug) :
        m_styles(styles), m_tokens(_tokens), m_foreground(fg), m_debug(debug) {}

    void operator() (const xml_token_attr_t& attr)
    {
        switch (attr.name)
        {
            case XML_rgb:
            {
                spreadsheet::color_elem_t alpha;
                spreadsheet::color_elem_t red;
                spreadsheet::color_elem_t green;
                spreadsheet::color_elem_t blue;
                if (!to_rgb(attr.value, alpha, red, green, blue))
                    // invalid RGB color format.
                    return;

                if (m_foreground)
                    m_styles.set_fill_fg_color(alpha, red, green, blue);
                else
                    m_styles.set_fill_bg_color(alpha, red, green, blue);
            }
            break;
            case XML_indexed:
            break;
            default:
                if (m_debug)
                    cerr << "warning: unknown attribute [ " << m_tokens.get_token_name(attr.name) << " ]" << endl;
        }
    }
};

class cell_protection_attr_parser
{
    spreadsheet::iface::import_styles& m_styles;
public:

    cell_protection_attr_parser(spreadsheet::iface::import_styles& styles) :
        m_styles(styles) {}

    void operator() (const xml_token_attr_t& attr)
    {
        switch (attr.name)
        {
            case XML_hidden:
            {
                bool b = to_long(attr.value) != 0;
                m_styles.set_cell_hidden(b);
            }
            break;
            case XML_locked:
            {
                bool b = to_long(attr.value) != 0;
                m_styles.set_cell_locked(b);
            }
            break;
        }
    }
};

class cell_alignment_attr_parser
{
    spreadsheet::hor_alignment_t m_hor_align;
    spreadsheet::ver_alignment_t m_ver_align;

public:
    cell_alignment_attr_parser() :
        m_hor_align(spreadsheet::hor_alignment_t::unknown),
        m_ver_align(spreadsheet::ver_alignment_t::bottom) // 'bottom' is the default if no vertical alignment is given.
    {}

    void operator() (const xml_token_attr_t& attr)
    {
        switch (attr.name)
        {
            case XML_horizontal:
            {
                if (attr.value == "center")
                    m_hor_align = spreadsheet::hor_alignment_t::center;
                else if (attr.value == "right")
                    m_hor_align = spreadsheet::hor_alignment_t::right;
                else if (attr.value == "left")
                    m_hor_align = spreadsheet::hor_alignment_t::left;
                else if (attr.value == "justify")
                    m_hor_align = spreadsheet::hor_alignment_t::justified;
                else if (attr.value == "distributed")
                    m_hor_align = spreadsheet::hor_alignment_t::distributed;
            }
            break;
            case XML_vertical:
            {
                if (attr.value == "top")
                    m_ver_align = spreadsheet::ver_alignment_t::top;
                else if (attr.value == "center")
                    m_ver_align = spreadsheet::ver_alignment_t::middle;
                else if (attr.value == "bottom")
                    m_ver_align = spreadsheet::ver_alignment_t::bottom;
                else if (attr.value == "justify")
                    m_ver_align = spreadsheet::ver_alignment_t::justified;
                else if (attr.value == "distributed")
                    m_ver_align = spreadsheet::ver_alignment_t::distributed;
            }
            break;
            default:
                ;
        }
    }

    spreadsheet::hor_alignment_t get_hor_align() const
    {
        return m_hor_align;
    }

    spreadsheet::ver_alignment_t get_ver_align() const
    {
        return m_ver_align;
    }
};

}

xlsx_styles_context::xlsx_styles_context(session_context& session_cxt, const tokens& tokens, spreadsheet::iface::import_styles* styles) :
    xml_context_base(session_cxt, tokens),
    mp_styles(styles),
    m_diagonal_up(false), m_diagonal_down(false),
    m_cur_border_dir(spreadsheet::border_direction_t::unknown),
    m_cell_style_xf(false) {}

xlsx_styles_context::~xlsx_styles_context() {}

xml_context_base* xlsx_styles_context::create_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/)
{
    return nullptr;
}

void xlsx_styles_context::end_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/, xml_context_base* /*child*/)
{
}

void xlsx_styles_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_attrs_t& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);

    if (ns == NS_ooxml_xlsx)
    {
        switch (name)
        {
            case XML_styleSheet:
            {
                // root element
                xml_element_expected(parent, XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN);
                if (get_config().debug)
                    print_attrs(get_tokens(), attrs);
            }
            break;
            case XML_fonts:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_styleSheet);
                pstring ps = for_each(attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_count)).get_value();
                size_t font_count = to_long(ps);
                mp_styles->set_font_count(font_count);
            }
            break;
            case XML_font:
            {
                xml_elem_stack_t expected_elements;
                expected_elements.push_back(xml_token_pair_t(NS_ooxml_xlsx, XML_fonts));
                expected_elements.push_back(xml_token_pair_t(NS_ooxml_xlsx, XML_dxf));
                xml_element_expected(parent, expected_elements);
            }
            break;
            case XML_b:
                xml_element_expected(parent, NS_ooxml_xlsx, XML_font);
                mp_styles->set_font_bold(true);
            break;
            case XML_i:
                xml_element_expected(parent, NS_ooxml_xlsx, XML_font);
                mp_styles->set_font_italic(true);
            break;
            case XML_u:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_font);
                pstring ps = for_each(attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_val)).get_value();
                if (ps == "double")
                    mp_styles->set_font_underline(spreadsheet::underline_t::double_line);
                else if (ps == "single")
                    mp_styles->set_font_underline(spreadsheet::underline_t::single_line);
                else if (ps == "singleAccounting")
                    mp_styles->set_font_underline(spreadsheet::underline_t::single_accounting);
                else if (ps == "doubleAccounting")
                    mp_styles->set_font_underline(spreadsheet::underline_t::double_accounting);
            }
            break;
            case XML_sz:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_font);
                pstring ps = for_each(attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_val)).get_value();
                double font_size = to_double(ps);
                mp_styles->set_font_size(font_size);
            }
            break;
            case XML_color:
            {
                // The color element can occur under various parent elements.
                const xml_elem_set_t expected = {
                    { NS_ooxml_xlsx, XML_bottom },
                    { NS_ooxml_xlsx, XML_diagonal },
                    { NS_ooxml_xlsx, XML_end },
                    { NS_ooxml_xlsx, XML_font },
                    { NS_ooxml_xlsx, XML_horizontal },
                    { NS_ooxml_xlsx, XML_left },
                    { NS_ooxml_xlsx, XML_mruColors },
                    { NS_ooxml_xlsx, XML_right },
                    { NS_ooxml_xlsx, XML_start },
                    { NS_ooxml_xlsx, XML_stop },
                    { NS_ooxml_xlsx, XML_top },
                    { NS_ooxml_xlsx, XML_vertical },
                };
                xml_element_expected(parent, expected);

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
            }
            break;
            case XML_name:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_font);
                std::string_view ps = for_each(attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_val)).get_value();
                mp_styles->set_font_name(ps);
            }
            break;
            case XML_family:
                xml_element_expected(parent, NS_ooxml_xlsx, XML_font);
            break;
            case XML_scheme:
                xml_element_expected(parent, NS_ooxml_xlsx, XML_font);
            break;
            case XML_fills:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_styleSheet);
                pstring ps = for_each(attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_count)).get_value();
                size_t fill_count = to_long(ps);
                mp_styles->set_fill_count(fill_count);
            }
            break;
            case XML_fill:
            {
                xml_elem_stack_t expected = {
                    xml_token_pair_t(NS_ooxml_xlsx, XML_fills),
                    xml_token_pair_t(NS_ooxml_xlsx, XML_dxf)
                };

                xml_element_expected(parent, expected);
            }
            break;
            case XML_patternFill:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_fill);
                pstring ps = for_each(attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_patternType)).get_value();
                mp_styles->set_fill_pattern_type(fill_pattern::get().find(ps.data(), ps.size()));
            }
            break;
            case XML_fgColor:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_patternFill);
                for_each(
                    attrs.begin(), attrs.end(),
                    fill_color_attr_parser(
                        *mp_styles, get_tokens(), true, get_config().debug));
            }
            break;
            case XML_bgColor:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_patternFill);
                for_each(
                    attrs.begin(), attrs.end(),
                    fill_color_attr_parser(
                        *mp_styles, get_tokens(), false, get_config().debug));
            }
            break;
            case XML_borders:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_styleSheet);
                pstring ps = for_each(attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_count)).get_value();
                size_t border_count = to_long(ps);
                mp_styles->set_border_count(border_count);
            }
            break;
            case XML_border:
            {
                start_element_border(parent, attrs);
                break;
            }
            case XML_top:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_border);
                m_cur_border_dir = spreadsheet::border_direction_t::top;
                border_attr_parser func(spreadsheet::border_direction_t::top, *mp_styles);
                for_each(attrs.begin(), attrs.end(), func);
            }
            break;
            case XML_bottom:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_border);
                m_cur_border_dir = spreadsheet::border_direction_t::bottom;
                border_attr_parser func(spreadsheet::border_direction_t::bottom, *mp_styles);
                for_each(attrs.begin(), attrs.end(), func);
            }
            break;
            case XML_left:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_border);
                m_cur_border_dir = spreadsheet::border_direction_t::left;
                border_attr_parser func(spreadsheet::border_direction_t::left, *mp_styles);
                for_each(attrs.begin(), attrs.end(), func);
            }
            break;
            case XML_right:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_border);
                m_cur_border_dir = spreadsheet::border_direction_t::right;
                border_attr_parser func(spreadsheet::border_direction_t::right, *mp_styles);
                for_each(attrs.begin(), attrs.end(), func);
            }
            break;
            case XML_diagonal:
            {
                start_element_diagonal(parent, attrs);
                break;
            }
            case XML_cellStyleXfs:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_styleSheet);
                pstring ps = for_each(
                    attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_count)).get_value();
                if (!ps.empty())
                {
                    size_t n = strtoul(ps.data(), nullptr, 10);
                    mp_styles->set_cell_style_xf_count(n);
                }
                m_cell_style_xf = true;
            }
            break;
            case XML_cellXfs:
            {
                // Collection of un-named cell formats used in the document.
                xml_element_expected(parent, NS_ooxml_xlsx, XML_styleSheet);
                pstring ps = for_each(
                    attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_count)).get_value();
                if (!ps.empty())
                {
                    size_t n = strtoul(ps.data(), nullptr, 10);
                    mp_styles->set_cell_xf_count(n);
                }
                m_cell_style_xf = false;
            }
            break;
            case XML_dxfs:
            {
                // Collection of differential formats used in the document.
                xml_element_expected(parent, NS_ooxml_xlsx, XML_styleSheet);
                pstring ps = for_each(
                    attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_count)).get_value();
                if (!ps.empty())
                {
                    size_t n = strtoul(ps.data(), nullptr, 10);
                    mp_styles->set_dxf_count(n);
                }
            }
            break;
            case XML_cellStyles:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_styleSheet);
                pstring ps = for_each(
                    attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_count)).get_value();
                if (!ps.empty())
                {
                    size_t n = strtoul(ps.data(), nullptr, 10);
                    mp_styles->set_cell_style_count(n);
                }
            }
            break;
            case XML_cellStyle:
            {
                // named cell style, some of which are built-in such as 'Normal'.
                xml_element_expected(parent, NS_ooxml_xlsx, XML_cellStyles);
                for_each(attrs.begin(), attrs.end(), cell_style_attr_parser(*mp_styles));
            }
            break;
            case XML_xf:
            {
                // Actual cell format attributes (for some reason) abbreviated to
                // 'xf'.  Used both by cells and cell styles.
                xml_elem_stack_t allowed;
                allowed.push_back(xml_elem_stack_t::value_type(NS_ooxml_xlsx, XML_cellXfs));
                allowed.push_back(xml_elem_stack_t::value_type(NS_ooxml_xlsx, XML_cellStyleXfs));
                xml_element_expected(parent, allowed);

                for_each(attrs.begin(), attrs.end(), xf_attr_parser(*mp_styles));
            }
            break;
            case XML_dxf:
            break;
            case XML_protection:
            {
                xml_elem_stack_t expected_elements;
                expected_elements.push_back(xml_token_pair_t(NS_ooxml_xlsx, XML_xf));
                expected_elements.push_back(xml_token_pair_t(NS_ooxml_xlsx, XML_dxf));
                xml_element_expected(parent, expected_elements);
                for_each(attrs.begin(), attrs.end(), cell_protection_attr_parser(*mp_styles));
            }
            break;
            case XML_alignment:
            {
                xml_elem_stack_t expected_elements;
                expected_elements.push_back(xml_token_pair_t(NS_ooxml_xlsx, XML_xf));
                expected_elements.push_back(xml_token_pair_t(NS_ooxml_xlsx, XML_dxf));
                xml_element_expected(parent, expected_elements);
                cell_alignment_attr_parser func;
                func = for_each(attrs.begin(), attrs.end(), func);
                mp_styles->set_xf_horizontal_alignment(func.get_hor_align());
                mp_styles->set_xf_vertical_alignment(func.get_ver_align());
            }
            break;
            case XML_numFmts:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_styleSheet);
                pstring val =
                    for_each(
                        attrs.begin(), attrs.end(),
                        single_attr_getter(m_pool, NS_ooxml_xlsx, XML_count)).get_value();
                if (!val.empty())
                {
                    size_t n = to_long(val);
                    mp_styles->set_number_format_count(n);
                }
            }
            break;
            case XML_numFmt:
                start_element_number_format(parent, attrs);
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
            mp_styles->commit_font();
        break;
        case XML_fill:
            mp_styles->commit_fill();
        break;
        case XML_border:
            mp_styles->commit_border();
        break;
        case XML_cellStyle:
            mp_styles->commit_cell_style();
        break;
        case XML_xf:
            if (m_cell_style_xf)
                mp_styles->commit_cell_style_xf();
            else
                mp_styles->commit_cell_xf();
        break;
        case XML_dxf:
            mp_styles->commit_dxf();
        break;
        case XML_protection:
        {
            size_t id = mp_styles->commit_cell_protection();
            mp_styles->set_xf_protection(id);
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

void xlsx_styles_context::start_element_number_format(const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    xml_elem_stack_t expected_elements;
    expected_elements.push_back(xml_token_pair_t(NS_ooxml_xlsx, XML_numFmts));
    expected_elements.push_back(xml_token_pair_t(NS_ooxml_xlsx, XML_dxf));
    xml_element_expected(parent, expected_elements);

    if (mp_styles)
    {
        for (const xml_token_attr_t& attr : attrs)
        {
            if (attr.ns && attr.ns != NS_ooxml_xlsx)
                continue;

            switch (attr.name)
            {
                case XML_numFmtId:
                {
                    long id = to_long(attr.value);
                    mp_styles->set_number_format_identifier(id);
                    break;
                }
                case XML_formatCode:
                {
                    mp_styles->set_number_format_code(attr.value);
                    break;
                }
            }
        }
    }
}

void xlsx_styles_context::start_element_border(const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    xml_elem_stack_t expected_elements;
    expected_elements.push_back(xml_token_pair_t(NS_ooxml_xlsx, XML_borders));
    expected_elements.push_back(xml_token_pair_t(NS_ooxml_xlsx, XML_dxf));
    xml_element_expected(parent, expected_elements);

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

void xlsx_styles_context::start_element_diagonal(const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    xml_element_expected(parent, NS_ooxml_xlsx, XML_border);

    m_cur_border_dir = spreadsheet::border_direction_t::unknown;

    if (m_diagonal_up)
    {
        m_cur_border_dir = m_diagonal_down ?
            spreadsheet::border_direction_t::diagonal :
            spreadsheet::border_direction_t::diagonal_bl_tr;
    }
    else
    {
        m_cur_border_dir = m_diagonal_down ?
            spreadsheet::border_direction_t::diagonal_tl_br :
            spreadsheet::border_direction_t::unknown;
    }

    if (m_cur_border_dir == spreadsheet::border_direction_t::unknown)
        return;

    border_attr_parser func(m_cur_border_dir, *mp_styles);
    for_each(attrs.begin(), attrs.end(), func);
}

void xlsx_styles_context::start_border_color(const xml_attrs_t& attrs)
{
    color_attr_parser func;
    func = for_each(attrs.begin(), attrs.end(), func);

    spreadsheet::color_elem_t alpha;
    spreadsheet::color_elem_t red;
    spreadsheet::color_elem_t green;
    spreadsheet::color_elem_t blue;
    if (to_rgb(func.get_rgb(), alpha, red, green, blue))
        mp_styles->set_border_color(m_cur_border_dir, alpha, red, green, blue);
}

void xlsx_styles_context::start_font_color(const xml_attrs_t& attrs)
{
    color_attr_parser func;
    func = for_each(attrs.begin(), attrs.end(), func);

    spreadsheet::color_elem_t alpha;
    spreadsheet::color_elem_t red;
    spreadsheet::color_elem_t green;
    spreadsheet::color_elem_t blue;
    if (to_rgb(func.get_rgb(), alpha, red, green, blue))
        mp_styles->set_font_color(alpha, red, green, blue);
}

void xlsx_styles_context::end_element_number_format()
{
    if (!mp_styles)
        return;

    mp_styles->commit_number_format();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
