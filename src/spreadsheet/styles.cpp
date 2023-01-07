/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/spreadsheet/styles.hpp"
#include "orcus/string_pool.hpp"

#include "ostream_utils.hpp"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <vector>
#include <map>

namespace orcus { namespace spreadsheet {

font_t::font_t() = default;

void font_t::reset()
{
    *this = font_t();
}

fill_t::fill_t() = default;

void fill_t::reset()
{
    *this = fill_t();
}

border_attrs_t::border_attrs_t() = default;

void border_attrs_t::reset()
{
    *this = border_attrs_t();
}

border_t::border_t() = default;

void border_t::reset()
{
    *this = border_t();
}

protection_t::protection_t() = default;

void protection_t::reset()
{
    *this = protection_t();
}

number_format_t::number_format_t() = default;

void number_format_t::reset()
{
    *this = number_format_t();
}

bool number_format_t::operator== (const number_format_t& other) const noexcept
{
    return identifier == other.identifier && format_string == other.format_string;
}

bool number_format_t::operator!= (const number_format_t& other) const noexcept
{
    return !operator== (other);
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
    ::orcus::detail::ostream_format_guard ifs(os);

    os << std::uppercase;

    os << "(ARGB:"
       << ' ' << std::hex << std::setfill('0') << std::setw(2) << int(c.alpha & 0xFF)
       << ' ' << std::hex << std::setfill('0') << std::setw(2) << int(c.red & 0xFF)
       << ' ' << std::hex << std::setfill('0') << std::setw(2) << int(c.green & 0xFF)
       << ' ' << std::hex << std::setfill('0') << std::setw(2) << int(c.blue & 0xFF)
       << ")";

    return os;
}

struct styles::impl
{
    std::vector<font_t> fonts;
    std::vector<fill_t> fills;
    std::vector<border_t> borders;
    std::vector<protection_t> protections;
    std::vector<number_format_t> number_formats;
    std::vector<cell_format_t> cell_style_formats;
    std::vector<cell_format_t> cell_formats;
    std::vector<cell_format_t> dxf_formats;
    std::vector<cell_style_t> cell_styles;
    std::map<std::size_t, std::size_t> cell_styles_map; // style xf to style position in `cell_styles`

