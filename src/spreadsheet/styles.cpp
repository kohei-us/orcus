/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/spreadsheet/styles.hpp"
#include "orcus/global.hpp"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <vector>

namespace orcus { namespace spreadsheet {

font_t::font_t() :
    size(0.0), bold(false),
    italic(false), underline_style(underline_t::none),
    underline_width(underline_width_t::none),
    underline_mode(underline_mode_t::continuos),
    underline_type(underline_type_t::none),
    color(),
    strikethrough_style(strikethrough_style_t::none),
    strikethrough_width(strikethrough_width_t::unknown),
    strikethrough_type(strikethrough_type_t::unknown),
    strikethrough_text(strikethrough_text_t::unknown)
{
}

void font_t::reset()
{
    *this = font_t();
}

color_t::color_t() :
    alpha(0), red(0), green(0), blue(0)
{
}

color_t::color_t(color_elem_t _alpha, color_elem_t _red, color_elem_t _green, color_elem_t _blue) :
    alpha(_alpha), red(_red), green(_green), blue(_blue)
{
}

void color_t::reset()
{
    *this = color_t();
}

bool color_t::operator==(const color_t& other) const
{
    return alpha == other.alpha && red == other.red && green == other.green && blue == other.blue;
}

bool color_t::operator!=(const color_t& other) const
{
    return !operator==(other);
}

fill_t::fill_t() :
    pattern_type(fill_pattern_t::none)
{
}

void fill_t::reset()
{
    *this = fill_t();
}

border_attrs_t::border_attrs_t():
    style(orcus::spreadsheet::border_style_t::unknown)
{
}

void border_attrs_t::reset()
{
    *this = border_attrs_t();
}

border_t::border_t()
{
}

void border_t::reset()
{
    *this = border_t();
}

protection_t::protection_t() :
    locked(false), hidden(false), print_content(false), formula_hidden(false)
{
}

void protection_t::reset()
{
    *this = protection_t();
}

number_format_t::number_format_t() : identifier(0) {}

void number_format_t::reset()
{
    *this = number_format_t();
}

bool number_format_t::operator== (const number_format_t& r) const
{
    return format_string == r.format_string;
}

cell_format_t::cell_format_t() :
    font(0),
    fill(0),
    border(0),
    protection(0),
    number_format(0),
    style_xf(0),
    hor_align(hor_alignment_t::unknown),
    ver_align(ver_alignment_t::unknown),
    apply_num_format(false),
    apply_font(false),
    apply_fill(false),
    apply_border(false),
    apply_alignment(false),
    apply_protection(false)
{
}

void cell_format_t::reset()
{
    *this = cell_format_t();
}

cell_style_t::cell_style_t() :
    xf(0), builtin(0)
{
}

void cell_style_t::reset()
{
    *this = cell_style_t();
}

std::ostream& operator<< (std::ostream& os, const color_t& c)
{
    os << std::hex << std::uppercase
        << "(ARGB: "
        << std::setfill('0') << std::setw(2) << int(c.alpha)
        << std::setfill('0') << std::setw(2) << int(c.red)
        << std::setfill('0') << std::setw(2) << int(c.green)
        << std::setfill('0') << std::setw(2) << int(c.blue)
        << ")";

    return os;
}

struct styles::impl
{
    std::vector<font_t> m_fonts;
    std::vector<fill_t> m_fills;
    std::vector<border_t> m_borders;
    std::vector<protection_t> m_protections;
    std::vector<number_format_t> m_number_formats;
    std::vector<cell_format_t> m_cell_style_formats;
    std::vector<cell_format_t> m_cell_formats;
    std::vector<cell_format_t> m_dxf_formats;
    std::vector<cell_style_t> m_cell_styles;
};

styles::styles() : mp_impl(orcus::make_unique<impl>()) {}
styles::~styles() {}

void styles::reserve_font_store(size_t n)
{
    mp_impl->m_fonts.reserve(n);
}

size_t styles::append_font(const font_t& font)
{
    mp_impl->m_fonts.push_back(font);
    return mp_impl->m_fonts.size() - 1;
}

void styles::reserve_fill_store(size_t n)
{
    mp_impl->m_fills.reserve(n);
}

size_t styles::append_fill(const fill_t& fill)
{
    mp_impl->m_fills.push_back(fill);
    return mp_impl->m_fills.size() - 1;
}

void styles::reserve_border_store(size_t n)
{
    mp_impl->m_borders.reserve(n);
}

size_t styles::append_border(const border_t& border)
{
    mp_impl->m_borders.push_back(border);
    return mp_impl->m_borders.size() - 1;
}

size_t styles::append_protection(const protection_t& protection)
{
    mp_impl->m_protections.push_back(protection);
    return mp_impl->m_protections.size() - 1;
}

void styles::reserve_number_format_store(size_t n)
{
    mp_impl->m_number_formats.reserve(n);
}

size_t styles::append_number_format(const number_format_t& nf)
{
    mp_impl->m_number_formats.push_back(nf);
    return mp_impl->m_number_formats.size() - 1;
}

void styles::reserve_cell_style_format_store(size_t n)
{
    mp_impl->m_cell_style_formats.reserve(n);
}

size_t styles::append_cell_style_format(const cell_format_t& cf)
{
    mp_impl->m_cell_style_formats.push_back(cf);
    return mp_impl->m_cell_style_formats.size() - 1;
}

void styles::reserve_cell_format_store(size_t n)
{
    mp_impl->m_cell_formats.reserve(n);
}

size_t styles::append_cell_format(const cell_format_t& cf)
{
    mp_impl->m_cell_formats.push_back(cf);
    return mp_impl->m_cell_formats.size() - 1;
}

void styles::reserve_diff_cell_format_store(size_t n)
{
    mp_impl->m_dxf_formats.reserve(n);
}

size_t styles::append_diff_cell_format(const cell_format_t& cf)
{
    mp_impl->m_dxf_formats.push_back(cf);
    return mp_impl->m_dxf_formats.size() - 1;
}

void styles::reserve_cell_style_store(size_t n)
{
    mp_impl->m_cell_styles.reserve(n);
}

size_t styles::append_cell_style(const cell_style_t& cs)
{
    mp_impl->m_cell_styles.push_back(cs);
    return mp_impl->m_cell_styles.size() - 1;
}

const font_t* styles::get_font(size_t index) const
{
    if (index >= mp_impl->m_fonts.size())
        return nullptr;

    return &mp_impl->m_fonts[index];
}

const cell_format_t* styles::get_cell_format(size_t index) const
{
    if (index >= mp_impl->m_cell_formats.size())
        return nullptr;

    return &mp_impl->m_cell_formats[index];
}

const fill_t* styles::get_fill(size_t index) const
{
    if (index >= mp_impl->m_fills.size())
        return nullptr;

    return &mp_impl->m_fills[index];
}

const border_t* styles::get_border(size_t index) const
{
    if (index >= mp_impl->m_borders.size())
        return nullptr;

    return &mp_impl->m_borders[index];
}

const protection_t* styles::get_protection(size_t index) const
{
    if (index >= mp_impl->m_protections.size())
        return nullptr;

    return &mp_impl->m_protections[index];
}

const number_format_t* styles::get_number_format(size_t index) const
{
    if (index >= mp_impl->m_number_formats.size())
        return nullptr;

    return &mp_impl->m_number_formats[index];
}

const cell_format_t* styles::get_cell_style_format(size_t index) const
{
    if (index >= mp_impl->m_cell_style_formats.size())
        return nullptr;

    return &mp_impl->m_cell_style_formats[index];
}

const cell_format_t* styles::get_dxf_format(size_t index) const
{
    if (index >= mp_impl->m_dxf_formats.size())
        return nullptr;

    return &mp_impl->m_dxf_formats[index];
}

const cell_style_t* styles::get_cell_style(size_t index) const
{
    if (index >= mp_impl->m_cell_styles.size())
        return nullptr;

    return &mp_impl->m_cell_styles[index];
}

size_t styles::get_font_count() const
{
    return mp_impl->m_fonts.size();
}

size_t styles::get_fill_count() const
{
    return mp_impl->m_fills.size();
}

size_t styles::get_border_count() const
{
    return mp_impl->m_borders.size();
}

size_t styles::get_protection_count() const
{
    return mp_impl->m_protections.size();
}

size_t styles::get_number_format_count() const
{
    return mp_impl->m_number_formats.size();
}

size_t styles::get_cell_formats_count() const
{
    return mp_impl->m_cell_formats.size();
}

size_t styles::get_cell_style_formats_count() const
{
    return mp_impl->m_cell_style_formats.size();
}

size_t styles::get_dxf_count() const
{
    return mp_impl->m_dxf_formats.size();
}

size_t styles::get_cell_styles_count() const
{
    return mp_impl->m_cell_styles.size();
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
