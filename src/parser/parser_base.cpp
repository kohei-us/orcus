/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/parser_base.hpp"
#include "orcus/parser_global.hpp"
#include "cpu_features.hpp"

#include <sstream>
#include <cstring>
#include <limits>
#include <cassert>

#ifdef __ORCUS_CPU_FEATURES
#include <immintrin.h>
#endif

namespace orcus {

parse_error::parse_error(const std::string& msg, std::ptrdiff_t offset) :
    general_error(msg), m_offset(offset) {}

parse_error::parse_error(const std::string& cls, const std::string& msg, std::ptrdiff_t offset) :
    general_error(cls, msg), m_offset(offset) {}

std::ptrdiff_t parse_error::offset() const
{
    return m_offset;
}

std::string parse_error::build_message(const char* msg_before, char c, const char* msg_after)
{
    std::ostringstream os;

    if (msg_before)
        os << msg_before;

    os << c;

    if (msg_after)
        os << msg_after;

    return os.str();
}

std::string parse_error::build_message(
    const char* msg_before, const char* p, size_t n, const char* msg_after)
{
    std::ostringstream os;

    if (msg_before)
        os << msg_before;

    write_to(os, p, n);

    if (msg_after)
        os << msg_after;

    return os.str();
}

parser_base::parser_base(const char* p, size_t n) :
    mp_begin(p), mp_char(p), mp_end(p+n)
{
}

void parser_base::next(size_t inc)
{
    mp_char += inc;
}

void parser_base::prev(size_t dec)
{
    mp_char -= dec;
}

char parser_base::cur_char() const
{
    return *mp_char;
}

char parser_base::next_char() const
{
    return *(mp_char+1);
}

void parser_base::skip(const char* chars_to_skip, size_t n_chars_to_skip)
{
#if defined(__ORCUS_CPU_FEATURES) && defined(__SSE4_2__)
    __m128i match = _mm_loadu_si128((const __m128i*)chars_to_skip);
    const int mode = _SIDD_LEAST_SIGNIFICANT | _SIDD_CMP_EQUAL_ANY | _SIDD_UBYTE_OPS | _SIDD_NEGATIVE_POLARITY;

    int n_total = available_size();

    while (n_total)
    {
        __m128i char_block = _mm_loadu_si128((const __m128i*)mp_char);

        // Find position of the first character that is NOT any of the
        // characters to skip.
        int n = std::min<int>(16, n_total);
        int r = _mm_cmpestri(match, n_chars_to_skip, char_block, n, mode);

        if (!r)
            // No characters to skip. Bail out.
            break;

        mp_char += r; // Move the current char position.

        if (r < 16)
            // No need to move to the next segment. Stop here.
            break;

        // Skip 16 chars to the next segment.
        n_total -= 16;
    }
#else
    for (; has_char(); next())
    {
        if (!is_in(*mp_char, chars_to_skip, n_chars_to_skip))
            break;
    }
#endif
}

void parser_base::skip_space_and_control()
{
#if defined(__ORCUS_CPU_FEATURES) && defined(__SSE4_2__)
    __m128i match = _mm_loadu_si128((const __m128i*)"\0 ");
    const int mode = _SIDD_LEAST_SIGNIFICANT | _SIDD_CMP_RANGES | _SIDD_UBYTE_OPS | _SIDD_NEGATIVE_POLARITY;

    int n_total = available_size();

    while (n_total)
    {
        __m128i char_block = _mm_loadu_si128((const __m128i*)mp_char);

        // Find position of the first character that is NOT any of the
        // characters to skip.
        int n = std::min<int>(16, n_total);
        int r = _mm_cmpestri(match, 2, char_block, n, mode);

        if (!r)
            // No characters to skip. Bail out.
            break;

        mp_char += r; // Move the current char position.

        if (r < 16)
            // No need to move to the next segment. Stop here.
            break;

        // Skip 16 chars to the next segment.
        n_total -= 16;
    }
#else
    for (; mp_char != mp_end && ((unsigned char)*mp_char) <= (unsigned char)' '; ++mp_char)
        ;
#endif
}

bool parser_base::parse_expected(const char* expected, size_t n_expected)
{
    if (n_expected > available_size())
        return false;

    for (size_t i = 0; i < n_expected; ++i, ++expected, next())
    {
        if (cur_char() != *expected)
            return false;
    }

    return true;
}

double parser_base::parse_double()
{
    size_t max_length = available_size();
    const char* p = mp_char;
    double val = parse_numeric(p, max_length);
    if (p == mp_char)
        return std::numeric_limits<double>::quiet_NaN();

    mp_char = p;
    return val;
}

size_t parser_base::remaining_size() const
{
    size_t n = available_size();
    return n ? (n - 1) : 0;
}

std::ptrdiff_t parser_base::offset() const
{
    return std::distance(mp_begin, mp_char);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
