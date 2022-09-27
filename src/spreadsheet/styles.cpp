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

namespace orcus { namespace spreadsheet {

font_t::font_t() :
    size(0.0), bold(false),
    italic(false), underline_style(underline_t::none),
    underline_width(underline_width_t::none),
    underline_mode(underline_mode_t::continuous),
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

void font_active_t::set() noexcept
{
    name = true;
    size = true;
    bold = true;
    italic = true;
    underline_style = true;
    underline_width = true;
    underline_mode = true;
    underline_type = true;
    underline_color = true;
    color = true;
    strikethrough_style = true;
    strikethrough_width = true;
    strikethrough_type = true;
    strikethrough_text = true;
}

void font_active_t::reset()
{
    *this = font_active_t();
}

bool font_active_t::operator== (const font_active_t& other) const noexcept
{
    return name == other.name &&
        size == other.size &&
        bold == other.bold &&
        italic == other.italic &&
        underline_style == other.underline_style &&
        underline_width == other.underline_width &&
        underline_mode == other.underline_mode &&
        underline_type == other.underline_type &&
        underline_color == other.underline_color &&
        color == other.color &&
        strikethrough_style == other.strikethrough_style &&
        strikethrough_width == other.strikethrough_width &&
        strikethrough_type == other.strikethrough_type &&
        strikethrough_text == other.strikethrough_text;
}

bool font_active_t::operator!= (const font_active_t& other) const noexcept
{
    return !operator== (other);
}

fill_t::fill_t() :
    pattern_type(fill_pattern_t::none)
{
}

void fill_t::reset()
{
    *this = fill_t();
}

void fill_active_t::set() noexcept
{
    pattern_type = true;
    fg_color = true;
    bg_color = true;
}

void fill_active_t::reset()
{
    *this = fill_active_t();
}

bool fill_active_t::operator== (const fill_active_t& other) const noexcept
{
    return pattern_type == other.pattern_type && fg_color == other.fg_color && bg_color == other.bg_color;
}

bool fill_active_t::operator!= (const fill_active_t& other) const noexcept
{
    return !operator==(other);
}

border_attrs_t::border_attrs_t():
    style(orcus::spreadsheet::border_style_t::unknown)
{
}

void border_attrs_t::reset()
{
    *this = border_attrs_t();
}

void border_attrs_active_t::set() noexcept
{
    style = true;
    border_color = true;
    border_width = true;
}

void border_attrs_active_t::reset()
{
    *this = border_attrs_active_t();
}

bool border_attrs_active_t::operator== (const border_attrs_active_t& other) const noexcept
{
    return style == other.style && border_color == other.border_color && border_width == other.border_width;
}

bool border_attrs_active_t::operator!= (const border_attrs_active_t& other) const noexcept
{
    return !operator==(other);
}

border_t::border_t()
{
}

void border_t::reset()
{
    *this = border_t();
}

void border_active_t::set() noexcept
{
    top.set();
    bottom.set();
    left.set();
    right.set();
    diagonal.set();
    diagonal_bl_tr.set();
    diagonal_tl_br.set();
}

void border_active_t::reset()
{
    top.reset();
    bottom.reset();
    left.reset();
    right.reset();
    diagonal.reset();
    diagonal_bl_tr.reset();
    diagonal_tl_br.reset();
}

bool border_active_t::operator== (const border_active_t& other) const noexcept
{
    return top == other.top && bottom == other.bottom &&
        left == other.left && right == other.right && diagonal == other.diagonal &&
        diagonal_bl_tr == other.diagonal_bl_tr && diagonal_tl_br == other.diagonal_tl_br;
}

bool border_active_t::operator!= (const border_active_t& other) const noexcept
{
    return !operator== (other);
}

protection_t::protection_t() :
    locked(false), hidden(false), print_content(false), formula_hidden(false)
{
}

void protection_t::reset()
{
    *this = protection_t();
}

void protection_active_t::set() noexcept
{
    locked = true;
    hidden = true;
    print_content = true;
    formula_hidden = true;
}

void protection_active_t::reset()
{
    *this = protection_active_t();
}

bool protection_active_t::operator== (const protection_active_t& other) const noexcept
{
    return locked == other.locked && hidden == other.hidden &&
        print_content == other.print_content &&
        formula_hidden == other.formula_hidden;
}

bool protection_active_t::operator!= (const protection_active_t& other) const noexcept
{
    return !operator== (other);
}

number_format_t::number_format_t() : identifier(0) {}

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

void number_format_active_t::set() noexcept
{
    identifier = true;
    format_string = true;
}

void number_format_active_t::reset()
{
    *this = number_format_active_t();
}

bool number_format_active_t::operator== (const number_format_active_t& other) const noexcept
{
    return identifier == other.identifier && format_string == other.format_string;
}

bool number_format_active_t::operator!= (const number_format_active_t& other) const noexcept
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
    std::vector<style_attrs_t<font_t>> fonts;
    std::vector<style_attrs_t<fill_t>> fills;
    std::vector<style_attrs_t<border_t>> borders;
    std::vector<style_attrs_t<protection_t>> protections;
    std::vector<style_attrs_t<number_format_t>> number_formats;
    std::vector<cell_format_t> cell_style_formats;
    std::vector<cell_format_t> cell_formats;
    std::vector<cell_format_t> dxf_formats;
    std::vector<cell_style_t> cell_styles;