    string_pool str_pool;
};

styles::styles() : mp_impl(std::make_unique<impl>()) {}
styles::~styles() {}

void styles::reserve_font_store(size_t n)
{
    mp_impl->fonts.reserve(n);
}

std::size_t styles::append_font(const font_t& font)
{
    mp_impl->fonts.emplace_back(font);
    return mp_impl->fonts.size() - 1;
}

void styles::reserve_fill_store(size_t n)
{
    mp_impl->fills.reserve(n);
}

std::size_t styles::append_fill(const fill_t& fill)
{
    mp_impl->fills.emplace_back(fill);
    return mp_impl->fills.size() - 1;
}

void styles::reserve_border_store(size_t n)
{
    mp_impl->borders.reserve(n);
}

std::size_t styles::append_border(const border_t& border)
{
    mp_impl->borders.emplace_back(border);
    return mp_impl->borders.size() - 1;
}

std::size_t styles::append_protection(const protection_t& protection)
{
    mp_impl->protections.emplace_back(protection);
    return mp_impl->protections.size() - 1;
}

void styles::reserve_number_format_store(size_t n)
{
    mp_impl->number_formats.reserve(n);
}

std::size_t styles::append_number_format(const number_format_t& nf)
{
    if (nf.format_string)
    {
        number_format_t copied = nf;
        copied.format_string = mp_impl->str_pool.intern(*nf.format_string).first;
        mp_impl->number_formats.emplace_back(copied);
    }
    else
        mp_impl->number_formats.emplace_back(nf);

    return mp_impl->number_formats.size() - 1;
}

void styles::reserve_cell_style_format_store(size_t n)
{
    mp_impl->cell_style_formats.reserve(n);
}

size_t styles::append_cell_style_format(const cell_format_t& cf)
{
    mp_impl->cell_style_formats.push_back(cf);
    return mp_impl->cell_style_formats.size() - 1;
}

void styles::reserve_cell_format_store(size_t n)
{
    mp_impl->cell_formats.reserve(n);
}

size_t styles::append_cell_format(const cell_format_t& cf)
{
    mp_impl->cell_formats.push_back(cf);
    return mp_impl->cell_formats.size() - 1;
}

void styles::reserve_diff_cell_format_store(size_t n)
{
    mp_impl->dxf_formats.reserve(n);
}

size_t styles::append_diff_cell_format(const cell_format_t& cf)
{
    mp_impl->dxf_formats.push_back(cf);
    return mp_impl->dxf_formats.size() - 1;
}

void styles::reserve_cell_style_store(size_t n)
{
    mp_impl->cell_styles.reserve(n);
}

void styles::append_cell_style(const cell_style_t& cs)
{
    mp_impl->cell_styles.push_back(cs);
}

const font_t* styles::get_font(size_t index) const
{
    if (index >= mp_impl->fonts.size())
        return nullptr;

    return &mp_impl->fonts[index];
}

const cell_format_t* styles::get_cell_format(size_t index) const
{
    if (index >= mp_impl->cell_formats.size())
        return nullptr;

    return &mp_impl->cell_formats[index];
}

const fill_t* styles::get_fill(size_t index) const
{
    if (index >= mp_impl->fills.size())
        return nullptr;

    return &mp_impl->fills[index];
}

const border_t* styles::get_border(size_t index) const
{
    if (index >= mp_impl->borders.size())
        return nullptr;

    return &mp_impl->borders[index];
}

const protection_t* styles::get_protection(size_t index) const
{
    if (index >= mp_impl->protections.size())
        return nullptr;

    return &mp_impl->protections[index];
}

const number_format_t* styles::get_number_format(size_t index) const
{
    if (index >= mp_impl->number_formats.size())
        return nullptr;

    return &mp_impl->number_formats[index];
}

const cell_format_t* styles::get_cell_style_format(size_t index) const
{
    if (index >= mp_impl->cell_style_formats.size())
        return nullptr;

    return &mp_impl->cell_style_formats[index];
}

const cell_format_t* styles::get_dxf_format(size_t index) const
{
    if (index >= mp_impl->dxf_formats.size())
        return nullptr;

    return &mp_impl->dxf_formats[index];
}

const cell_style_t* styles::get_cell_style(size_t index) const
{
    if (index >= mp_impl->cell_styles.size())
        return nullptr;

    return &mp_impl->cell_styles[index];
}

const cell_style_t* styles::get_cell_style_by_xf(size_t xfid) const
{
    auto it = mp_impl->cell_styles_map.find(xfid);
    if (it == mp_impl->cell_styles_map.end())
        return nullptr;

    auto index = it->second;
    return &mp_impl->cell_styles[index];
}

size_t styles::get_font_count() const
{
    return mp_impl->fonts.size();
}

size_t styles::get_fill_count() const
{
    return mp_impl->fills.size();
}

size_t styles::get_border_count() const
{
    return mp_impl->borders.size();
}

size_t styles::get_protection_count() const
{
    return mp_impl->protections.size();
}

size_t styles::get_number_format_count() const
{
    return mp_impl->number_formats.size();
}

size_t styles::get_cell_formats_count() const
{
    return mp_impl->cell_formats.size();
}

size_t styles::get_cell_style_formats_count() const
{
    return mp_impl->cell_style_formats.size();
}

size_t styles::get_dxf_count() const
{
    return mp_impl->dxf_formats.size();
}

size_t styles::get_cell_styles_count() const
{
    return mp_impl->cell_styles.size();
}

void styles::clear()
{
    mp_impl = std::make_unique<impl>();
}

void styles::finalize_import()
{
    for (std::size_t i = 0; i < mp_impl->cell_styles.size(); ++i)
    {
        const auto& entry = mp_impl->cell_styles[i];
        mp_impl->cell_styles_map.insert_or_assign(entry.xf, i);
    }
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
