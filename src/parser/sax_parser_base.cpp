/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/sax_parser_base.hpp"

#include "utf8.hpp"

#include <cstring>
#include <vector>
#include <memory>

#ifdef __ORCUS_CPU_FEATURES
#include <immintrin.h>
#endif

namespace orcus { namespace sax {

char decode_xml_encoded_char(const char* p, size_t n)
{
    if (n == 2)
    {
        if (!std::strncmp(p, "lt", n))
            return '<';
        else if (!std::strncmp(p, "gt", n))
            return '>';
        else
            return '\0';
    }
    else if (n == 3)
    {
        if (!std::strncmp(p, "amp", n))
            return '&';
        else
            return '\0';
    }
    else if (n == 4)
    {
        if (!std::strncmp(p, "apos", n))
            return '\'';
        else if (!std::strncmp(p, "quot", 4))
            return '"';
        else
            return '\0';
    }

    return '\0';
}

std::string decode_xml_unicode_char(const char* p, size_t n)
{
    if (*p == '#' && n >= 2)
    {
        uint32_t point = 0;
        if (p[1] == 'x')
        {
            if (n == 2)
                throw orcus::xml_structure_error(
                    "invalid number of characters for hexadecimal unicode reference");

            point = std::stoi(std::string(p + 2, n - 2), nullptr, 16);
        }
        else
            point = std::stoi(std::string(p + 1, n - 1), nullptr, 10);

        if (point < 0x80)
        {
            // is it really necessary to do the bit manipulation here?
            std::string s(1, static_cast<char>(point & 0x7F));
            return s;
        }
        else if (point < 0x0800)
        {
            std::string s(1, static_cast<char>((point >> 6 & 0x1F) | 0xC0));
            s += static_cast<char>((point & 0x3F) | 0x80);
            return s;
        }
        else if (point < 0x010000)
        {
            std::string s(1, static_cast<char>((point >> 12 & 0x0F) | 0xE0));
            s += static_cast<char>((point >> 6 & 0x3F) | 0x80);
            s += static_cast<char>((point & 0x3F) | 0x80);
            return s;
        }
        else if (point < 0x110000)
        {
            std::string s(1, static_cast<char>((point >> 18 & 0x07) | 0xF0));
            s += static_cast<char>((point >> 12 & 0x3F) | 0x80);
            s += static_cast<char>((point >> 6 & 0x3F) | 0x80);
            s += static_cast<char>((point & 0x3F) | 0x80);
            return s;
        }
        else
        {
            // should not happen as that is not represented by utf-8
            assert(false);
        }
    }

    return std::string();
}

struct parser_base::impl
{
    std::vector<std::unique_ptr<cell_buffer>> m_cell_buffers;
};

parser_base::parser_base(const char* content, size_t size) :
    ::orcus::parser_base(content, size),
    mp_impl(std::make_unique<impl>()),
    m_nest_level(0),
    m_buffer_pos(0),
    m_root_elem_open(true)
{
    mp_impl->m_cell_buffers.push_back(std::make_unique<cell_buffer>());
}

parser_base::~parser_base() {}

void parser_base::inc_buffer_pos()
{
    ++m_buffer_pos;
    if (m_buffer_pos == mp_impl->m_cell_buffers.size())
        mp_impl->m_cell_buffers.push_back(std::make_unique<cell_buffer>());
}

cell_buffer& parser_base::get_cell_buffer()
{
    return *mp_impl->m_cell_buffers[m_buffer_pos];
}

void parser_base::comment()
{
    // Parse until we reach '-->'.
    size_t len = available_size();
    assert(len > 3);
    char c = cur_char();
    size_t i = 0;
    bool hyphen = false;
    for (; i < len; ++i, c = next_and_char())
    {
        if (c == '-')
        {
            if (!hyphen)
                // first hyphen.
                hyphen = true;
            else
                // second hyphen.
                break;
        }
        else
            hyphen = false;
    }

    if (len - i < 2 || next_and_char() != '>')
        throw malformed_xml_error(
            "'--' should not occur in comment other than in the closing tag.", offset());

    next();
}

void parser_base::expects_next(const char* p, size_t n)
{
    if (available_size() < n+1)
        throw malformed_xml_error(
            "not enough stream left to check for an expected string segment.", offset());

    const char* p0 = p;
    const char* p_end = p + n;
    char c = next_and_char();
    for (; p != p_end; ++p, c = next_and_char())
    {
        if (c == *p)
            continue;

        std::ostringstream os;
        os << "'" << std::string(p0, n) << "' was expected, but not found.";
        throw malformed_xml_error(os.str(), offset());
    }
}

void parser_base::parse_encoded_char(cell_buffer& buf)
{
    assert(cur_char() == '&');
    next();
    const char* p0 = mp_char;
    for (; has_char(); next())
    {
        if (cur_char() != ';')
            continue;

        size_t n = mp_char - p0;
        if (!n)
            throw malformed_xml_error("empty encoded character.", offset());

#if ORCUS_DEBUG_SAX_PARSER
        cout << "sax_parser::parse_encoded_char: raw='" << std::string(p0, n) << "'" << endl;
#endif

        char c = decode_xml_encoded_char(p0, n);
        if (c)
            buf.append(&c, 1);
        else
        {
            std::string utf8 = decode_xml_unicode_char(p0, n);

            if (!utf8.empty())
            {
                buf.append(utf8.data(), utf8.size());
                c = '1'; // just to avoid hitting the !c case below
            }
        }

        // Move to the character past ';' before returning to the parent call.
        next();

        if (!c)
        {
#if ORCUS_DEBUG_SAX_PARSER
            cout << "sax_parser::parse_encoded_char: not a known encoding name. Use the original." << endl;
#endif
            // Unexpected encoding name. Use the original text.
            buf.append(p0, mp_char-p0);
        }

        return;
    }

    throw malformed_xml_error(
        "error parsing encoded character: terminating character is not found.", offset());
}

void parser_base::value_with_encoded_char(cell_buffer& buf, std::string_view& str, char quote_char)
{
    assert(cur_char() == '&');
    parse_encoded_char(buf);

    const char* p0 = mp_char;

    while (has_char())
    {
        if (cur_char() == '&')
        {
            if (mp_char > p0)
                buf.append(p0, mp_char-p0);

            parse_encoded_char(buf);
            p0 = mp_char;
        }

        if (cur_char() == quote_char)
            break;

        if (cur_char() != '&')
            next();
    }

    if (mp_char > p0)
        buf.append(p0, mp_char-p0);

    if (!buf.empty())
        str = buf.str();

    // Skip the closing quote.
    assert(!has_char() || cur_char() == quote_char);
    if (has_char())
       next();
}

bool parser_base::value(std::string_view& str, bool decode)
{
    char c = cur_char_checked();
    if (c != '"' && c != '\'')
        throw malformed_xml_error("value must be quoted", offset());

    char quote_char = c;

    c = next_char_checked();

    const char* p0 = mp_char;
    for (; c != quote_char; c = next_char_checked())
    {
        if (decode && c == '&')
        {
            // This value contains one or more encoded characters.
            cell_buffer& buf = get_cell_buffer();
            buf.reset();
            buf.append(p0, mp_char-p0);
            value_with_encoded_char(buf, str, quote_char);
            return true;
        }
    }

    str = std::string_view(p0, mp_char-p0);

    // Skip the closing quote.
    next();

    return false;
}

void parser_base::name(std::string_view& str)
{
    const char* p0 = mp_char;
    mp_char = parse_utf8_xml_name_start_char(mp_char, mp_end);
    if (mp_char == p0)
    {
        ::std::ostringstream os;
        os << "name must begin with an alphabet, but got this instead '" << cur_char() << "'";
        throw malformed_xml_error(os.str(), offset());
    }

#if defined(__ORCUS_CPU_FEATURES) && defined(__SSE4_2__)

    const __m128i match = _mm_loadu_si128((const __m128i*)"azAZ09--__..");
    const int mode = _SIDD_LEAST_SIGNIFICANT | _SIDD_CMP_RANGES | _SIDD_UBYTE_OPS | _SIDD_NEGATIVE_POLARITY;

    size_t n_total = available_size();

    while (n_total)
    {
        __m128i char_block = _mm_loadu_si128((const __m128i*)mp_char);

        int n = std::min<size_t>(16u, n_total);
        int r = _mm_cmpestri(match, 12, char_block, n, mode);
        mp_char += r; // Move the current char position.
        n_total -= r;

        if (r < 16 && n_total)
        {
            // There is a character that does not match the SSE-based ASCII-only check.
            // It may either by an ascii character that is not allowed, in which case stop,
            // or it may possibly be an allowed utf-8 character, in which case move over it
            // using the slow function.

            const char* p = parse_utf8_xml_name_char(mp_char, mp_end);
            if (p == mp_char)
                break;

            n_total -= p - mp_char;
            mp_char = p;
        }

    }
    cur_char_checked(); // check end of xml stream

#else
    for(;;)
    {
        cur_char_checked(); // check end of xml stream
        const char* p = parse_utf8_xml_name_char(mp_char, mp_end);

        if (p == mp_char)
            break;

        mp_char = p;
    }
#endif

    str = std::string_view(p0, mp_char-p0);
}

void parser_base::element_name(parser_element& elem, std::ptrdiff_t begin_pos)
{
    elem.begin_pos = begin_pos;
    name(elem.name);
    if (cur_char() == ':')
    {
        elem.ns = elem.name;
        next_check();
        name(elem.name);
    }
}

void parser_base::attribute_name(std::string_view& attr_ns, std::string_view& attr_name)
{
    name(attr_name);
    if (cur_char() == ':')
    {
        // Attribute name is namespaced.
        attr_ns = attr_name;
        next_check();
        name(attr_name);
    }
}

void parser_base::characters_with_encoded_char(cell_buffer& buf)
{
    assert(cur_char() == '&');
    parse_encoded_char(buf);

    const char* p0 = mp_char;

    while (has_char())
    {
        if (cur_char() == '&')
        {
            if (mp_char > p0)
                buf.append(p0, mp_char-p0);

            parse_encoded_char(buf);
            p0 = mp_char;
        }

        if (cur_char() == '<')
            break;

        if (cur_char() != '&')
            next();
    }

    if (mp_char > p0)
        buf.append(p0, mp_char-p0);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