    string_pool str_pool;
};

styles::styles() : mp_impl(std::make_unique<impl>()) {}
styles::~styles() {}

void styles::reserve_font_store(size_t n)
{
    mp_impl->fonts.reserve(n);
}

size_t styles::append_font(const font_t& font)
{
    // Preserve current behavior until next API version.
    font_active_t active;
    active.set();
    mp_impl->fonts.emplace_back(font, active);
    return mp_impl->fonts.size() - 1;
}

size_t styles::append_font(const font_t& value, const font_active_t& active)
{
    mp_impl->fonts.emplace_back(value, active);
    return mp_impl->fonts.size() - 1;
}

void styles::reserve_fill_store(size_t n)
{
    mp_impl->fills.reserve(n);
}

size_t styles::append_fill(const fill_t& fill)
{
    // Preserve current behavior until next API version.
    fill_active_t active;
    active.set();
    mp_impl->fills.emplace_back(fill, active);
    return mp_impl->fills.size() - 1;
}

size_t styles::append_fill(const fill_t& value, const fill_active_t& active)
{
    mp_impl->fills.emplace_back(value, active);
    return mp_impl->fills.size() - 1;
}

void styles::reserve_border_store(size_t n)
{
    mp_impl->borders.reserve(n);
}

size_t styles::append_border(const border_t& border)
{
    // Preserve current behavior until next API version.
    border_active_t active;
    active.set();
    mp_impl->borders.emplace_back(border, active);
    return mp_impl->borders.size() - 1;
}

size_t styles::append_border(const border_t& value, const border_active_t& active)
{
    mp_impl->borders.emplace_back(value, active);
    return mp_impl->borders.size() - 1;
}

size_t styles::append_protection(const protection_t& protection)
{
    // Preserve current behavior until next API version.
    protection_active_t active;
    active.set();
    mp_impl->protections.emplace_back(protection, active);
    return mp_impl->protections.size() - 1;
}

size_t styles::append_protection(const protection_t& value, const protection_active_t& active)
{
    mp_impl->protections.emplace_back(value, active);
    return mp_impl->protections.size() - 1;
}

void styles::reserve_number_format_store(size_t n)
{
    mp_impl->number_formats.reserve(n);
}

size_t styles::append_number_format(const number_format_t& nf)
{
    // Preserve current behavior until next API version.
    number_format_active_t active;
    active.set();
    number_format_t copied = nf;
    copied.format_string = mp_impl->str_pool.intern(nf.format_string).first;
    mp_impl->number_formats.emplace_back(copied, active);
    return mp_impl->number_formats.size() - 1;
}

size_t styles::append_number_format(const number_format_t& value, const number_format_active_t& active)
{
    number_format_t copied = value;
    copied.format_string = mp_impl->str_pool.intern(value.format_string).first;
    mp_impl->number_formats.emplace_back(copied, active);
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

size_t styles::append_cell_style(const cell_style_t& cs)
{
    mp_impl->cell_styles.push_back(cs);
    return mp_impl->cell_styles.size() - 1;
}

const font_t* styles::get_font(size_t index) const
{
    if (index >= mp_impl->fonts.size())
        return nullptr;

    return &mp_impl->fonts[index].first;
}

const style_attrs_t<font_t>* styles::get_font_state(size_t index) const
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

    return &mp_impl->fills[index].first;
}

const style_attrs_t<fill_t>* styles::get_fill_state(size_t index) const
{
    if (index >= mp_impl->fills.size())
        return nullptr;

    return &mp_impl->fills[index];
}

const border_t* styles::get_border(size_t index) const
{
    if (index >= mp_impl->borders.size())
        return nullptr;

    return &mp_impl->borders[index].first;
}

const style_attrs_t<border_t>* styles::get_border_state(size_t index) const
{
    if (index >= mp_impl->borders.size())
        return nullptr;

    return &mp_impl->borders[index];
}

const protection_t* styles::get_protection(size_t index) const
{
    if (index >= mp_impl->protections.size())
        return nullptr;

    return &mp_impl->protections[index].first;
}

const style_attrs_t<protection_t>* styles::get_protection_state(size_t index) const
{
    if (index >= mp_impl->protections.size())
        return nullptr;

    return &mp_impl->protections[index];
}

const number_format_t* styles::get_number_format(size_t index) const
{
    if (index >= mp_impl->number_formats.size())
        return nullptr;

    return &mp_impl->number_formats[index].first;
}

const style_attrs_t<number_format_t>* styles::get_number_format_state(size_t index) const
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

void styles::finalize()
{
    // Sort the cell styles records by their xf ID's, as they may not appear in
    // the same order as their corresponding cell style format records that the
    // xf ID's point to.

    auto less_func = [](const cell_style_t& left, const cell_style_t& right)
    {
        return left.xf < right.xf;
    };

    std::sort(mp_impl->cell_styles.begin(), mp_impl->cell_styles.end(), less_func);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
