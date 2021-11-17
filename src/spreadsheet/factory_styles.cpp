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

namespace {

struct border_attr_access_t
{
    border_attrs_t* values = nullptr;
    border_attrs_active_t* active = nullptr;

    operator bool() const noexcept
    {
        return values != nullptr && active != nullptr;
    }
};

} // anonymous namespace

struct import_styles::impl
{
    styles& styles_model;
    string_pool& str_pool;

    font_t cur_font;
    font_active_t cur_font_active;

    fill_t cur_fill;
    fill_active_t cur_fill_active;

    border_t cur_border;
    border_active_t cur_border_active;

    protection_t cur_protection;
    number_format_t cur_number_format;
    cell_format_t cur_cell_format;
    cell_style_t cur_cell_style;

    impl(styles& _styles_model, string_pool& sp) :
        styles_model(_styles_model),
        str_pool(sp) {}

    border_attr_access_t get_border_attrs(border_direction_t dir)
    {
        border_attr_access_t ret;

        switch (dir)
        {
            case border_direction_t::top:
                ret.values = &cur_border.top;
                ret.active = &cur_border_active.top;
                break;
            case border_direction_t::bottom:
                ret.values = &cur_border.bottom;
                ret.active = &cur_border_active.bottom;
                break;
            case border_direction_t::left:
                ret.values = &cur_border.left;
                ret.active = &cur_border_active.left;
                break;
            case border_direction_t::right:
                ret.values = &cur_border.right;
                ret.active = &cur_border_active.right;
                break;
            case border_direction_t::diagonal:
                ret.values = &cur_border.diagonal;
                ret.active = &cur_border_active.diagonal;
                break;
            case border_direction_t::diagonal_bl_tr:
                ret.values = &cur_border.diagonal_bl_tr;
                ret.active = &cur_border_active.diagonal_bl_tr;
                break;
            case border_direction_t::diagonal_tl_br:
                ret.values = &cur_border.diagonal_tl_br;
                ret.active = &cur_border_active.diagonal_tl_br;
                break;
            default:
                ;
        }

        return ret;
    }
};

import_styles::import_styles(styles& styles_model, string_pool& sp) :
    mp_impl(std::make_unique<impl>(styles_model, sp)) {}

import_styles::~import_styles() {}

void import_styles::set_font_count(size_t n)
{
    mp_impl->styles_model.reserve_font_store(n);
}

void import_styles::set_font_bold(bool b)
{
    mp_impl->cur_font.bold = b;
    mp_impl->cur_font_active.bold = true;
}

void import_styles::set_font_italic(bool b)
{
    mp_impl->cur_font.italic = b;
    mp_impl->cur_font_active.italic = true;
}

void import_styles::set_font_name(std::string_view s)
{
    mp_impl->cur_font.name = mp_impl->str_pool.intern(s).first;
    mp_impl->cur_font_active.name = true;
}

void import_styles::set_font_size(double point)
{
    mp_impl->cur_font.size = point;
    mp_impl->cur_font_active.size = true;
}

void import_styles::set_font_underline(underline_t e)
{
    mp_impl->cur_font.underline_style = e;
    mp_impl->cur_font_active.underline_style = true;
}

void import_styles::set_font_underline_width(underline_width_t e)
{
    mp_impl->cur_font.underline_width = e;
    mp_impl->cur_font_active.underline_width = true;
}

void import_styles::set_font_underline_mode(underline_mode_t e)
{
    mp_impl->cur_font.underline_mode = e;
    mp_impl->cur_font_active.underline_mode = true;
}

void import_styles::set_font_underline_type(underline_type_t e)
{
    mp_impl->cur_font.underline_type = e;
    mp_impl->cur_font_active.underline_type = true;
}

void import_styles::set_font_underline_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    mp_impl->cur_font.underline_color = color_t(alpha, red, green, blue);
    mp_impl->cur_font_active.underline_color = true;
}

