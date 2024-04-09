/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/spreadsheet/document_types.hpp>

namespace orcus { namespace spreadsheet {

color_t::color_t() :
    alpha(0), red(0), green(0), blue(0)
{
}

color_t::color_t(color_elem_t _red, color_elem_t _green, color_elem_t _blue) :
    alpha(255), red(_red), green(_green), blue(_blue)
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

format_run_t::format_run_t() = default;
format_run_t::format_run_t(const format_run_t& other) = default;
format_run_t::~format_run_t() = default;

format_run_t& format_run_t::operator=(const format_run_t& other)
{
    pos = other.pos;
    size = other.size;
    font = other.font;
    font_size = other.font_size;
    color = other.color;
    bold = other.bold;
    italic = other.italic;

    return *this;
}

void format_run_t::reset()
{
    pos = 0;
    size = 0;
    font.reset();
    font_size.reset();
    color.reset();
    bold.reset();
    italic.reset();
}

bool format_run_t::formatted() const
{
    return font.has_value() || font_size.has_value() || color.has_value() || bold.has_value() || italic.has_value();
}

}} // namespace orcus::spreadsheet

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
