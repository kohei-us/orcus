/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "odf_helper.hpp"
#include "string_helper.hpp"
#include <orcus/spreadsheet/types.hpp>
#include <orcus/measurement.hpp>
#include <mdds/sorted_string_map.hpp>
#include <mdds/global.hpp>
#include <orcus/spreadsheet/styles.hpp>

namespace ss = orcus::spreadsheet;

namespace orcus {

namespace {

namespace border_style {

using map_type = mdds::sorted_string_map<spreadsheet::border_style_t>;

// Keys must be sorted.
constexpr map_type::entry_type entries[] =
{
    { "dash-dot", spreadsheet::border_style_t::dash_dot },
    { "dash-dot-dot", spreadsheet::border_style_t::dash_dot_dot },
    { "dashed", spreadsheet::border_style_t::dashed },
    { "dotted", spreadsheet::border_style_t::dotted },
    { "double-thin", spreadsheet::border_style_t::double_thin },
    { "fine-dashed", spreadsheet::border_style_t::fine_dashed },
    { "none", spreadsheet::border_style_t::none },
    { "solid", spreadsheet::border_style_t::solid },
};

const map_type& get()
{
    static const map_type mt(entries, std::size(entries), ss::border_style_t::unknown);
    return mt;
}

} // namespace border_style

namespace underline_width {

using map_type = mdds::sorted_string_map<spreadsheet::underline_width_t>;

// Keys must be sorted.
constexpr map_type::entry_type entries[] =
{
    { "auto", ss::underline_width_t::automatic },
    { "bold", ss::underline_width_t::bold },
    { "dash", ss::underline_width_t::dash },
    { "medium", ss::underline_width_t::medium },
    { "thick", ss::underline_width_t::thick },
    { "thin", ss::underline_width_t::thin },
};

const map_type& get()
{
    static const map_type mt(entries, std::size(entries), ss::underline_width_t::none);
    return mt;
}

} // namespace underline_width

namespace underline_style {

using map_type = mdds::sorted_string_map<ss::underline_t>;

// Keys must be sorted.
constexpr map_type::entry_type entries[] =
{
    { "dash", ss::underline_t::dash },
    { "dot-dash", ss::underline_t::dot_dash },
    { "dot-dot-dash", ss::underline_t::dot_dot_dash },
    { "dotted", ss::underline_t::dotted },
    { "long-dash", ss::underline_t::long_dash },
    { "none", ss::underline_t::none },
    { "solid", ss::underline_t::single_line },
    { "wave", ss::underline_t::wave }
};

const map_type& get()
{
    static map_type mt(entries, std::size(entries), ss::underline_t::none);
    return mt;
}

} // namespace underline_style

namespace hor_align {

using map_type = mdds::sorted_string_map<spreadsheet::hor_alignment_t>;

// Keys must be sorted.
constexpr map_type::entry_type entries[] =
{
    { "center", spreadsheet::hor_alignment_t::center },
    { "end", spreadsheet::hor_alignment_t::right },
    { "justified", spreadsheet::hor_alignment_t::justified },
    { "start", spreadsheet::hor_alignment_t::left }
};

const map_type& get()
{
    static map_type mt(entries, std::size(entries), ss::hor_alignment_t::unknown);
    return mt;
}

} // namespace hor_align

namespace ver_align {

using map_type = mdds::sorted_string_map<spreadsheet::ver_alignment_t>;

// Keys must be sorted.
constexpr map_type::entry_type entries[] =
{
    { "bottom", spreadsheet::ver_alignment_t::bottom },
    { "justified", spreadsheet::ver_alignment_t::justified },
    { "middle", spreadsheet::ver_alignment_t::middle },
    { "top", spreadsheet::ver_alignment_t::top }
};

const map_type& get()
{
    static map_type mt(entries, std::size(entries), ss::ver_alignment_t::unknown);
    return mt;
}

} // namespace ver_align

bool is_valid_hex_digit(const char& character, orcus::spreadsheet::color_elem_t& val)
{
    if ('0' <= character && character <= '9')
    {
        val += character - '0';
        return true;
    }

    if ('A' <= character && character <= 'F')
    {
        val += character - 'A' + 10;
        return true;
    }

    if ('a' <= character && character <= 'f')
    {
        val += character - 'a' + 10;
        return true;
    }

    return false;
}

// converts two characters starting at index to a color value
bool convert_color_digits(std::string_view value, orcus::spreadsheet::color_elem_t& color_val, size_t index)
{
    const char& high_val = value[index];
    color_val = 0;
    if (!is_valid_hex_digit(high_val, color_val))
        return false;
    color_val *= 16;
    const char& low_val = value[++index];
    return is_valid_hex_digit(low_val, color_val);
}

} // anonymous namespace

bool odf::convert_fo_color(
    std::string_view value,
    spreadsheet::color_elem_t& red,
    spreadsheet::color_elem_t& green,
    spreadsheet::color_elem_t& blue)
{
    auto color = convert_fo_color(value);
    if (!color)
        return false;

    red = color->red;
    green = color->green;
    blue = color->blue;
    return true;
}

std::optional<spreadsheet::color_rgb_t> odf::convert_fo_color(std::string_view value)
{
    std::optional<spreadsheet::color_rgb_t> ret;

    // first character needs to be '#'
    if (value.size() != 7)
        return ret;

    if (value[0] != '#')
        return ret;

    spreadsheet::color_rgb_t color;
    if (!convert_color_digits(value, color.red, 1))
        return ret;

    if (!convert_color_digits(value, color.green, 3))
        return ret;

    if (!convert_color_digits(value, color.blue, 5))
        return ret;

    return color;
}

orcus::odf::border_details_t odf::extract_border_details(std::string_view value)
{
    border_details_t border_details;

    auto detail = orcus::string_helper::split_string(value,' ');

    for (const auto& sub_detail : detail)
    {
        if (sub_detail[0] == '#')
            convert_fo_color(sub_detail, border_details.red, border_details.green, border_details.blue);
        else if (sub_detail[0] >= '0' && sub_detail[0] <='9')
            border_details.border_width = orcus::to_length(sub_detail);
        else    //  This has to be a style
            border_details.border_style = border_style::get().find(sub_detail);
    }
    return border_details;
}

ss::underline_width_t odf::extract_underline_width(std::string_view value)
{
    // TODO: style:text-underline-width also allows:
    // * percent value
    // * positive integer
    // * positive length
    // As we encounter real-life examples of these values, we should add code to
    // handle them here.  For now, we only handle enumerated values.
    return underline_width::get().find(value);
}

orcus::spreadsheet::underline_t odf::extract_underline_style(std::string_view value)
{
    return underline_style::get().find(value);
}

ss::hor_alignment_t odf::extract_hor_alignment_style(std::string_view value)
{
    return hor_align::get().find(value);
}

spreadsheet::ver_alignment_t odf::extract_ver_alignment_style(std::string_view value)
{
    return ver_align::get().find(value);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