void import_styles::set_font_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    mp_impl->cur_font.color = color_t(alpha, red, green, blue);
    mp_impl->cur_font_active.color = true;
}

void import_styles::set_strikethrough_style(strikethrough_style_t s)
{
    mp_impl->cur_font.strikethrough_style = s;
    mp_impl->cur_font_active.strikethrough_style = true;
}

void import_styles::set_strikethrough_width(strikethrough_width_t s)
{
    mp_impl->cur_font.strikethrough_width = s;
    mp_impl->cur_font_active.strikethrough_width = true;
}

void import_styles::set_strikethrough_type(strikethrough_type_t s)
{
    mp_impl->cur_font.strikethrough_type = s;
    mp_impl->cur_font_active.strikethrough_type = true;
}

void import_styles::set_strikethrough_text(strikethrough_text_t s)
{
    mp_impl->cur_font.strikethrough_text = s;
    mp_impl->cur_font_active.strikethrough_text = true;
}

size_t import_styles::commit_font()
{
    size_t font_id = mp_impl->styles_model.append_font(mp_impl->cur_font, mp_impl->cur_font_active);
    mp_impl->cur_font.reset();
    mp_impl->cur_font_active.reset();
    return font_id;
}

void import_styles::set_fill_count(size_t n)
{
    mp_impl->styles_model.reserve_fill_store(n);
}

void import_styles::set_fill_pattern_type(fill_pattern_t fp)
{
    mp_impl->cur_fill.pattern_type = fp;
    mp_impl->cur_fill_active.pattern_type = true;
}

void import_styles::set_fill_fg_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    mp_impl->cur_fill.fg_color = color_t(alpha, red, green, blue);
    mp_impl->cur_fill_active.fg_color = true;
}

void import_styles::set_fill_bg_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    mp_impl->cur_fill.bg_color = color_t(alpha, red, green, blue);
    mp_impl->cur_fill_active.bg_color = true;
}

size_t import_styles::commit_fill()
{
    size_t fill_id = mp_impl->styles_model.append_fill(mp_impl->cur_fill, mp_impl->cur_fill_active);
    mp_impl->cur_fill.reset();
    mp_impl->cur_fill_active.reset();
    return fill_id;
}

void import_styles::set_border_count(size_t n)
{
    mp_impl->styles_model.reserve_border_store(n);
}

void import_styles::set_border_style(border_direction_t dir, border_style_t style)
{
    auto v = mp_impl->get_border_attrs(dir);
    if (!v)
        return;

    v.values->style = style;
    v.active->style = true;
}

void import_styles::set_border_color(
    border_direction_t dir, color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    auto v = mp_impl->get_border_attrs(dir);
    if (!v)
        return;

    v.values->border_color = color_t(alpha, red, green, blue);
    v.active->border_color = true;
}

void import_styles::set_border_width(border_direction_t dir, double width, orcus::length_unit_t unit)
{
    auto v = mp_impl->get_border_attrs(dir);
    if (!v)
        return;

    v.values->border_width.value = width;
    v.values->border_width.unit = unit;
    v.active->border_width = true;
}

size_t import_styles::commit_border()
{
    size_t border_id = mp_impl->styles_model.append_border(mp_impl->cur_border, mp_impl->cur_border_active);
    mp_impl->cur_border.reset();
    mp_impl->cur_border_active.reset();
    return border_id;
}

void import_styles::set_cell_hidden(bool b)
{
    mp_impl->cur_protection.hidden = b;
}

void import_styles::set_cell_locked(bool b)
{
    mp_impl->cur_protection.locked = b;
}

void import_styles::set_cell_print_content(bool b)
{
    mp_impl->cur_protection.print_content = b;
}

void import_styles::set_cell_formula_hidden(bool b)
{
    mp_impl->cur_protection.formula_hidden = b;
}

size_t import_styles::commit_cell_protection()
{
    size_t cp_id = mp_impl->styles_model.append_protection(mp_impl->cur_protection);
    mp_impl->cur_protection.reset();
    return cp_id;
}

