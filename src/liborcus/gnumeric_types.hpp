/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <orcus/spreadsheet/types.hpp>

#include <string_view>
#include <optional>

namespace orcus {

/**
 * Values are as specified in the Gnumeric source code except for vt_unknown.
 */
enum gnumeric_value_type
{
    vt_empty = 10,
    vt_boolean = 20,
    vt_float = 40,
    vt_error = 50,
    vt_string = 60,
    vt_cellrange = 70,
    vt_array = 80
};

struct gnumeric_named_exp
{
    std::string_view name;
    std::string_view value;
    spreadsheet::src_address_t position = {0, 0, 0};

    void reset();
};

struct gnumeric_style
{
    spreadsheet::sheet_t sheet = -1;
    spreadsheet::range_t region = {{-1, -1}, {-1, -1}};
    spreadsheet::hor_alignment_t hor_align = spreadsheet::hor_alignment_t::unknown;
    spreadsheet::ver_alignment_t ver_align = spreadsheet::ver_alignment_t::unknown;
    std::optional<bool> wrap_text;

    std::optional<spreadsheet::color_rgb_t> fore;
    std::optional<spreadsheet::color_rgb_t> back;
    std::optional<spreadsheet::color_rgb_t> pattern_color;
    spreadsheet::fill_pattern_t pattern = spreadsheet::fill_pattern_t::none;

    bool valid() const;
};

std::optional<spreadsheet::color_rgb_t> parse_gnumeric_rgb(std::string_view v);

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
