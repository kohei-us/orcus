/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/css_parser_base.hpp>
#include <orcus/parser_global.hpp>

#include "utf8.hpp"

#include <cstring>
#include <cassert>
#include <cmath>
#include <limits>

namespace orcus { namespace css {

parser_base::parser_base(std::string_view content) :
    orcus::parser_base(content.data(), content.size()),
    m_simple_selector_count(0),
    m_combinator(combinator_t::descendant) {}

void parser_base::identifier(const char*& p, size_t& len, std::string_view extra)
{
    p = mp_char;
    len = 1;
    for (next(); has_char(); next(), ++len)
    {
        char c = cur_char();
        if (is_alpha(c) || is_numeric(c) || is_in(c, "-_"))
            continue;

        if (!extra.empty())
        {
            // See if the character is one of the extra allowed characters.
            if (is_in(c, extra))
                continue;
        }
        return;
    }
}

uint8_t parser_base::parse_uint8()
{
    // 0 - 255
    int val = 0;
    size_t len = 0;
    for (; has_char() && len <= 3; next())
    {
        char c = cur_char();
        if (!is_numeric(c))
            break;

        ++len;
        val *= 10;
        val += c - '0';
    }

    if (!len)
        throw parse_error("parse_uint8: no digit encountered.", offset());

    int maxval = std::numeric_limits<uint8_t>::max();
    if (val > maxval)
        val = maxval;

    return static_cast<uint8_t>(val);
}

std::string_view parser_base::parse_value()
{
    auto throw_invalid = [this](uint8_t n_bytes)
    {
        std::ostringstream os;
        os << "parse_value: invalid utf-8 byte length (" << int(n_bytes) << ")";
        throw parse_error(os.str(), offset());
    };

    auto check_byte_length_or_throw = [this](uint8_t n_bytes, std::size_t max_size)
    {
        if (std::size_t(n_bytes) > max_size)
        {
            std::ostringstream os;
            os << "parse_value: utf-8 byte length is " << int(n_bytes) << " but only " << max_size << " bytes remaining.";
            throw parse_error(os.str(), offset());
        }
    };

    std::size_t max_size = available_size();
    if (!max_size)
        return {};

    const char* p0 = mp_char;
    std::size_t len = 0;

    char c = cur_char();
    uint8_t n_bytes = calc_utf8_byte_length(c);

    // any of '-+.#' is allowed as first character, while any of '-_.%' is
    // allowed as second characters.

    switch (n_bytes)
    {
        case 1:
        {
            if (!is_alpha(c) && !is_numeric(c) && !is_in(c, "-+.#"))
                parse_error::throw_with("parse_value: illegal first character of a value '", c, "'", offset());
            break;
        }
        case 2:
        case 3:
        case 4:
        {
            check_byte_length_or_throw(n_bytes, max_size);
            break;
        }
        default:
            throw_invalid(n_bytes);
    }

    len += n_bytes;
    next(n_bytes);

    while (has_char())
    {
        c = cur_char();
        max_size = available_size();
        n_bytes = calc_utf8_byte_length(c);

        switch (n_bytes)
        {
            case 1:
            {
                if (!is_alpha(c) && !is_numeric(c) && !is_in(c, "-_.%"))
                    return {p0, len};
                break;
            }
            case 2:
            case 3:
            case 4:
            {
                check_byte_length_or_throw(n_bytes, max_size);
                break;
            }
            default:
                throw_invalid(n_bytes);
        }

        len += n_bytes;
        next(n_bytes);
    }

    return {p0, len};
}

double parser_base::parse_percent()
{
    double v = parse_double_or_throw();

    if (*mp_char != '%')
        parse_error::throw_with(
            "parse_percent: '%' expected after the numeric value, but '", *mp_char, "' found.", offset());

    next(); // skip the '%'.
    return v;
}

double parser_base::parse_double_or_throw()
{
    double v = parse_double();
    if (std::isnan(v))
        throw parse_error("parse_double: failed to parse double precision value.", offset());
    return v;
}

void parser_base::literal(const char*& p, size_t& len, char quote)
{
    assert(cur_char() == quote);
    next();
    skip_to(p, len, quote);

    if (cur_char() != quote)
        throw parse_error("literal: end quote has never been reached.", offset());
}

void parser_base::skip_to(const char*&p, size_t& len, char c)
{
    p = mp_char;
    len = 0;
    for (; has_char(); next(), ++len)
    {
        if (cur_char() == c)
            return;
    }
}

void parser_base::skip_to_or_blank(const char*&p, size_t& len, std::string_view chars)
{
    p = mp_char;
    len = 0;
    for (; has_char(); next(), ++len)
    {
        if (is_blank(*mp_char) || is_in(*mp_char, chars))
            return;
    }
}

void parser_base::skip_blanks()
{
    skip(" \t\r\n");
}

void parser_base::skip_blanks_reverse()
{
    const char* p = mp_char + remaining_size();
    for (; p != mp_char; --p, --mp_end)
    {
        if (!is_blank(*p))
            break;
    }
}

void parser_base::shrink_stream()
{
    // Skip any leading blanks.
    skip_blanks();

    if (!remaining_size())
        return;

    // Skip any trailing blanks.
    skip_blanks_reverse();

    // Skip leading <!-- if present.

    const char* com_open = "<!--";
    size_t com_open_len = std::strlen(com_open);
    if (remaining_size() < com_open_len)
        // Not enough stream left.  Bail out.
        return;

    const char* p = mp_char;
    for (size_t i = 0; i < com_open_len; ++i, ++p)
    {
        if (*p != com_open[i])
            return;
        next();
    }
    mp_char = p;

    // Skip leading blanks once again.
    skip_blanks();

    // Skip trailing --> if present.
    const char* com_close = "-->";
    size_t com_close_len = std::strlen(com_close);
    size_t n = remaining_size();
    if (n < com_close_len)
        // Not enough stream left.  Bail out.
        return;

    p = mp_char + n; // move to the last char.
    for (size_t i = com_close_len; i > 0; --i, --p)
    {
        if (*p != com_close[i-1])
            return;
    }
    mp_end -= com_close_len;

    skip_blanks_reverse();
}

bool parser_base::skip_comment()
{
    char c = cur_char();
    if (c != '/')
        return false;

    if (remaining_size() > 2 && peek_char() == '*')
    {
        next();
        comment();
        skip_blanks();
        return true;
    }

    return false;
}

void parser_base::comment()
{
    assert(cur_char() == '*');

    // Parse until we reach either EOF or '*/'.
    bool has_star = false;
    for (next(); has_char(); next())
    {
        char c = cur_char();
        if (has_star && c == '/')
        {
            next();
            return;
        }
        has_star = (c == '*');
    }

    // EOF reached.
}

void parser_base::skip_comments_and_blanks()
{
    skip_blanks();
    while (skip_comment())
        ;
}

void parser_base::set_combinator(char c, css::combinator_t combinator)
{
    if (!m_simple_selector_count)
        parse_error::throw_with(
            "set_combinator: combinator '", c, "' encountered without parent element.", offset());

    m_combinator = combinator;
    next();
    skip_comments_and_blanks();
}

void parser_base::reset_before_block()
{
    m_simple_selector_count = 0;
    m_combinator = css::combinator_t::descendant;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
