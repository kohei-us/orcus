/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "utf8.hpp"

#include <cassert>
#include <stdexcept>
#include <limits>

// https://en.wikipedia.org/wiki/UTF-8#Encoding
// https://www.w3.org/TR/2006/REC-xml11-20060816/#NT-NameStartChar

namespace orcus {

namespace {

uint8_t calc_byte_length(uint8_t c1)
{
    if ((c1 & 0x80) == 0x00)
        // highest bit is not set.
        return 1;

    if ((c1 & 0xE0) == 0xC0)
        // highest 3 bits are 110.
        return 2;

    if ((c1 & 0xF0) == 0xE0)
        // highest 4 bits are 1110.
        return 3;

    if ((c1 & 0xFC) == 0xF0)
        // highest 5 bits are 11110.
        return 4;

    return std::numeric_limits<uint8_t>::max();
}

bool valid_second_byte(uint8_t b)
{
    return (b & 0xC0) == 0x80;
}

bool parse_1b_start_char(uint8_t c1)
{
    if (c1 == '_')
        return true;

    if ('a' <= c1 && c1 <= 'z')
        return true;

    if ('A' <= c1 && c1 <= 'Z')
        return true;

    return false;
}

bool parse_1b_second_char(uint8_t c1)
{
    if (c1 == '-' || c1 == '.')
        return true;

    if ('0' <= c1 && c1 <= '9')
        return true;

    return false;
}

//  [ #xC0- #xD6]:      C3 80 -> C3 96
//  [ #xD8- #xF6]:      C3 98 -> C3 B6
//  [ #xF8-#x2FF]:      C3 B8 -> CB BF
//  [#x370-#x37D]:      CD B0 -> CD BD
//
//  [#x37F-#x1FFF]:     (split)
//  [#x37F-#x07FF]:     CD BF -> DF BF
//
bool parse_2b_start_char(uint8_t c1, uint8_t c2)
{
    if (c1 == 0xC3)
    {
        if (0x80 <= c2 && c2 <= 0x96)
            return true;

        if (0x98 <= c2 && c2 <= 0xB6)
            return true;

        if (0xB8 <= c2)
            return true;
    }

    // C4 80 -> CB BF
    if (0xC4 <= c1 && c1 <= 0xCB)
        return 0x80 <= c2 && c2 <= 0xBF;

    // CD B0 -> CD BD
    // CD BF -> DF BF

    if (c1 == 0xCD)
    {
        if (0xB0 <= c2 && c2 <= 0xBD)
            return true;

        return c2 == 0xBF;
    }

    // CE xx -> DF xx
    return 0xCE <= c1 && c1 <= 0xDF;
}

// #xB7:            C2 B7
// [#x0300-#x036F]: CC 80 -> CD AF
bool parse_2b_second_char(uint8_t c1, uint8_t c2)
{
    // C2 B7
    if (c1 == 0xC2)
        return c2 == 0xB7;

    // CC 80 -> CD AF
    //  - CC xx
    //  - CD xx -> CD AF

    if (c1 == 0xCC)
        return true;

    if (c1 == 0xCD)
        return c2 <= 0xAF;

    return false;
}

// [#x800-#x1FFF]:     E0 A0 80 -> E1 BF BF
//
// [#x200C-#x200D]:    E2 80 8C -> E2 80 8D
// [#x2070-#x218F]:    E2 81 B0 -> E2 86 8F
// [#x2C00-#x2FEF]:    E2 B0 80 -> E2 BF AF
// [#x3001-#xD7FF]:    E3 80 81 -> ED 9F BF
// [#xF900-#xFDCF]:    EF A4 80 -> EF B7 8F
// [#xFDF0-#xFFFD]:    EF B7 B0 -> EF BF BD
bool parse_3b_start_char(uint8_t c1, uint8_t c2, uint8_t c3)
{
    // E0 A0 80 -> E1 BF BF
    //  - E0 A0 80 -> E0 BF BF
    //  - E1 xx xx

    if (c1 == 0xE0)
    {
        // A0 80 -> BF BF
        return (0xA0 <= c2 && c2 <= 0xBF && 0x80 <= c3 && c3 <= 0xBF);
    }

    if (c1 == 0xE1)
        // entire E1 xx xx range is valid.
        return true;

    if (c1 == 0xE2)
    {
        // E2 80 8C -> E2 80 8D
        // E2 81 B0 -> E2 86 8F
        // E2 B0 80 -> E2 BF AF

        if (c2 == 0x80)
            // 8C -> 8D
            return c3 == 0x8C || c3 == 0x8D;

        // 81 B0 -> 86 8F
        if (c2 == 0x81)
            return c3 >= 0xB0;

        if (0x82 <= c2 && c2 <= 0x85)
            return true;

        if (c2 == 0x86)
            return c3 <= 0x8F;

        // B0 80 -> BF AF
        if (0xB0 <= c2 && c2 <= 0xBE)
            return true;

        if (c2 == 0xBF)
            return c3 <= 0xAF;
    }

    // E3 80 81 -> ED 9F BF
    //  - E3 80 81 -> E3 80 BF
    //  - E3 81 xx
    //  - E4 xx xx -> EC xx xx
    //  - ED xx xx -> ED 9F xx
    if (c1 == 0xE3)
    {
        if (c2 == 0x80)
            return c3 >= 0x81;

        return 0x81 <= c2;
    }

    if (0xE4 <= c1 && c1 <= 0xEC)
        return true;

    if (c1 == 0xED)
        return c2 <= 0x9F;

    // EF A4 80 -> EF B7 8F
    //  - EF A4 xx
    //  - EF A5 xx -> EF B6 xx
    //  - EF B7 xx -> EF B7 8F
    // EF B7 B0 -> EF BF BD
    //  - EF B7 B0 -> EF B7 xx
    //  - EF B8 xx -> EF BE xx
    //  - EF BF xx -> EF BF BD
    if (c1 == 0xEF)
    {
        if (c2 == 0xA4)
            return true;

        if (0xA5 <= c2 && c2 <= 0xB6)
            return true;

        if (c2 == 0xB7)
        {
            if (c3 <= 0x8F)
                return true;

            return 0xB0 <= c3;
        }

        if (0xB8 <= c2 && c2 <= 0xBE)
            return true;

        if (c2 == 0xBF)
            return c3 <= 0xBD;
    }

    return false;
}

// [#x203F-#x2040]: E2 80 BF -> E2 81 80
bool parse_3b_second_char(uint8_t c1, uint8_t c2, uint8_t c3)
{
    if (c1 != 0xE2)
        return false;

    if (c2 == 0x80)
        return c3 == 0xBF;

    if (c2 == 0x81)
        return c3 == 0x80;

    return false;
}

// [#x10000-#xEFFFF]:  F0 90 80 80 -> F3 AF BF BF
bool parse_4b_char(uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4)
{
    // F0 90 80 80 -> F3 AF BF BF
    //  - F0 90 xx xx -> F0 xx xx xx
    //  - F1 xx xx xx -> F2 xx xx xx
    //  - F3 xx xx xx -> F3 AF xx xx
    if (c1 == 0xF0)
        return 0x90 <= c2;

    if (0xF1 <= c1 && c1 <= 0xF2)
        return true;

    if (c1 == 0xF3)
        return c2 <= 0xAF;

    return false;
}

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

} // anonymous namespace

const char* parse_utf8_xml_name_start_char(const char* p, const char* p_end)
{
    size_t n_remaining = p_end - p;
    if (!n_remaining)
        return p;

    uint8_t n_bytes = calc_byte_length(*p);

    switch (n_bytes)
    {
        case 1:
            return parse_1b_start_char(*p) ? p + 1 : p;
        case 2:
        {
            if (n_remaining < 2)
                return p;

            uint8_t c1 = p[0];
            uint8_t c2 = p[1];

            if (!valid_second_byte(c2))
                return p;

            return parse_2b_start_char(c1, c2) ? p + 2 : p;
        }
        case 3:
        {
            if (n_remaining < 3)
                return p;

            uint8_t c1 = p[0];
            uint8_t c2 = p[1];
            uint8_t c3 = p[2];

            if (!valid_second_byte(c2) || !valid_second_byte(c3))
                return p;

            return parse_3b_start_char(c1, c2, c3) ? p + 3 : p;
        }
        case 4:
        {
            if (n_remaining < 4)
                return p;

            uint8_t c1 = p[0];
            uint8_t c2 = p[1];
            uint8_t c3 = p[2];
            uint8_t c4 = p[3];

            if (!valid_second_byte(c2) || !valid_second_byte(c3) || !valid_second_byte(c4))
                return p;

            return parse_4b_char(c1, c2, c3, c4) ? p + 4 : p;
        }
    }

    return p;
}

const char* parse_utf8_xml_name_char(const char* p, const char* p_end)
{
    size_t n_remaining = p_end - p;
    if (!n_remaining)
        return p;

    uint8_t n_bytes = calc_byte_length(*p);

    switch (n_bytes)
    {
        case 1:
        {
            if (parse_1b_start_char(*p))
                return p + 1;

            return parse_1b_second_char(*p) ? p + 1 : p;
        }
        case 2:
        {
            if (n_remaining < 2)
                return p;

            uint8_t c1 = p[0];
            uint8_t c2 = p[1];

            if (!valid_second_byte(c2))
                return p;

            if (parse_2b_start_char(c1, c2))
                return p + 2;

            return parse_2b_second_char(c1, c2) ? p + 2 : p;
        }
        case 3:
        {
            if (n_remaining < 3)
                return p;

            uint8_t c1 = p[0];
            uint8_t c2 = p[1];
            uint8_t c3 = p[2];

            if (!valid_second_byte(c2) || !valid_second_byte(c3))
                return p;

            if (parse_3b_start_char(c1, c2, c3))
                return p + 3;

            return parse_3b_second_char(c1, c2, c3) ? p + 3 : p;
        }
        case 4:
        {
            if (n_remaining < 4)
                return p;

            uint8_t c1 = p[0];
            uint8_t c2 = p[1];
            uint8_t c3 = p[2];
            uint8_t c4 = p[3];

            if (!valid_second_byte(c2) || !valid_second_byte(c3) || !valid_second_byte(c4))
                return p;

            return parse_4b_char(c1, c2, c3, c4) ? p + 4 : p;
        }
    }

    return p;
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