void import_styles::set_number_format_count(size_t n)
{
    mp_impl->styles_model.reserve_number_format_store(n);
}

void import_styles::set_number_format_identifier(size_t id)
{
    mp_impl->cur_number_format.identifier = id;
}

void import_styles::set_number_format_code(std::string_view s)
{
    mp_impl->cur_number_format.format_string = s;
}

size_t import_styles::commit_number_format()
{
    size_t nf_id = mp_impl->styles_model.append_number_format(mp_impl->cur_number_format);
    mp_impl->cur_number_format.reset();
    return nf_id;
}

void import_styles::set_cell_xf_count(size_t n)
{
    mp_impl->styles_model.reserve_cell_format_store(n);
}

void import_styles::set_cell_style_xf_count(size_t n)
{
    mp_impl->styles_model.reserve_cell_style_format_store(n);
}

void import_styles::set_dxf_count(size_t n)
{
    mp_impl->styles_model.reserve_diff_cell_format_store(n);
}

void import_styles::set_xf_font(size_t index)
{
    mp_impl->cur_cell_format.font = index;
}

void import_styles::set_xf_fill(size_t index)
{
    mp_impl->cur_cell_format.fill = index;
}

void import_styles::set_xf_border(size_t index)
{
    mp_impl->cur_cell_format.border = index;

    // TODO : we need to decide whether to have interface methods for these
    // apply_foo attributes.  For now there is only one, for alignment.
    mp_impl->cur_cell_format.apply_border = index > 0;
}

void import_styles::set_xf_protection(size_t index)
{
    mp_impl->cur_cell_format.protection = index;
}

void import_styles::set_xf_number_format(size_t index)
{
    mp_impl->cur_cell_format.number_format = index;
}

void import_styles::set_xf_style_xf(size_t index)
{
    mp_impl->cur_cell_format.style_xf = index;
}

void import_styles::set_xf_apply_alignment(bool b)
{
    mp_impl->cur_cell_format.apply_alignment = b;
}

void import_styles::set_xf_horizontal_alignment(orcus::spreadsheet::hor_alignment_t align)
{
    mp_impl->cur_cell_format.hor_align = align;
}

void import_styles::set_xf_vertical_alignment(orcus::spreadsheet::ver_alignment_t align)
{
    mp_impl->cur_cell_format.ver_align = align;
}

size_t import_styles::commit_cell_xf()
{
    size_t n = mp_impl->styles_model.append_cell_format(mp_impl->cur_cell_format);
    mp_impl->cur_cell_format.reset();
    return n;
}

size_t import_styles::commit_cell_style_xf()
{
    size_t n = mp_impl->styles_model.append_cell_style_format(mp_impl->cur_cell_format);
    mp_impl->cur_cell_format.reset();
    return n;
}

size_t import_styles::commit_dxf()
{
    size_t n = mp_impl->styles_model.append_diff_cell_format(mp_impl->cur_cell_format);
    mp_impl->cur_cell_format.reset();
    return n;
}

void import_styles::set_cell_style_count(size_t n)
{
    mp_impl->styles_model.reserve_cell_style_store(n);
}

void import_styles::set_cell_style_name(std::string_view s)
{
    mp_impl->cur_cell_style.name = mp_impl->str_pool.intern(s).first;
}

void import_styles::set_cell_style_xf(size_t index)
{
    mp_impl->cur_cell_style.xf = index;
}

void import_styles::set_cell_style_builtin(size_t index)
{
    mp_impl->cur_cell_style.builtin = index;
}

void import_styles::set_cell_style_parent_name(std::string_view s)
{
    mp_impl->cur_cell_style.parent_name = mp_impl->str_pool.intern(s).first;
}

size_t import_styles::commit_cell_style()
{
    size_t n = mp_impl->styles_model.append_cell_style(mp_impl->cur_cell_style);
    mp_impl->cur_cell_style.reset();
    return n;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
