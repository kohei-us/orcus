/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gnumeric_types.hpp"
#include "number_utils.hpp"

#include <mdds/sorted_string_map.hpp>

namespace ss = orcus::spreadsheet;

namespace orcus {

namespace value_format_type {

using map_type = mdds::sorted_string_map<gnumeric_value_format_type, mdds::string_view_map_entry>;

// Keys must be sorted.
constexpr map_type::entry entries[] = {
    { "bold", gnumeric_value_format_type::bold },
    { "color", gnumeric_value_format_type::color },
    { "family", gnumeric_value_format_type::family },
    { "italic", gnumeric_value_format_type::italic },
    { "size", gnumeric_value_format_type::size },
    { "strikethrough", gnumeric_value_format_type::strikethrough },
    { "subscript", gnumeric_value_format_type::subscript },
    { "superscript", gnumeric_value_format_type::superscript },
    { "underline", gnumeric_value_format_type::underline },
};

const map_type& get()
{
    static const map_type mt(entries, std::size(entries), gnumeric_value_format_type::unknown);
    return mt;
}

} // namespace value_format_type

ss::border_style_t to_standard_type(gnumeric_border_type v)
{
    switch (v)
    {
        case gnumeric_border_type::border_none:
            return ss::border_style_t::none;
        case gnumeric_border_type::border_thin:
            return ss::border_style_t::thin;
        case gnumeric_border_type::border_medium:
            return ss::border_style_t::medium;
        case gnumeric_border_type::border_dashed:
            return ss::border_style_t::dashed;
        case gnumeric_border_type::border_dotted:
            return ss::border_style_t::dotted;
        case gnumeric_border_type::border_thick:
            return ss::border_style_t::thick;
        case gnumeric_border_type::border_double:
            return ss::border_style_t::double_border;
        case gnumeric_border_type::border_hair:
            return ss::border_style_t::hair;
        case gnumeric_border_type::border_medium_dash:
            return ss::border_style_t::medium_dashed;
        case gnumeric_border_type::border_dash_dot:
            return ss::border_style_t::dash_dot;
        case gnumeric_border_type::border_medium_dash_dot:
            return ss::border_style_t::medium_dash_dot;
        case gnumeric_border_type::border_dash_dot_dot:
            return ss::border_style_t::dash_dot_dot;
        case gnumeric_border_type::border_medium_dash_dot_dot:
            return ss::border_style_t::medium_dash_dot_dot;
        case gnumeric_border_type::border_slanted_dash_dot:
            return ss::border_style_t::slant_dash_dot;
    }

    return ss::border_style_t::unknown;
}

gnumeric_value_format_type to_gnumeric_value_format_type(std::string_view s)
{
    return value_format_type::get().find(s);
}

void gnumeric_named_exp::reset()
{
    name = std::string_view{};
    value = std::string_view{};
    position = {0, 0, 0};
}

bool gnumeric_style::valid() const
{
    if (sheet < 0)
        return false;

    if (region.first.column < 0 || region.first.row < 0 || region.last.column < 0 || region.last.row < 0)
        return false;

    return true;
}

std::optional<spreadsheet::color_rgb_t> parse_gnumeric_rgb(std::string_view v)
{
    ss::color_rgb_t color;

    auto pos = v.find(':');
    if (pos == v.npos)
        return {};

    std::string_view seg = v.substr(0, pos);

    std::optional<std::uint16_t> elem = hex_to_uint16(seg);
    if (!elem)
        return {};

    color.red = *elem >> 8;  // 16-bit to 8-bit
    v = v.substr(pos + 1);
    pos = v.find(':');
    if (pos == v.npos)
        return {};

    seg = v.substr(0, pos);
    elem = hex_to_uint16(seg);
    if (!elem)
        return {};

    color.green = *elem >> 8;
    v = v.substr(pos + 1);
    elem = hex_to_uint16(v);
    if (!elem)
        return {};

    color.blue = *elem >> 8;
    return color;
}

std::optional<spreadsheet::color_rgb_t> parse_gnumeric_rgb_8x(std::string_view v)
{
    ss::color_rgb_t color;

    auto pos = v.find('x');
    if (pos == v.npos)
        return {};

    std::string_view seg = v.substr(0, pos);

    std::optional<std::uint16_t> elem = hex_to_uint8(seg);
    if (!elem)
        return {};

    color.red = *elem;
    v = v.substr(pos + 1);
    pos = v.find('x');
    if (pos == v.npos)
        return {};

    seg = v.substr(0, pos);
    elem = hex_to_uint8(seg);
    if (!elem)
        return {};

    color.green = *elem;
    v = v.substr(pos + 1);
    elem = hex_to_uint8(v);
    if (!elem)
        return {};

    color.blue = *elem;
    return color;
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
