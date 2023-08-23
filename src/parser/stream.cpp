/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/stream.hpp>
#include <orcus/exception.hpp>

#include "utf8.hpp"

#include <sstream>
#include <fstream>
#include <tuple>
#include <cassert>
#include <algorithm>
#include <locale>
#include <codecvt>
#include <iostream>

#include "filesystem_env.hpp"

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

namespace bip = boost::interprocess;

namespace orcus {

namespace {

enum class unicode_t
{
    unknown,
    utf16_be,
    utf16_le
};

unicode_t check_unicode_type(const char* p, size_t n)
{
    if (n > 2)
    {
        if (p[0] == '\xFE' && p[1] == '\xFF')
            return unicode_t::utf16_be;

        if (p[0] == '\xFF' && p[1] == '\xFE')
            return unicode_t::utf16_le;
    }

    return unicode_t::unknown;
}

std::string convert_utf16_to_utf8(const char* p, size_t n, unicode_t ut)
{
    assert(ut == unicode_t::utf16_be || ut == unicode_t::utf16_le);

    if (n & 0x01)
        throw std::invalid_argument("size of a UTF-16 string must be divisible by 2.");

    p += 2; // skip the BOM.

    size_t n_buf = n / 2u - 1;
    std::u16string buf(n_buf, 0);

    switch (ut)
    {
        case unicode_t::utf16_be:
        {
            for (size_t i = 0; i < n_buf; ++i)
            {
                size_t offset = i * 2;
                buf[i] = static_cast<char16_t>(p[offset+1] | p[offset] << 8);
            }
            break;
        }
        case unicode_t::utf16_le:
        {
            for (size_t i = 0; i < n_buf; ++i)
            {
                size_t offset = i * 2;
                buf[i] = static_cast<char16_t>(p[offset] | p[offset+1]);
            }
            break;
        }
        default:
            ;
    }

#if defined(_MSC_VER)
    // char16_t does not work with MSVC just yet. This is a workaround. c.f.
    // https://stackoverflow.com/questions/32055357/visual-studio-c-2015-stdcodecvt-with-char16-t-or-char32-t
    const int16_t* pi16 = reinterpret_cast<const int16_t*>(buf.data());
    const int16_t* pi16_end = pi16 + buf.size();
    std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> conversion;
    return conversion.to_bytes(pi16, pi16_end);
#else
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conversion;
    return conversion.to_bytes(buf);
#endif
}

std::tuple<std::string_view, size_t, size_t> find_line_with_offset(std::string_view strm, std::ptrdiff_t offset)
{
    const char* p0 = strm.data();
    const char* p_end = p0 + strm.size();
    const char* p_offset = p0 + offset;

    if (p_offset >= p_end)
    {
        std::ostringstream os;
        os << "offset value of " << offset << " is out-of-bound for a stream of length " << strm.size();
        throw std::invalid_argument(os.str());
    }

    // Determine the line number.
    std::size_t line_num = 0;
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
    std::size_t offset_on_line = std::distance(p_line_start, p_offset);
    std::string_view line(p_line_start, p_line_end - p_line_start);

    return std::make_tuple(line, line_num, offset_on_line);
}

} // anonymous namespace

struct file_content::impl
{
    boost::uintmax_t content_size;
    bip::file_mapping mapped_file;
    bip::mapped_region mapped_region;

    std::string buffer; // its own buffer in case of stream conversion.

    const char* content;

    impl() : content_size(0), content(nullptr) {}

    impl(std::string_view filepath) :
        content_size(fs::file_size(std::string{filepath}.c_str())),
        mapped_file(std::string{filepath}.c_str(), bip::read_only),
        mapped_region(mapped_file, bip::read_only, 0, content_size),
        content(nullptr)
    {
        content = static_cast<const char*>(mapped_region.get_address());
    }
};

file_content::file_content() :
    mp_impl(std::make_unique<impl>()) {}

file_content::file_content(file_content&& other) = default;

file_content::file_content(std::string_view filepath) :
    mp_impl(std::make_unique<impl>(filepath)) {}

file_content::~file_content() = default;

const char* file_content::data() const
{
    return mp_impl->content;
}

size_t file_content::size() const
{
    return mp_impl->content_size;
}

bool file_content::empty() const
{
    return mp_impl->content_size == 0;
}

void file_content::swap(file_content& other)
{
    std::swap(mp_impl, other.mp_impl);
}

void file_content::load(std::string_view filepath)
{
    file_content tmp(filepath);
    swap(tmp);
}

void file_content::convert_to_utf8()
{
    unicode_t ut = check_unicode_type(mp_impl->content, mp_impl->content_size);

    switch (ut)
    {
        case unicode_t::utf16_be:
        case unicode_t::utf16_le:
        {
            // Convert to utf-8 stream, and reset the content pointer and size.
            mp_impl->buffer = convert_utf16_to_utf8(mp_impl->content, mp_impl->content_size, ut);
            mp_impl->content = mp_impl->buffer.data();
            mp_impl->content_size = mp_impl->buffer.size();
            break;
        }
        default:
            ;
    }
}

std::string_view file_content::str() const
{
    return std::string_view(mp_impl->content, mp_impl->content_size);
}

struct memory_content::impl
{
    std::string_view content;
    std::string buffer; // its own buffer in case of stream conversion.

