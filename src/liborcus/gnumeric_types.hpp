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

#include <mdds/segment_tree.hpp>

namespace orcus {

namespace spreadsheet { namespace iface {

class import_underline;

}}

/**
 * Values are as specified in the Gnumeric source code.
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

/**
 * Values are as specified in the Gnumeric source code (see
 * GnmStyleBorderType).
 */
enum class gnumeric_border_type
{
    border_none = 0x0,
    border_thin = 0x1,
    border_medium = 0x2,
    border_dashed = 0x3,
    border_dotted = 0x4,
    border_thick = 0x5,
    border_double = 0x6,
    border_hair = 0x7,
    border_medium_dash = 0x8,
    border_dash_dot = 0x9,
    border_medium_dash_dot = 0xA,
    border_dash_dot_dot = 0xB,
    border_medium_dash_dot_dot = 0xC,
    border_slanted_dash_dot = 0xD,
};

spreadsheet::border_style_t to_standard_type(gnumeric_border_type v);

enum class gnumeric_value_format_type
{
    unknown,
    bold,
    color,
    family,
    italic,
    size,
    strikethrough,
    subscript,
    superscript,
    underline,
};

gnumeric_value_format_type to_gnumeric_value_format_type(std::string_view s);

struct gnumeric_value_format_segment
{
    gnumeric_value_format_type type = gnumeric_value_format_type::unknown;
    std::string_view value;

    bool operator==(const gnumeric_value_format_segment& other) const;
};

using value_format_segments_type = mdds::segment_tree<std::size_t, gnumeric_value_format_segment>;

enum gnumeric_script_type
{
    gnm_script_none = 0,
    gnm_script_super = 1,
    gnm_script_sub = -1
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
    struct border_type
    {
        std::optional<gnumeric_border_type> style;
        std::optional<spreadsheet::color_rgb_t> color;
    };

    spreadsheet::sheet_t sheet = -1;
    spreadsheet::range_t region = {{-1, -1}, {-1, -1}};
    spreadsheet::hor_alignment_t hor_align = spreadsheet::hor_alignment_t::unknown;
    spreadsheet::ver_alignment_t ver_align = spreadsheet::ver_alignment_t::unknown;

    std::optional<std::string_view> font_name;
    std::optional<double> font_unit;

    std::optional<bool> wrap_text;
    std::optional<bool> bold;
    std::optional<bool> italic;
    std::optional<long> underline;
    std::optional<bool> strikethrough;
    std::optional<gnumeric_script_type> script; // TODO : not supported yet

    std::optional<spreadsheet::color_rgb_t> fore;
    std::optional<spreadsheet::color_rgb_t> back;
    std::optional<spreadsheet::color_rgb_t> pattern_color;
    spreadsheet::fill_pattern_t pattern = spreadsheet::fill_pattern_t::none;

    std::optional<std::string_view> number_format;

    border_type border_top;
    border_type border_bottom;
    border_type border_left;
    border_type border_right;
    border_type border_bl_tr; // bottom-left to top-right  (diagonal)
    border_type border_br_tl; // top-left to bottom-right (rev-diagonal)

    bool valid() const;
};

/**
 * Parse an RGB color encoded as 'RRRR:GGGG:BBBB', each element being a 16-bit
 * hex value.
 */
std::optional<spreadsheet::color_rgb_t> parse_gnumeric_rgb(std::string_view v);

/**
 * Parse an RGB color encoded as 'RRxGGxBB', each element being an 8-bit hex
 * value.
 */
std::optional<spreadsheet::color_rgb_t> parse_gnumeric_rgb_8x(std::string_view v);

/**
 * Push a numeric underline value to the import interface.
 *
 * @param v  Numeric value representing the underline type.
 * @param ul Import interface for underline attributes.
 *
 * @return Warning message in case the import attempt has failed.
 *         If successful an empty string gets returned.
 */
std::string push_underline(long v, spreadsheet::iface::import_underline* ul);

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
