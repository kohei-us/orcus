/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gnumeric_types.hpp"
#include "number_utils.hpp"

namespace ss = orcus::spreadsheet;

namespace orcus {

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

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
