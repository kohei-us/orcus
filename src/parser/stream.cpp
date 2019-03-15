/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/stream.hpp"
#include "orcus/exception.hpp"
#include "orcus/pstring.hpp"

#include <sstream>
#include <fstream>
#include <tuple>
#include <cassert>
#include <algorithm>
#include <locale>
#include <codecvt>

namespace orcus {

namespace {

std::tuple<pstring, size_t, size_t> find_line_with_offset(
    const pstring& strm, std::ptrdiff_t offset)
{
    const char* p0 = strm.get();
    const char* p_end = p0 + strm.size();
    const char* p_offset = p0 + offset;

    // Determine the line number.
    size_t line_num = 1;
    for (const char* p = p0; p != p_offset; ++p)
    {
        if (*p == '\n')
            ++line_num;
    }

    // Determine the beginning of the line.
    const char* p_line_start = p_offset;

    // if the error points at the new line character
    // we have most likely an unterminated quote.
    // Report the line with the actual error.
    if (*p_offset == '\n' && offset > 0)
        --p_line_start;

    for (; p0 <= p_line_start; --p_line_start)
    {
        if (*p_line_start == '\n')
            break;
    }

    ++p_line_start;
    assert(p0 <= p_line_start);

    // Determine the end of the line.
    const char* p_line_end = p_offset;
    for (; p_line_end < p_end; ++p_line_end)
    {
        if (*p_line_end == '\n')
            // one character after the last character of the line.
            break;
    }

    assert(p_line_start <= p_offset);
    size_t offset_on_line = std::distance(p_line_start, p_offset);
    pstring line(p_line_start, p_line_end - p_line_start);

    return std::make_tuple(line, line_num, offset_on_line);
}

}

std::string load_file_content(const char* filepath)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file)
    {
        // failed to open the specified file.
        std::ostringstream os;
        os << "failed to load " << filepath;
        throw general_error(os.str());
    }

    std::ostringstream os;
    os << file.rdbuf();
    file.close();

    return os.str();
}

std::string create_parse_error_output(const pstring& strm, std::ptrdiff_t offset)
{
    if (offset < 0)
        return std::string();

    const size_t max_line_length = 60;

    auto line_info = find_line_with_offset(strm, offset);
    pstring line = std::get<0>(line_info);
    size_t line_num = std::get<1>(line_info);
    size_t offset_on_line = std::get<2>(line_info);

    if (offset_on_line < 30)
    {
        std::ostringstream os;
        os << line_num << ":" << (offset_on_line+1) << ": ";
        size_t line_num_width = os.str().size();

        // Truncate line if it's too long.
        if (line.size() > max_line_length)
            line.resize(max_line_length);

        os << line << std::endl;

        for (size_t i = 0; i < (offset_on_line+line_num_width); ++i)
            os << ' ';
        os << '^';
        return os.str();
    }

    // The error line is too long.  Only show a segment of the line where the
    // error occurred.

    const size_t fixed_offset = 20;

    size_t line_start = offset_on_line - fixed_offset;
    size_t line_end = line_start + max_line_length;
    if (line_end > line.size())
        line_end = line.size();

    size_t line_length = line_end - line_start;

    line = pstring(line.get()+line_start, line_length);

    std::ostringstream os;
    os << line_num << ":" << (line_start+1) << ": ";
    size_t line_num_width = os.str().size();

    os << line << std::endl;

    for (size_t i = 0; i < (fixed_offset+line_num_width); ++i)
        os << ' ';
    os << '^';

    return os.str();
}

size_t locate_first_different_char(const pstring& left, const pstring& right)
{
    if (left.empty() || right.empty())
        // If one of them is empty, then the first characters are considered
        // different.
        return 0;

    size_t n = std::min(left.size(), right.size());
    const char* p1 = left.data();
    const char* p2 = right.data();
    const char* p1_end = p1 + n;

    for (; p1 != p1_end; ++p1, ++p2)
    {
        if (*p1 != *p2)
            return std::distance(left.data(), p1);
    }

    return n;
}

std::string convert_to_utf8(std::string src)
{
    size_t n_src = src.size();

    if (n_src > 2 && src[0] == '\xFE' && src[1] == '\xFF')
    {
        if (n_src & 0x01)
            throw std::invalid_argument("size of a UTF-16 string must be divisible by 2.");

        // big-endian utf-16
        const char* p = src.data();
        p += 2; // skip the BOM.

        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conversion;
        size_t n_buf = n_src / 2u - 1;
        std::u16string buf(n_buf, 0);

        for (size_t i = 0; i < n_buf; ++i)
        {
            size_t offset = i * 2;
            buf[i] = static_cast<char16_t>(p[offset+1] | p[offset] << 8);
        }

        return conversion.to_bytes(buf);
    }

    // No conversion needed.
    return src;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
