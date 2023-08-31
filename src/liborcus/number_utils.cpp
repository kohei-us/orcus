/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "number_utils.hpp"

namespace orcus {

namespace {

std::optional<std::uint8_t> char_to_uint8(char c)
{
    std::uint8_t v;
    if ('0' <= c && c <= '9')
    {
        v = c - '0';
        return v;
    }

    if ('A' <= c && c <= 'F')
    {
        v = c - 'A' + 10;
        return v;
    }

    if ('a' <= c && c <= 'f')
    {
        v = c - 'a' + 10;
        return v;
    }

    return {};
}

template<typename IntT>
std::optional<IntT> hex_to_uint(std::string_view s)
{
    static_assert(std::is_integral_v<IntT>);

    constexpr std::size_t expected_len = sizeof(IntT) * 2u;
    if (s.size() > expected_len)
        return {};

    IntT value = 0;
    for (char c : s)
    {
        value = value << 4;
        auto v = char_to_uint8(c);
        if (!v)
            return {};
        value += *v;
    }

    return value;
}

} // anonymous namespace

std::optional<std::uint8_t> hex_to_uint8(std::string_view s)
{
    return hex_to_uint<std::uint8_t>(s);
}

std::optional<std::uint16_t> hex_to_uint16(std::string_view s)
{
    return hex_to_uint<std::uint16_t>(s);
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