    impl() {}
    impl(std::string_view s) : content(s) {}
};

memory_content::memory_content() : mp_impl(std::make_unique<impl>()) {}

memory_content::memory_content(std::string_view s) :
    mp_impl(std::make_unique<impl>(s)) {}

memory_content::memory_content(memory_content&& other) = default;
memory_content::~memory_content() = default;

const char* memory_content::data() const
{
    return mp_impl->content.data();
}

size_t memory_content::size() const
{
    return mp_impl->content.size();
}

bool memory_content::empty() const
{
    return mp_impl->content.empty();
}

void memory_content::swap(memory_content& other)
{
    std::swap(mp_impl, other.mp_impl);
}

void memory_content::convert_to_utf8()
{
    unicode_t ut = check_unicode_type(mp_impl->content.data(), mp_impl->content.size());

    switch (ut)
    {
        case unicode_t::utf16_be:
        case unicode_t::utf16_le:
        {
            // Convert to utf-8 stream, and reset the content pointer and size.
            mp_impl->buffer = convert_utf16_to_utf8(mp_impl->content.data(), mp_impl->content.size(), ut);
            mp_impl->content = mp_impl->buffer;
            break;
        }
        default:
            ;
    }
}

std::string_view memory_content::str() const
{
    return mp_impl->content;
}

line_with_offset::line_with_offset(std::string _line, std::size_t _line_number, std::size_t _offset_on_line) :
    line(std::move(_line)),
    line_number(_line_number),
    offset_on_line(_offset_on_line)
{}

line_with_offset::line_with_offset(const line_with_offset& other) = default;
line_with_offset::line_with_offset(line_with_offset&& other) = default;
line_with_offset::~line_with_offset() = default;

bool line_with_offset::operator== (const line_with_offset& other) const
{
    return line == other.line && line_number == other.line_number && offset_on_line == other.offset_on_line;
}

bool line_with_offset::operator!= (const line_with_offset& other) const
{
    return !operator==(other);
}

std::string create_parse_error_output(std::string_view strm, std::ptrdiff_t offset)
{
    if (strm.empty() || offset < 0)
        return std::string();

    const size_t max_line_length = 60;
    offset = std::min<std::ptrdiff_t>(strm.size() - 1, offset);

    auto line_info = find_line_with_offset(strm, offset);
    std::string_view line = std::get<0>(line_info);
    size_t line_num = std::get<1>(line_info);
    size_t offset_on_line = std::get<2>(line_info);

    if (offset_on_line < 30)
    {
        std::ostringstream os;
        os << (line_num+1) << ":" << (offset_on_line+1) << ": ";
        size_t line_num_width = os.str().size();

        // Truncate line if it's too long.
        if (line.size() > max_line_length)
            line = std::string_view(line.data(), max_line_length);

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

    line = std::string_view(line.data()+line_start, line_length);

    std::ostringstream os;
    os << line_num << ":" << (line_start+1) << ": ";
    size_t line_num_width = os.str().size();

    os << line << std::endl;

    for (size_t i = 0; i < (fixed_offset+line_num_width); ++i)
        os << ' ';
    os << '^';

    return os.str();
}

line_with_offset locate_line_with_offset(std::string_view strm, std::ptrdiff_t offset)
{
    auto line_info = find_line_with_offset(strm, offset);
    std::string_view line = std::get<0>(line_info);
    size_t line_num = std::get<1>(line_info);
    size_t offset_on_line = std::get<2>(line_info);

    return line_with_offset(std::string{line}, line_num, offset_on_line);
}

size_t locate_first_different_char(std::string_view left, std::string_view right)
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

std::size_t calc_logical_string_length(std::string_view s)
{
    std::size_t length = 0;

    const char* p = s.data();
    const char* p_end = p + s.size();

    while (p < p_end)
    {
        ++length;

        auto n_bytes = calc_utf8_byte_length(*p);
        if (!n_bytes || n_bytes > 4)
        {
            std::ostringstream os;
            os << "'" << s << "' contains invalid character at position " << std::distance(s.data(), p);
            throw std::invalid_argument(os.str());
        }

        p += n_bytes;
    }

    if (p != p_end)
    {
        std::ostringstream os;
        os << "last character of '" << s << "' ended prematurely";
        throw std::invalid_argument(os.str());
    }

    return length;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
