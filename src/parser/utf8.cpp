/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "utf8.hpp"

#include <cassert>
#include <stdexcept>

namespace orcus {

// https://www.w3.org/TR/2006/REC-xml11-20060816/#NT-NameStartChar
// Return length of the character in bytes, otherwise 0.
template< bool only_start_name >
static
int is_name_char_helper(const char* mp_char, const char* mp_end)
{
    const unsigned char first = mp_char[0];
    // Note that ':' technically is an allowed name character, but it is handled separately
    // e.g. in element_name(), so here pretend it isn't.
    if (/*first == ':' ||*/ first == '_' || (first >= 'A' && first <= 'Z') || (first >= 'a' && first <= 'z'))
        return 1;
    if (!only_start_name && (first == '-' || first == '.' || (first >= '0' && first <= '9')))
        return 1;

    if (first < 0x7f) // other ascii characters are not allowed
        return 0;
    if (mp_end < mp_char + 1)
        return 0;
    const unsigned char second = mp_char[1];

    // 0xb7 = 0xc2 0xb7 utf-8
    if (!only_start_name && first == 0xc2 && second == 0xb7)
        return 2;

    // [#xC0-#xD6] | [#xD8-#xF6] | [#xF8-#x2FF]
    // 0xc0 = 0xc3 0x80 utf-8
    if (first < 0xc3)
        return 0;
    // xd7 = 0xc3 0x97 utf-8, 0xf7 = 0xc3 0xb7 utf-8
    if (first == 0xc3)
        return second >= 0x80 && second <= 0xff && second != 0x97 && second != 0xb7 ? 2 : 0;
    // 0x2ff = 0xcb 0xbf utf-8, 0x300 = 0xcc 0x80 utf-8
    if (first >= 0xc4 && first <= 0xcb)
        return 2;

    // [#x0300-#x036F]
    // 0x0300 = 0xcc 0x80 utf-8, 0x36f = 0xcd 0xaf utf-8
    if (!only_start_name && first == 0xcc)
        return 2;
    if (!only_start_name && first == 0xcd && second <= 0xaf)
        return 2;

    // [#x370-#x37D] | [#x37F-#x1FFF]
    // 0x370 = 0xcd 0xb0 utf-8, 0x37e = 0xcd 0xbe
    if (first < 0xcd)
        return 0;
    if (first == 0xcd)
        return second >= 0xb0 && second != 0xbe ? 2 : 0;
    // 0x07ff = 0xdf 0xbf utf-8 (the last 2-byte utf-8)
    if (first <= 0xdf)
        return 2;

    if (first < 0xe0)
        return 0;
    if (mp_end < mp_char + 2)
        return 0;
    const unsigned char third = mp_char[2];

    // 0x0800 = 0xe0 0xa0 0x80 utf-8, 0x1fff = 0xe1 0xbf 0xbf utf-8, 0x2000 = 0xe2 0x80 0x80
    if (first == 0xe0 || first == 0xe1)
        return 3;

    // [#x200C-#x200D]
    // 0x200c = 0xe2 0x80 0x8c utf-8, 0x200d = 0xe2 0x80 0x8d utf-8
    if (first < 0xe2)
        return 0;
    if (first == 0xe2 && second == 0x80 && (third == 0x8c || third == 0x8d))
        return 3;

    // [#x203F-#x2040]
    // 0x203f = 0xe2 0x80 0xbf utf-8, 0x2040 = 0xe2 0x81 0x80 utf-8
    if (!only_start_name && first == 0xe2 && second == 0x80 && third == 0xbf)
        return 3;
    if (!only_start_name && first == 0xe2 && second == 0x81 && third == 0x80)
        return 3;

    // [#x2070-#x218F]
    // 0x2070 = 0xe2 0x81 0xb0 utf-8, 0x218f = 0xe2 0x86 0x8f utf-8
    if (first == 0xe2)
    {
        if (second < 0x81)
            return 0;
        if (second >= 0x81 && second < 0x86)
            return 3;
        if (second == 0x86 && third <= 0x8f)
            return 3;
    }

    // [#x2C00-#x2FEF]
    // 0x2c00 = 0xe2 0xb0 0x80 utf-8, 0x2fef = 0xe2 0xbf 0xaf utf-8
    if (first == 0xe2)
    {
        if (second < 0xb0)
            return 0;
        if (second < 0xbf)
            return 3;
        if (second == 0xbf && third <= 0xaf)
            return 3;
    }

    // [#x3001-#xD7FF]
    // 0x3001 = 0xe3 0x80 0x81 utf-8, 0xd7ff = 0xed 0x9f 0xbf utf-8, 0xd800 = 0xed 0xa0 0x80 utf-8
    if (first < 0xe3)
        return 0;
    if (first < 0xed)
        return 3;
    if (first == 0xed && second <= 0x9f)
        return 3;

    // [#xF900-#xFDCF]
    // 0xf900 = 0xef 0xa4 0x80 utf-8, 0xfdcf = 0xef 0xb7 0x8f utf-8
    if (first == 0xef)
    {
        if (second < 0xa4)
            return 0;
        if (second < 0xb7)
            return 3;
        if (second == 0xb7 && third <= 0x8f)
            return 3;
    }

    // [#xFDF0-#xFFFD]
    // 0xfdf0 = 0xef 0xb7 0xb0 utf-8, 0xfffd = 0xef 0xbf 0xbd utf-8
    if (first == 0xef)
    {
        assert(second >= 0xb7);
        if (second == 0xb7 && third < 0xb0)
            return 0;
        if (second < 0xbe)
            return 3;
        if (second == 0xbf && third <= 0xbd)
            return 3;
    }

    if (first < 0xf0)
        return 0;
    if (mp_end < mp_char + 3)
        return 0;
    // const unsigned char fourth = mp_char[3];

    // [#x10000-#xEFFFF]
    // 0x10000 = 0xf0 0x90 0x80 0x80 utf-8, 0xeffff = 0xf3 0xaf 0xbf 0xbf utf-8,
    // 0xf0000 = 0xf3 0xb0 0x80 0x80 utf-8
    if (first >= 0xf0 && first < 0xf2)
        return 4;
    if (first == 0xf3 && second < 0xb0)
        return 4;

    return 0;
}

const char* parse_utf8_xml_name_start_char(const char* p, const char* p_end)
{
    int skip = is_name_char_helper<true>(p, p_end);
    return p + skip;
}

const char* parse_utf8_xml_name_char(const char* p, const char* p_end)
{
    int skip = is_name_char_helper<false>(p, p_end);
    return p + skip;
}

namespace {

uint8_t calc_encoded_length(uint32_t cp)
{
    if (cp < 0x7F)
        return 1;

    if (0x80 <= cp && cp <= 0x7FF)
        return 2;

    if (0x800 <= cp && cp <= 0xFFFF)
        return 3;

    if (0x10000 <= cp && cp <= 0x10FFFF)
        return 4;

    throw std::runtime_error("invalid utf-8 range.");
}

// input must be less than or equal to 0x7FF
//
// b1: 0b110xxxxx (5)
// b2: 0b10xxxxxx (6)
std::vector<char> encode_2b(uint32_t cp)
{
    assert(cp <= 0x7FF);

    // Get the lowest 6 bits
    char low = (cp & 0x3F);
    low |= 0x80;

    // Get the next 5 bits
    cp = cp >> 6;
    char high = (cp & 0x1F);
    high |= 0xC0;

    std::vector<char> ret = { high, low };
    return ret;
}

// input must be less than or equal to 0xFFFF
//
// b1: 0b1110xxxx (4)
// b2: 0b10xxxxxx (6)
// b3: 0b10xxxxxx (6)
std::vector<char> encode_3b(uint32_t cp)
{
    assert(cp <= 0xFFFF);

    // Get the lowest 6 bits
    char low = (cp & 0x3F);
    low |= 0x80;
    cp = cp >> 6;

    // Get the middle 6 bits
    char mid = (cp & 0x3F);
    mid |= 0x80;
    cp = cp >> 6;

    // Get the next 4 bits
    char high = (cp & 0x0F);
    high |= 0xE0;

    std::vector<char> ret = { high, mid, low };
    return ret;
}

// input must be less than or equal to 0x10FFFF
//
// b1: 0b11110xxx (3)
// b2: 0b10xxxxxx (6)
// b3: 0b10xxxxxx (6)
// b4: 0b10xxxxxx (6)
std::vector<char> encode_4b(uint32_t cp)
{
    assert(cp <= 0x10FFFF);

    // Get the lowest 6 bits
    char low = (cp & 0x3F);
    low |= 0x80;
    cp = cp >> 6;

    // Get the next 6 bits
    char mid1 = (cp & 0x3F);
    mid1 |= 0x80;
    cp = cp >> 6;

    // Get the next 6 bits
    char mid2 = (cp & 0x3F);
    mid2 |= 0x80;
    cp = cp >> 6;

    // Get the next 3 bits
    char high = (cp & 0x07);
    high |= 0xF0;

    std::vector<char> ret = { high, mid2, mid1, low };
    return ret;
}

}

std::vector<char> encode_utf8(uint32_t cp)
{
    uint8_t n_encoded = calc_encoded_length(cp);

    switch (n_encoded)
    {
        case 1:
            // no conversion
            return std::vector<char>(1, cp);
        case 2:
            return encode_2b(cp);
        case 3:
            return encode_3b(cp);
        case 4:
            return encode_4b(cp);
    }

    throw std::logic_error("this should never be reached.");
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
