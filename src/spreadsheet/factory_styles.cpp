/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "factory_styles.hpp"

namespace orcus { namespace spreadsheet {

_import_styles::~_import_styles() {}

void _import_styles::set_font_count(size_t n) {}
void _import_styles::set_font_bold(bool b) {}
void _import_styles::set_font_italic(bool b) {}
void _import_styles::set_font_name(const char* s, size_t n) {}
void _import_styles::set_font_size(double point) {}
void _import_styles::set_font_underline(orcus::spreadsheet::underline_t e) {}
void _import_styles::set_font_underline_width(underline_width_t e) {}
void _import_styles::set_font_underline_mode(underline_mode_t e) {}
void _import_styles::set_font_underline_type(underline_type_t e) {}
void _import_styles::set_font_underline_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) {}
void _import_styles::set_font_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) {}
void _import_styles::set_strikethrough_style(orcus::spreadsheet::strikethrough_style_t s) {}
void _import_styles::set_strikethrough_type(orcus::spreadsheet::strikethrough_type_t s) {}
void _import_styles::set_strikethrough_width(orcus::spreadsheet::strikethrough_width_t s) {}
void _import_styles::set_strikethrough_text(orcus::spreadsheet::strikethrough_text_t s) {}
size_t _import_styles::commit_font() { return 0; }

void _import_styles::set_fill_count(size_t n) {}
void _import_styles::set_fill_pattern_type(orcus::spreadsheet::fill_pattern_t fp) {}
void _import_styles::set_fill_fg_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) {}
void _import_styles::set_fill_bg_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) {}
size_t _import_styles::commit_fill() { return 0; }

void _import_styles::set_border_count(size_t n) {}
void _import_styles::set_border_style(orcus::spreadsheet::border_direction_t dir, border_style_t style) {}
void _import_styles::set_border_color(
    orcus::spreadsheet::border_direction_t dir, color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) {}
void _import_styles::set_border_width(orcus::spreadsheet::border_direction_t dir, double width, orcus::length_unit_t unit) {}
size_t _import_styles::commit_border() { return 0; }

void _import_styles::set_cell_hidden(bool b) {}
void _import_styles::set_cell_locked(bool b) {}
void _import_styles::set_cell_print_content(bool b) {}
void _import_styles::set_cell_formula_hidden(bool b) {}
size_t _import_styles::commit_cell_protection() { return 0; }

void _import_styles::set_number_format_count(size_t n) {}
void _import_styles::set_number_format_identifier(size_t id) {}
void _import_styles::set_number_format_code(const char* s, size_t n) {}
size_t _import_styles::commit_number_format() { return 0; }

void _import_styles::set_cell_xf_count(size_t n) {}
void _import_styles::set_cell_style_xf_count(size_t n) {}
void _import_styles::set_dxf_count(size_t n) {}

void _import_styles::set_xf_font(size_t index) {}
void _import_styles::set_xf_fill(size_t index) {}
void _import_styles::set_xf_border(size_t index) {}
void _import_styles::set_xf_protection(size_t index) {}
void _import_styles::set_xf_number_format(size_t index) {}
void _import_styles::set_xf_style_xf(size_t index) {}
void _import_styles::set_xf_apply_alignment(bool b) {}
void _import_styles::set_xf_horizontal_alignment(orcus::spreadsheet::hor_alignment_t align) {}
void _import_styles::set_xf_vertical_alignment(orcus::spreadsheet::ver_alignment_t align) {}

size_t _import_styles::commit_cell_xf() { return 0; }
size_t _import_styles::commit_cell_style_xf() { return 0; }
size_t _import_styles::commit_dxf() { return 0; }

void _import_styles::set_cell_style_count(size_t n) {}
void _import_styles::set_cell_style_name(const char* s, size_t n) {}
void _import_styles::set_cell_style_xf(size_t index) {}
void _import_styles::set_cell_style_builtin(size_t index) {}
void _import_styles::set_cell_style_parent_name(const char* s, size_t n) {}
size_t _import_styles::commit_cell_style() { return 0; }

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
