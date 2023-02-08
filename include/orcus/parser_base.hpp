/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_PARSER_BASE_HPP
#define INCLUDED_ORCUS_PARSER_BASE_HPP

#include "env.hpp"
#include "exception.hpp"

#include <string>
#include <cstdlib>
#include <cstddef>
#include <cassert>
#include <functional>

namespace orcus {

class ORCUS_PSR_DLLPUBLIC parser_base
{
protected:
    using numeric_parser_type = std::function<const char*(const char*, const char*, double&)>;

    const char* const mp_begin;
    const char* mp_char;
    const char* mp_end;

private:
    numeric_parser_type m_func_parse_numeric;

protected:
    parser_base(const char* p, size_t n);

    void set_numeric_parser(const numeric_parser_type& func)
    {
        m_func_parse_numeric = func;
    }

    bool has_char() const
    {
        assert(mp_char <= mp_end);
        return mp_char != mp_end;
    }

    bool has_next() const
    {
        assert((mp_char+1) <= mp_end);
        return (mp_char+1) != mp_end;
    }

    void next(size_t inc=1) { mp_char += inc; }

    void prev(size_t dec=1);

    char cur_char() const { return *mp_char; }

    /**
     * Peek a character at specified offset from the current position without
     * advancing the current position.
     *
     * @note The caller <strong>must</strong> ensure that the specified offset
     *       position is a valid position.  This method does not check its
     *       validity.
     *
     * @param offset offset from the current position to peek at.
     *
     * @return character at a specified offset position from the current
     *         position.
     */
    char peek_char(std::size_t offset=1) const;

    /**
     * Peek a segment of contiguous characters of a specified length starting
     * from the current position.
     *
     * @note The caller <strong>must</strong> ensure that the specified
     *       substring segment is entirely valid.  This method does not check
     *       its validity.
     *
     * @param length length of the segment to peek.
     *
     * @return segment of contiguous characters.
     */
    std::string_view peek_chars(std::size_t length) const;

    /**
     * Skip an optional byte order mark at the current position of the stream.
     *
     * Currently we only check for UTF-8 BOM.
     */
    void skip_bom();

    void skip(std::string_view chars_to_skip);

    /**
     * Skip all characters that are 0-32 in ASCII range
     */
    void skip_space_and_control();

    /**
     * Parse and check next characters to see if it matches specified
     * character sequence.
     *
     * @param expected sequence of characters to match against.
     *
     * @return true if it matches specified character sequence, false
     *         otherwise.
     */
    bool parse_expected(std::string_view expected);

    /**
     * Try to parse the next characters as double, or return NaN in case of
     * failure.
     *
     * @return double value on success, or NaN on failure.
     */
    double parse_double();

    /**
     * Determine the number of characters remaining <strong>after</strong> the
     * current character.  For instance, if the current character is on the
     * last character in the stream, this method will return 0, whereas if
     * it's on the first character, it will return the total length - 1.
     *
     * @return number of characters remaining after the current character.
     */
    size_t remaining_size() const;

    /**
     * Determine the number of characters available from the current character
     * to the end of the buffer.  The current character is included.
     *
     * @return number of characters available including the current character.
     */
    size_t available_size() const
    {
        return std::distance(mp_char, mp_end);
    }

    /**
     * Return the current offset from the beginning of the character stream.
     *
     * @return current offset from the beginning of the character stream.
     */
    std::ptrdiff_t offset() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
