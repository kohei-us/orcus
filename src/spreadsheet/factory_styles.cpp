/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/spreadsheet/factory.hpp"
#include "orcus/spreadsheet/styles.hpp"

namespace orcus { namespace spreadsheet {

import_styles::import_styles(orcus::spreadsheet::styles& styles) : m_styles(styles) {}
import_styles::~import_styles() {}

void import_styles::set_font_count(size_t n)
{
    m_styles.set_font_count(n);
}

void import_styles::set_font_bold(bool b)
{
    m_styles.set_font_bold(b);
}

void import_styles::set_font_italic(bool b)
{
    m_styles.set_font_italic(b);
}

void import_styles::set_font_name(const char* s, size_t n)
{
    m_styles.set_font_name(s, n);
}

void import_styles::set_font_size(double point)
{
    m_styles.set_font_size(point);
}

void import_styles::set_font_underline(underline_t e)
{
    m_styles.set_font_underline(e);
}

void import_styles::set_font_underline_width(underline_width_t e)
{
    m_styles.set_font_underline_width(e);
}

void import_styles::set_font_underline_mode(underline_mode_t e)
{
    m_styles.set_font_underline_mode(e);
}

void import_styles::set_font_underline_type(underline_type_t e)
{
    m_styles.set_font_underline_type(e);
}

void import_styles::set_font_underline_color(
    color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    m_styles.set_font_underline_color(alpha, red, green, blue);
}

void import_styles::set_font_color(
    color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    m_styles.set_font_color(alpha, red, green, blue);
}

void import_styles::set_strikethrough_style(strikethrough_style_t s)
{
    m_styles.set_strikethrough_style(s);
}

void import_styles::set_strikethrough_type(strikethrough_type_t s)
{
    m_styles.set_strikethrough_type(s);
}

void import_styles::set_strikethrough_width(strikethrough_width_t s)
{
    m_styles.set_strikethrough_width(s);
}

void import_styles::set_strikethrough_text(strikethrough_text_t s)
{
    m_styles.set_strikethrough_text(s);
}

size_t import_styles::commit_font()
{
    return m_styles.commit_font();
}

void import_styles::set_fill_count(size_t n)
{
    m_styles.set_fill_count(n);
}

void import_styles::set_fill_pattern_type(fill_pattern_t fp)
{
    m_styles.set_fill_pattern_type(fp);
}

void import_styles::set_fill_fg_color(
    color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    m_styles.set_fill_fg_color(alpha, red, green, blue);
}

void import_styles::set_fill_bg_color(
    color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    m_styles.set_fill_bg_color(alpha, red, green, blue);
}

size_t import_styles::commit_fill()
{
    return m_styles.commit_fill();
}

void import_styles::set_border_count(size_t n)
{
    m_styles.set_border_count(n);
}

void import_styles::set_border_style(border_direction_t dir, border_style_t style)
{
    m_styles.set_border_style(dir, style);
}

void import_styles::set_border_color(
    border_direction_t dir, color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    m_styles.set_border_color(dir, alpha, red, green, blue);
}

void import_styles::set_border_width(border_direction_t dir, double width, orcus::length_unit_t unit)
{
    m_styles.set_border_width(dir, width, unit);
}

size_t import_styles::commit_border()
{
    return m_styles.commit_border();
}

void import_styles::set_cell_hidden(bool b)
{
    m_styles.set_cell_hidden(b);
}

void import_styles::set_cell_locked(bool b)
{
    m_styles.set_cell_locked(b);
}

void import_styles::set_cell_print_content(bool b)
{
    m_styles.set_cell_print_content(b);
}

void import_styles::set_cell_formula_hidden(bool b)
{
    m_styles.set_cell_formula_hidden(b);
}

size_t import_styles::commit_cell_protection()
{
    return m_styles.commit_cell_protection();
}

void import_styles::set_number_format_count(size_t n)
{
    m_styles.set_number_format_count(n);
}

void import_styles::set_number_format_identifier(size_t id)
{
    m_styles.set_number_format_identifier(id);
}

void import_styles::set_number_format_code(const char* s, size_t n)
{
    m_styles.set_number_format_code(s, n);
}

size_t import_styles::commit_number_format()
{
    return m_styles.commit_number_format();
}

void import_styles::set_cell_xf_count(size_t n)
{
    m_styles.set_cell_xf_count(n);
}

void import_styles::set_cell_style_xf_count(size_t n)
{
    m_styles.set_cell_style_xf_count(n);
}

void import_styles::set_dxf_count(size_t n)
{
    m_styles.set_dxf_count(n);
}

void import_styles::set_xf_font(size_t index)
{
    m_styles.set_xf_font(index);
}

void import_styles::set_xf_fill(size_t index)
{
    m_styles.set_xf_fill(index);
}

void import_styles::set_xf_border(size_t index)
{
    m_styles.set_xf_border(index);
}

void import_styles::set_xf_protection(size_t index)
{
    m_styles.set_xf_protection(index);
}

void import_styles::set_xf_number_format(size_t index)
{
    m_styles.set_xf_number_format(index);
}

void import_styles::set_xf_style_xf(size_t index)
{
    m_styles.set_xf_style_xf(index);
}

void import_styles::set_xf_apply_alignment(bool b)
{
    m_styles.set_xf_apply_alignment(b);
}

void import_styles::set_xf_horizontal_alignment(hor_alignment_t align)
{
    m_styles.set_xf_horizontal_alignment(align);
}

void import_styles::set_xf_vertical_alignment(ver_alignment_t align)
{
    m_styles.set_xf_vertical_alignment(align);
}

size_t import_styles::commit_cell_xf()
{
    return m_styles.commit_cell_xf();
}

size_t import_styles::commit_cell_style_xf()
{
    return m_styles.commit_cell_style_xf();
}

size_t import_styles::commit_dxf()
{
    return m_styles.commit_dxf();
}

void import_styles::set_cell_style_count(size_t n)
{
    m_styles.set_cell_style_count(n);
}

void import_styles::set_cell_style_name(const char* s, size_t n)
{
    m_styles.set_cell_style_name(s, n);
}

void import_styles::set_cell_style_xf(size_t index)
{
    m_styles.set_cell_style_xf(index);
}

void import_styles::set_cell_style_builtin(size_t index)
{
    m_styles.set_cell_style_builtin(index);
}

void import_styles::set_cell_style_parent_name(const char* s, size_t n)
{
    m_styles.set_cell_style_parent_name(s, n);
}

size_t import_styles::commit_cell_style()
{
    return m_styles.commit_cell_style();
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
