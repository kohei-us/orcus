/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/spreadsheet/factory.hpp"
#include "orcus/spreadsheet/styles.hpp"
#include "orcus/global.hpp"
#include "orcus/string_pool.hpp"

namespace orcus { namespace spreadsheet {

struct import_styles::impl
{
    styles& m_styles;
    string_pool& m_string_pool;

    font_t m_cur_font;
    fill_t m_cur_fill;
    border_t m_cur_border;
    protection_t m_cur_protection;
    number_format_t m_cur_number_format;
    cell_format_t m_cur_cell_format;
    cell_style_t m_cur_cell_style;

    impl(styles& styles, string_pool& sp) :
        m_styles(styles),
        m_string_pool(sp) {}
};

import_styles::import_styles(styles& styles, string_pool& sp) :
    mp_impl(orcus::make_unique<impl>(styles, sp)) {}

import_styles::~import_styles() {}

void import_styles::set_font_count(size_t n)
{
    mp_impl->m_styles.reserve_font_store(n);
}

void import_styles::set_font_bold(bool b)
{
    mp_impl->m_cur_font.bold = b;
}

void import_styles::set_font_italic(bool b)
{
    mp_impl->m_cur_font.italic = b;
}

void import_styles::set_font_name(const char* s, size_t n)
{
    mp_impl->m_cur_font.name = mp_impl->m_string_pool.intern(s, n).first;
}

void import_styles::set_font_size(double point)
{
    mp_impl->m_cur_font.size = point;
}

void import_styles::set_font_underline(underline_t e)
{
    mp_impl->m_cur_font.underline_style = e;
}

void import_styles::set_font_underline_width(underline_width_t e)
{
    mp_impl->m_cur_font.underline_width = e;
}

void import_styles::set_font_underline_mode(underline_mode_t e)
{
    mp_impl->m_cur_font.underline_mode = e;
}

void import_styles::set_font_underline_type(underline_type_t e)
{
    mp_impl->m_cur_font.underline_type = e;
}

void import_styles::set_font_underline_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    mp_impl->m_cur_font.underline_color = color_t(alpha, red, green, blue);
}

void import_styles::set_font_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    mp_impl->m_cur_font.color = color_t(alpha, red, green, blue);
}

void import_styles::set_strikethrough_style(strikethrough_style_t s)
{
    mp_impl->m_cur_font.strikethrough_style = s;
}

void import_styles::set_strikethrough_width(strikethrough_width_t s)
{
    mp_impl->m_cur_font.strikethrough_width = s;
}

void import_styles::set_strikethrough_type(strikethrough_type_t s)
{
    mp_impl->m_cur_font.strikethrough_type = s;
}

void import_styles::set_strikethrough_text(strikethrough_text_t s)
{
    mp_impl->m_cur_font.strikethrough_text = s;
}

size_t import_styles::commit_font()
{
    size_t font_id = mp_impl->m_styles.append_font(mp_impl->m_cur_font);
    mp_impl->m_cur_font.reset();
    return font_id;
}

void import_styles::set_fill_count(size_t n)
{
    mp_impl->m_styles.reserve_fill_store(n);
}

void import_styles::set_fill_pattern_type(fill_pattern_t fp)
{
    mp_impl->m_cur_fill.pattern_type = fp;
}

void import_styles::set_fill_fg_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    mp_impl->m_cur_fill.fg_color = color_t(alpha, red, green, blue);
}

void import_styles::set_fill_bg_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    mp_impl->m_cur_fill.bg_color = color_t(alpha, red, green, blue);
}

size_t import_styles::commit_fill()
{
    size_t fill_id = mp_impl->m_styles.append_fill(mp_impl->m_cur_fill);
    mp_impl->m_cur_fill.reset();
    return fill_id;
}

void import_styles::set_border_count(size_t n)
{
    mp_impl->m_styles.reserve_border_store(n);
}

namespace {

border_attrs_t* get_border_attrs(border_t& cur_border, border_direction_t dir)
{
    border_attrs_t* p = nullptr;
    switch (dir)
    {
        case border_direction_t::top:
            p = &cur_border.top;
        break;
        case border_direction_t::bottom:
            p = &cur_border.bottom;
        break;
        case border_direction_t::left:
            p = &cur_border.left;
        break;
        case border_direction_t::right:
            p = &cur_border.right;
        break;
        case border_direction_t::diagonal:
            p = &cur_border.diagonal;
        break;
        case border_direction_t::diagonal_bl_tr:
            p = &cur_border.diagonal_bl_tr;
        break;
        case border_direction_t::diagonal_tl_br:
            p = &cur_border.diagonal_tl_br;
        break;
        default:
            ;
    }

    return p;
}

}

void import_styles::set_border_style(border_direction_t dir, border_style_t style)
{
    border_attrs_t* p = get_border_attrs(mp_impl->m_cur_border, dir);
    if (p)
        p->style = style;
}

void import_styles::set_border_color(
    border_direction_t dir, color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    border_attrs_t* p = get_border_attrs(mp_impl->m_cur_border, dir);
    if (p)
        p->border_color = color_t(alpha, red, green, blue);
}

