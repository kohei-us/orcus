/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_STREAM_HPP
#define INCLUDED_ORCUS_STREAM_HPP

#include "env.hpp"
#include "orcus/pstring.hpp"

#include <memory>

namespace orcus {

/**
 * Represents the content of a file.  The file content may be either
 * in-memory, or memory-mapped; it is initially memory-mapped, but it may
 * become in-memory when converted to a different encoding.
 */
class ORCUS_PSR_DLLPUBLIC file_content
{
    struct impl;
    std::unique_ptr<impl> mp_impl;
public:
    file_content();
    file_content(const char* filepath);
    ~file_content();

    const char* data() const;
    size_t size() const;
    bool empty() const;

    void swap(file_content& other);

    /**
     * Load from a new file.  This will invalidate the pointer returned from
     * the {@link data()} method prior to the call.
     *
     *
     * @param filepath path of the file to load from.
     */
    void load(const char* filepath);

    /**
     * Convert a non-utf-8 stream to a utf-8 one if the source stream contains
     * a byte order mark.  If not, it does nothing.  When the conversion
     * happens, the converted content will be stored in-memory.
     */
    void convert_to_utf8();

    pstring str() const;
};

/**
 * Generate a sensible error output for parse error including the line where
 * the error occurred and the offset of the error position on that line.
 *
 * @param strm entire character stream where the error occurred.
 * @param offset offset of the error position within the stream.
 *
 * @return string formatted to be usable as an error message for stdout.
 */
ORCUS_PSR_DLLPUBLIC std::string create_parse_error_output(
    const pstring& strm, std::ptrdiff_t offset);

/**
 * Given two strings, locate the position of the first character that is
 * different between the two strings.  Note that if one of the strings is
 * empty (or both of them are empty), it returns 0.
 *
 * @param left one of the strings to compare.
 * @param right one of the strings to compare.
 *
 * @return position of the first character that is different between the two
 *         compared strings.
 */
ORCUS_PSR_DLLPUBLIC size_t locate_first_different_char(
    const pstring& left, const pstring& right);

} // namespace orcus

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
