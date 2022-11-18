/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_STREAM_HPP
#define INCLUDED_ORCUS_STREAM_HPP

#include "env.hpp"

#include <memory>
#include <string>

namespace orcus {

/**
 * Represents the content of a file.
 *
 * The file content is memory-mapped initially, but may later become in-memory
 * if the non-utf-8 content gets converted to utf-8.
 */
class ORCUS_PSR_DLLPUBLIC file_content
{
    struct impl;
    std::unique_ptr<impl> mp_impl;
public:
    file_content(const file_content&) = delete;
    file_content& operator= (const file_content&) = delete;

    file_content();
    file_content(file_content&& other);
    file_content(std::string_view filepath);
    ~file_content();

    /**
     * Obtain the memory address to the first character in the content buffer.
     *
     * @return pointer to the first character in the buffer.
     */
    const char* data() const;

    /**
     * Return the size of the content i.e. the number of characters in the
     * content buffer.
     *
     * @return size of the content.
     */
    size_t size() const;

    /**
     * Query whether or not the content is empty.
     *
     * @return true if the content is empty, otherwise false.
     */
    bool empty() const;

    /**
     * Swap content with another instance.
     *
     * @param other another instance to swap content with.
     */
    void swap(file_content& other);

    /**
     * Load from a new file.  This will invalidate the pointer returned from the
     * data() method prior to the call.
     *
     * @param filepath path of the file to load from.
     */
    void load(std::string_view filepath);

    /**
     * Convert a non-utf-8 stream to a utf-8 one if the source stream contains
     * a byte order mark.  If not, it does nothing.  When the conversion
     * happens, the converted content will be stored in-memory.
     */
    void convert_to_utf8();

    std::string_view str() const;
};

/**
 * Represents the content of an in-memory buffer.  Note that this class will
 * NOT own the content of the source buffer but simply will reference it,
 * except when the original buffer is a non-utf-8 stream and the caller
 * chooses to convert it to utf-8 by calling its convert_to_utf8() method.
 */
class ORCUS_PSR_DLLPUBLIC memory_content
{
    struct impl;
    std::unique_ptr<impl> mp_impl;
public:
    memory_content(const file_content&) = delete;
    memory_content& operator= (const file_content&) = delete;

    memory_content();
    memory_content(std::string_view s);
    memory_content(memory_content&& other);
    ~memory_content();

    const char* data() const;
    size_t size() const;
    bool empty() const;

    void swap(memory_content& other);

    /**
     * Convert a non-utf-8 stream to a utf-8 one if the source stream contains
     * a byte order mark.  If not, it does nothing.  When the conversion
     * happens, the converted content will be owned by the object.
     */
    void convert_to_utf8();

    std::string_view str() const;
};

struct ORCUS_PSR_DLLPUBLIC line_with_offset
{
    /** content of the entire line. */
    std::string line;
    /** 0-based line number. */
    std::size_t line_number;
    /** 0-based offset within the line. */
    std::size_t offset_on_line;

    line_with_offset(std::string _line, std::size_t _line_number, std::size_t _offset_on_line);
    line_with_offset(const line_with_offset& other);
    line_with_offset(line_with_offset&& other);
    ~line_with_offset();

    bool operator== (const line_with_offset& other) const;
    bool operator!= (const line_with_offset& other) const;
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
ORCUS_PSR_DLLPUBLIC std::string create_parse_error_output(std::string_view strm, std::ptrdiff_t offset);

/**
 * Given a string consisting of multiple lines i.e. multiple line breaks,
 * find the line that contains the specified offset position.
 *
 * @param strm string stream containing multiple lines to search.
 * @param offset offset position.
 *
 * @return structure containing information about the line containing the
 *         offset position.
 *
 * @exception std::invalid_argument if the offset value equals or exceeds the
 *               length of the string stream being searched.
 */
ORCUS_PSR_DLLPUBLIC line_with_offset locate_line_with_offset(std::string_view strm, std::ptrdiff_t offset);

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
ORCUS_PSR_DLLPUBLIC size_t locate_first_different_char(std::string_view left, std::string_view right);

/**
 * Calculate the logical length of a UTF-8 encoded string.
 *
 * @param s string to calculate the logical length of.
 * @return logical length of the UTF-8 encoded string.
 */
ORCUS_PSR_DLLPUBLIC std::size_t calc_logical_string_length(std::string_view s);

} // namespace orcus

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