void import_styles::set_border_width(border_direction_t dir, double width, orcus::length_unit_t unit)
{
    border_attrs_t* p = get_border_attrs(mp_impl->m_cur_border, dir);
    if (p)
    {
        p->border_width.value = width;
        p->border_width.unit = unit;
    }
}

size_t import_styles::commit_border()
{
    size_t border_id = mp_impl->m_styles.append_border(mp_impl->m_cur_border);
    mp_impl->m_cur_border.reset();
    return border_id;
}

void import_styles::set_cell_hidden(bool b)
{
    mp_impl->m_cur_protection.hidden = b;
}

void import_styles::set_cell_locked(bool b)
{
    mp_impl->m_cur_protection.locked = b;
}

void import_styles::set_cell_print_content(bool b)
{
    mp_impl->m_cur_protection.print_content = b;
}

void import_styles::set_cell_formula_hidden(bool b)
{
    mp_impl->m_cur_protection.formula_hidden = b;
}

size_t import_styles::commit_cell_protection()
{
    size_t cp_id = mp_impl->m_styles.append_protection(mp_impl->m_cur_protection);
    mp_impl->m_cur_protection.reset();
    return cp_id;
}

void import_styles::set_number_format_count(size_t n)
{
    mp_impl->m_styles.reserve_number_format_store(n);
}

void import_styles::set_number_format_identifier(size_t id)
{
    mp_impl->m_cur_number_format.identifier = id;
}

void import_styles::set_number_format_code(const char* s, size_t n)
{
    mp_impl->m_cur_number_format.format_string = pstring(s, n);
}

size_t import_styles::commit_number_format()
{
    size_t nf_id = mp_impl->m_styles.append_number_format(mp_impl->m_cur_number_format);
    mp_impl->m_cur_number_format.reset();
    return nf_id;
}

void import_styles::set_cell_xf_count(size_t n)
{
    mp_impl->m_styles.reserve_cell_format_store(n);
}

void import_styles::set_cell_style_xf_count(size_t n)
{
    mp_impl->m_styles.reserve_cell_style_format_store(n);
}

void import_styles::set_dxf_count(size_t n)
{
    mp_impl->m_styles.reserve_diff_cell_format_store(n);
}

void import_styles::set_xf_font(size_t index)
{
    mp_impl->m_cur_cell_format.font = index;
}

void import_styles::set_xf_fill(size_t index)
{
    mp_impl->m_cur_cell_format.fill = index;
}

void import_styles::set_xf_border(size_t index)
{
    mp_impl->m_cur_cell_format.border = index;

    // TODO : we need to decide whether to have interface methods for these
    // apply_foo attributes.  For now there is only one, for alignment.
    mp_impl->m_cur_cell_format.apply_border = index > 0;
}

void import_styles::set_xf_protection(size_t index)
{
    mp_impl->m_cur_cell_format.protection = index;
}

void import_styles::set_xf_number_format(size_t index)
{
    mp_impl->m_cur_cell_format.number_format = index;
}

void import_styles::set_xf_style_xf(size_t index)
{
    mp_impl->m_cur_cell_format.style_xf = index;
}

void import_styles::set_xf_apply_alignment(bool b)
{
    mp_impl->m_cur_cell_format.apply_alignment = b;
}

void import_styles::set_xf_horizontal_alignment(orcus::spreadsheet::hor_alignment_t align)
{
    mp_impl->m_cur_cell_format.hor_align = align;
}

void import_styles::set_xf_vertical_alignment(orcus::spreadsheet::ver_alignment_t align)
{
    mp_impl->m_cur_cell_format.ver_align = align;
}

size_t import_styles::commit_cell_xf()
{
    size_t n = mp_impl->m_styles.append_cell_format(mp_impl->m_cur_cell_format);
    mp_impl->m_cur_cell_format.reset();
    return n;
}

size_t import_styles::commit_cell_style_xf()
{
    size_t n = mp_impl->m_styles.append_cell_style_format(mp_impl->m_cur_cell_format);
    mp_impl->m_cur_cell_format.reset();
    return n;
}

size_t import_styles::commit_dxf()
{
    size_t n = mp_impl->m_styles.append_diff_cell_format(mp_impl->m_cur_cell_format);
    mp_impl->m_cur_cell_format.reset();
    return n;
}

void import_styles::set_cell_style_count(size_t n)
{
    mp_impl->m_styles.reserve_cell_style_store(n);
}

void import_styles::set_cell_style_name(const char* s, size_t n)
{
    mp_impl->m_cur_cell_style.name =
        mp_impl->m_string_pool.intern(s, n).first;
}

void import_styles::set_cell_style_xf(size_t index)
{
    mp_impl->m_cur_cell_style.xf = index;
}

void import_styles::set_cell_style_builtin(size_t index)
{
    mp_impl->m_cur_cell_style.builtin = index;
}

void import_styles::set_cell_style_parent_name(const char* s, size_t n)
{
    mp_impl->m_cur_cell_style.parent_name =
        mp_impl->m_string_pool.intern(s, n).first;
}

size_t import_styles::commit_cell_style()
{
    size_t n = mp_impl->m_styles.append_cell_style(mp_impl->m_cur_cell_style);
    mp_impl->m_cur_cell_style.reset();
    return n;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
