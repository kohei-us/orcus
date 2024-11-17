/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/parser_global.hpp>
#include <orcus/cell_buffer.hpp>
#include <orcus/exception.hpp>

#include "numeric_parser.hpp"
#include "utf8.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <algorithm>
#include <cctype>

namespace orcus {

bool is_blank(char c)
{
    return is_in(c, " \t\n\r");
}

bool is_alpha(char c)
{
    return std::isalpha(static_cast<unsigned char>(c));
}

bool is_numeric(char c)
{
    return std::isdigit(static_cast<unsigned char>(c));
}

bool is_in(char c, std::string_view allowed)
{
#ifdef __ORCUS_DEBUG_UTILS
    if (allowed.empty())
        throw std::invalid_argument("'allowed' string should not be empty.");
#endif
    auto f = [c](char c_allowed) { return c == c_allowed; };
    return std::any_of(allowed.begin(), allowed.end(), f);
}

const char* parse_numeric(const char* p, const char* p_end, double& value)
{
    using numeric_parser_type = detail::numeric_parser<detail::generic_parser_trait>;

    numeric_parser_type parser(p, p_end);
    double v = parser.parse();
    if (!std::isnan(v))
        p = parser.get_char_position();

    value = v;
    return p;
}

const char* parse_integer(const char* p, const char* p_end, long& value)
{
    if (p >= p_end)
        return p;

    long result = 0;
    bool negative_sign = false;

    // Check for presence of a sign.
    if (p != p_end)
    {
        switch (*p)
        {
            case '+':
                ++p;
                break;
            case '-':
                negative_sign = true;
                ++p;
                break;
            default:
                ;
        }
    }

    for (; p != p_end; ++p)
    {
        if (*p < '0' || '9' < *p)
        {
            value = negative_sign ? -result : result;
            return p;
        }

        result *= 10;
        result += *p - '0';
    }

    value = negative_sign ? -result : result;
    return p;
}

string_escape_char_t get_string_escape_char_type(char c)
{
    switch (c)
    {
        case '"':
        case '\\':
        case '/':
            return string_escape_char_t::regular_char;
        case 'b': // backspace
        case 'f': // formfeed
        case 'n': // newline
        case 'r': // carriage return
        case 't': // horizontal tab
            return string_escape_char_t::control_char;
        case 'u':
            return string_escape_char_t::unicode;
    }

    return string_escape_char_t::invalid;
}

/**
 * Given the literal character immediately following the '\', convert it to an
 * escaped character.
 */
char to_escaped_char(char c)
{
    switch (c)
    {
        case 'b': // backspace
            return '\b';
        case 'f': // formfeed
            return '\f';
        case 'n': // newline
            return '\n';
        case 'r': // carriage return
            return '\r';
        case 't': // horizontal tab
            return '\t';
    }

    return c;
}

namespace {

enum class double_quoted_string_parse_mode_t
{
    unspecified,
    escaped,
    hex_digit,
};

parse_quoted_string_state parse_double_quoted_string_with_buffer(cell_buffer& buffer, const char*& p, const char* p_end)
{
    parse_quoted_string_state ret;
    ret.str = nullptr;
    ret.length = 0;
    ret.transient = true;
    ret.has_control_character = false;

    if (p == p_end)
    {
        ret.length = parse_quoted_string_state::error_no_closing_quote;
        return ret;
    }

    std::size_t len = 0;
    const char* p_head = nullptr;
    double_quoted_string_parse_mode_t mode = double_quoted_string_parse_mode_t::unspecified;

    for (; p != p_end; ++p, ++len)
    {
        char c = *p;

        if (!p_head)
        {
            p_head = p;
            len = 0;
        }

        switch (mode)
        {
            case double_quoted_string_parse_mode_t::escaped:
            {
                mode = double_quoted_string_parse_mode_t::unspecified;

                switch (get_string_escape_char_type(c))
                {
                    case string_escape_char_t::regular_char:
                    case string_escape_char_t::control_char:
                    {
                        c = to_escaped_char(c);
                        if (p_head && len > 1)
                            buffer.append(p_head, len-1);
                        buffer.append(&c, 1);
                        p_head = nullptr;
                        continue;
                        break;
                    }
                    case string_escape_char_t::unicode:
                    {
                        mode = double_quoted_string_parse_mode_t::hex_digit;
                        p_head = nullptr;
                        break;
                    }
                    case string_escape_char_t::invalid:
                    default:
                        ret.length = parse_quoted_string_state::error_illegal_escape_char;
                        return ret;
                }
                break;
            }
            case double_quoted_string_parse_mode_t::hex_digit:
            {
                if (!std::isxdigit(c))
                {
                    if (len != 4)
                    {
                        ret.length = parse_quoted_string_state::error_invalid_hex_digits;
                        return ret;
                    }

                    std::stringstream ss;
                    uint32_t cp;
                    ss << std::string_view{p_head, len};
                    ss >> std::hex >> cp;

                    auto encoded = encode_utf8(cp);
                    if (encoded.empty())
                    {
                        // failed to encode it as utf-8
                        ret.length = parse_quoted_string_state::error_invalid_hex_digits;
                        return ret;
                    }

                    buffer.append(encoded.data(), encoded.size());
                    p_head = nullptr;
                    mode = double_quoted_string_parse_mode_t::unspecified;

                    switch (c)
                    {
                        case '"': // closing quote
                        {
                            ++p; // skip the quote
                            std::string_view s = buffer.str();
                            ret.str = s.data();
                            ret.length = s.size();
                            return ret;
                        }
                        case '\\': // escape char
                        {
                            mode = double_quoted_string_parse_mode_t::escaped;
                            break;
                        }
                    }
                }
                break;
            }
            case double_quoted_string_parse_mode_t::unspecified:
            {
                switch (*p)
                {
                    case '"':
                    {
                        // closing quote.
                        buffer.append(p_head, len);
                        ++p; // skip the quote.
                        std::string_view s = buffer.str();
                        ret.str = s.data();
                        ret.length = s.size();
                        return ret;
                    }
                    case '\\':
                    {
                        mode = double_quoted_string_parse_mode_t::escaped;
                        continue;
                    }
                }
                break;
            }
        }
    }

    ret.length = parse_quoted_string_state::error_no_closing_quote;
    return ret;
}

parse_quoted_string_state parse_single_quoted_string_buffered(
    const char*& p, const char* p_end, cell_buffer& buffer)
{
    const char* p0 = p;
    size_t len = 0;
    char last = 0;

    parse_quoted_string_state ret;
    ret.transient = true;
    ret.has_control_character = false;

    for (; p != p_end; ++p)
    {
        if (!p0)
            p0 = p;

        char c = *p;
        switch (c)
        {
            case '\'':
            {
                if (last == c)
                {
                    // Second "'" in series.  This is an encoded single quote.
                    buffer.append(p0, len);
                    p0 = nullptr;
                    last = 0;
                    len = 0;
                    continue;
                }
            }
            break;
            default:
            {
                if (last == '\'')
                {
                    buffer.append(p0, len-1);
                    auto s = buffer.str();
                    ret.str = s.data();
                    ret.length = s.size();
                    return ret;
                }
            }
        }

        last = c;
        ++len;
    }

    if (last == '\'')
    {
        buffer.append(p0, len-1);
        auto s = buffer.str();
        ret.str = s.data();
        ret.length = s.size();
        return ret;
    }

    ret.str = nullptr;
    ret.length = parse_quoted_string_state::error_no_closing_quote;
    return ret;
}

} // anonymous namespace

parse_quoted_string_state parse_single_quoted_string(
    const char*& p, size_t max_length, cell_buffer& buffer)
{
    assert(*p == '\'');
    const char* p_end = p + max_length;
    ++p;

    parse_quoted_string_state ret;
    ret.str = p;
    ret.length = 0;
    ret.transient = false;
    ret.has_control_character = false;

    if (p == p_end)
    {
        ret.str = nullptr;
        ret.length = parse_quoted_string_state::error_no_closing_quote;
        return ret;
    }

    char last = 0;
    char c = 0;
    for (; p != p_end; last = c, ++p, ++ret.length)
    {
        c = *p;
        switch (c)
        {
            case '\'':
            {
                if (last == c)
                {
                    // Encoded single quote.
                    buffer.reset();
                    buffer.append(ret.str, ret.length);
                    ++p;
                    return parse_single_quoted_string_buffered(p, p_end, buffer);
                }
            }
            break;
            default:
            {
                if (last == '\'')
                {
                    --ret.length;
                    return ret;
                }
            }

        }
    }

    if (last == '\'')
    {
        --ret.length;
        return ret;
    }

    ret.str = nullptr;
    ret.length = parse_quoted_string_state::error_no_closing_quote;
    return ret;
}

const char* parse_to_closing_single_quote(const char* p, size_t max_length)
{
    assert(*p == '\'');
    const char* p_end = p + max_length;
    ++p;

    if (p == p_end)
        return nullptr;

    char last = 0;
    for (; p != p_end; ++p)
    {
        char c = *p;
        switch (c)
        {
            case '\'':
                if (last == '\'')
                {
                    last = 0;
                    continue;
                }
            break;
            default:
            {
                if (last == '\'')
                    return p;
            }
        }

        last = c;
    }

    if (last == '\'')
        return p;

    return nullptr;
}

parse_quoted_string_state parse_double_quoted_string(
    const char*& p, size_t max_length, cell_buffer& buffer)
{
    if (max_length == 0 || !p || *p != '"')
        throw invalid_arg_error("parse_double_quoted_string: invalid input string");

    parse_quoted_string_state ret;
    ret.str = nullptr;
    ret.length = 0;
    ret.transient = false;
    ret.has_control_character = false;

    const char* p_end = p + max_length;
    ++p; // skip the opening quote.

    ret.str = p;

    if (p == p_end)
    {
        // The string contains only the opening quote.
        ret.str = nullptr;
        ret.length = parse_quoted_string_state::error_no_closing_quote;
        return ret;
    }

    double_quoted_string_parse_mode_t mode = double_quoted_string_parse_mode_t::unspecified;
    const char* p_head = nullptr;

    for (; p != p_end; ++p, ++ret.length)
    {
        char c = *p;

        switch (mode)
        {
            case double_quoted_string_parse_mode_t::escaped:
            {
                switch (get_string_escape_char_type(c))
                {
                    case string_escape_char_t::regular_char:
                    case string_escape_char_t::control_char:
                    {
                        // Start the buffer with the string we've parsed so far.
                        buffer.reset();
                        if (ret.str && ret.length > 1)
                            buffer.append(ret.str, ret.length-1);

                        // add the escaped char to the buffer too
                        c = to_escaped_char(c);
                        buffer.append(&c, 1);

                        ++p; // skip the escaped char
                        return parse_double_quoted_string_with_buffer(buffer, p, p_end);
                    }
                    case string_escape_char_t::unicode:
                    {
                        mode = double_quoted_string_parse_mode_t::hex_digit;
                        break;
                    }
                    case string_escape_char_t::invalid:
                        ret.str = nullptr;
                        ret.length = parse_quoted_string_state::error_illegal_escape_char;
                        return ret;
                }

                break;
            }
            case double_quoted_string_parse_mode_t::hex_digit:
            {
                if (!p_head)
                    p_head = p;

                if (!std::isxdigit(c))
                {
                    std::size_t n_digits = std::distance(p_head, p);
                    if (n_digits != 4)
                    {
                        ret.str = nullptr;
                        ret.length = parse_quoted_string_state::error_invalid_hex_digits;
                        return ret;
                    }

                    std::stringstream ss;
                    uint32_t cp;
                    ss << std::string_view{p_head, n_digits};
                    ss >> std::hex >> cp;

                    // Start the buffer with the parsed segment prior to '\u'
                    buffer.reset();
                    if (ret.str && ret.length > 6)
                        buffer.append(ret.str, ret.length-6);

                    auto encoded = encode_utf8(cp);
                    if (encoded.empty())
                    {
                        // failed to encode it as utf-8
                        ret.str = nullptr;
                        ret.length = parse_quoted_string_state::error_invalid_hex_digits;
                        return ret;
                    }

                    buffer.append(encoded.data(), encoded.size());

                    return parse_double_quoted_string_with_buffer(buffer, p, p_end);
                }
                break;
            }
            case double_quoted_string_parse_mode_t::unspecified:
            {
                switch (*p)
                {
                    case '"':
                    {
                        // closing quote
                        ++p; // skip the quote.
                        return ret;
                    }
                    case '\\':
                    {
                        // escape character
                        mode = double_quoted_string_parse_mode_t::escaped;
                        continue;
                    }
                }

                if (0x00 <= c && c <= 0x1F)
                {
                    // This is an unescaped control character.
                    ret.has_control_character = true;
                }

                break;
            }
        }
    }

    ret.str = nullptr;
    ret.length = parse_quoted_string_state::error_no_closing_quote;
    return ret;
}

const char* parse_to_closing_double_quote(const char* p, size_t max_length)
{
    assert(*p == '"');
    const char* p_end = p + max_length;
    ++p;

    if (p == p_end)
        return nullptr;

    bool escape = false;

    for (; p != p_end; ++p)
    {
        if (escape)
        {
            char c = *p;
            escape = false;

            if (get_string_escape_char_type(c) == string_escape_char_t::invalid)
                return nullptr;
        }

        switch (*p)
        {
            case '"':
                // closing quote.
                ++p; // skip the quote.
                return p;
            case '\\':
                escape = true;
            break;
            default:
                ;
        }
    }

    return nullptr;
}

std::string_view trim(std::string_view str)
{
    const char* p = str.data();
    const char* p_end = p + str.size();

    // Find the first non-space character.
    p = std::find_if_not(p, p_end, is_blank);

    if (p == p_end)
    {
        // This string is empty.
        return std::string_view{};
    }

    // Find the last non-space character.
    auto last = std::find_if_not(std::reverse_iterator(p_end), std::reverse_iterator(p), is_blank);
    std::size_t n = std::distance(p, last.base());

    return std::string_view{p, n};
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
