/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/sax_parser_base.hpp"
#include "orcus/global.hpp"

#include <cstring>
#include <vector>
#include <memory>

#ifdef __ORCUS_CPU_FEATURES
#include <immintrin.h>
#endif

namespace orcus { namespace sax {

malformed_xml_error::malformed_xml_error(const std::string& msg, std::ptrdiff_t offset) :
    ::orcus::parse_error("malformed_xml_error", msg, offset) {}

malformed_xml_error::~malformed_xml_error() throw() {}

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

parser_base::parser_base(const char* content, size_t size, bool transient_stream) :
    ::orcus::parser_base(content, size, transient_stream),
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
    size_t len = remains();
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

void parser_base::skip_bom()
{
    if (remains() < 4)
        // Stream too short to have a byte order mark.
        return;

    if (is_blank(cur_char()))
        // Allow leading whitespace in the XML stream.
        // TODO : Make this configurable since strictly speaking such an XML
        // sttream is invalid.
        return;

    // 0xef 0xbb 0 xbf is the UTF-8 byte order mark
    unsigned char c = static_cast<unsigned char>(cur_char());
    if (c != '<')
    {
        if (c != 0xef || static_cast<unsigned char>(next_and_char()) != 0xbb ||
            static_cast<unsigned char>(next_and_char()) != 0xbf || next_and_char() != '<')
            throw malformed_xml_error(
                "unsupported encoding. only 8 bit encodings are supported", offset());
    }
}

void parser_base::expects_next(const char* p, size_t n)
{
    if (remains() < n+1)
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

void parser_base::value_with_encoded_char(cell_buffer& buf, pstring& str, char quote_char)
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
        str = pstring(buf.get(), buf.size());

    // Skip the closing quote.
    assert(!has_char() || cur_char() == quote_char);
    next();
}

bool parser_base::value(pstring& str, bool decode)
{
    char c = cur_char();
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

    str = pstring(p0, mp_char-p0);

    // Skip the closing quote.
    next();

    return transient_stream();
}

// https://www.w3.org/TR/2006/REC-xml11-20060816/#NT-NameStartChar
// Return length of the character in bytes, otherwise 0.
template< bool only_start_name >
static
int is_name_char_helper(const char* mp_char, const char* mp_end)
{
    const unsigned char first = mp_char[0];
    // Note that ':' technically is an allowed name character, but it is handled separately
    // e.g. in element_name(), so here pretend it isn't.
    if (/*first == ':' ||*/ first == '_' || (first >= 'A' && first <= 'Z') || (first >= 'a' && first <= 'z'))
        return 1;
    if (!only_start_name && (first == '-' || first == '.' || (first >= '0' && first <= '9')))
        return 1;

    if (first < 0x7f) // other ascii characters are not allowed
        return 0;
    if (mp_end < mp_char + 1)
        return 0;
    const unsigned char second = mp_char[1];

    // 0xb7 = 0xc2 0xb7 utf-8
    if (!only_start_name && first == 0xc2 && second == 0xb7)
        return 2;

    // [#xC0-#xD6] | [#xD8-#xF6] | [#xF8-#x2FF]
    // 0xc0 = 0xc3 0x80 utf-8
    if (first < 0xc3)
        return 0;
    // xd7 = 0xc3 0x97 utf-8, 0xf7 = 0xc3 0xb7 utf-8
    if (first == 0xc3)
        return second >= 0x80 && second <= 0xff && second != 0x97 && second != 0xb7 ? 2 : 0;
    // 0x2ff = 0xcb 0xbf utf-8, 0x300 = 0xcc 0x80 utf-8
    if (first >= 0xc4 && first <= 0xcb)
        return 2;

    // [#x0300-#x036F]
    // 0x0300 = 0xcc 0x80 utf-8, 0x36f = 0xcd 0xaf utf-8
    if (!only_start_name && first == 0xcc)
        return 2;
    if (!only_start_name && first == 0xcd && second <= 0xaf)
        return 2;

    // [#x370-#x37D] | [#x37F-#x1FFF]
    // 0x370 = 0xcd 0xb0 utf-8, 0x37e = 0xcd 0xbe
    if (first < 0xcd)
        return 0;
    if (first == 0xcd)
        return second >= 0xb0 && second != 0xbe ? 2 : 0;
    // 0x07ff = 0xdf 0xbf utf-8 (the last 2-byte utf-8)
    if (first <= 0xdf)
        return 2;

    if (first < 0xe0)
        return 0;
    if (mp_end < mp_char + 2)
        return 0;
    const unsigned char third = mp_char[2];

    // 0x0800 = 0xe0 0xa0 0x80 utf-8, 0x1fff = 0xe1 0xbf 0xbf utf-8, 0x2000 = 0xe2 0x80 0x80
    if (first == 0xe0 || first == 0xe1)
        return 3;

    // [#x200C-#x200D]
    // 0x200c = 0xe2 0x80 0x8c utf-8, 0x200d = 0xe2 0x80 0x8d utf-8
    if (first < 0xe2)
        return 0;
    if (first == 0xe2 && second == 0x80 && (third == 0x8c || third == 0x8d))
        return 3;

    // [#x203F-#x2040]
    // 0x203f = 0xe2 0x80 0xbf utf-8, 0x2040 = 0xe2 0x81 0x80 utf-8
    if (!only_start_name && first == 0xe2 && second == 0x80 && third == 0xbf)
        return 3;
    if (!only_start_name && first == 0xe2 && second == 0x81 && third == 0x80)
        return 3;

    // [#x2070-#x218F]
    // 0x2070 = 0xe2 0x81 0xb0 utf-8, 0x218f = 0xe2 0x86 0x8f utf-8
    if (first == 0xe2)
    {
        if (second < 0x81)
            return 0;
        if (second >= 0x81 && second < 0x86)
            return 3;
        if (second == 0x86 && third <= 0x8f)
            return 3;
    }

    // [#x2C00-#x2FEF]
    // 0x2c00 = 0xe2 0xb0 0x80 utf-8, 0x2fef = 0xe2 0xbf 0xaf utf-8
    if (first == 0xe2)
    {
        if (second < 0xb0)
            return 0;
        if (second < 0xbf)
            return 3;
        if (second == 0xbf && third <= 0xaf)
            return 3;
    }

    // [#x3001-#xD7FF]
    // 0x3001 = 0xe3 0x80 0x81 utf-8, 0xd7ff = 0xed 0x9f 0xbf utf-8, 0xd800 = 0xed 0xa0 0x80 utf-8
    if (first < 0xe3)
        return 0;
    if (first < 0xed)
        return 3;
    if (first == 0xed && second <= 0x9f)
        return 3;

    // [#xF900-#xFDCF]
    // 0xf900 = 0xef 0xa4 0x80 utf-8, 0xfdcf = 0xef 0xb7 0x8f utf-8
    if (first == 0xef)
    {
        if (second < 0xa4)
            return 0;
        if (second < 0xb7)
            return 3;
        if (second == 0xb7 && third <= 0x8f)
            return 3;
    }

    // [#xFDF0-#xFFFD]
    // 0xfdf0 = 0xef 0xb7 0xb0 utf-8, 0xfffd = 0xef 0xbf 0xbd utf-8
    if (first == 0xef)
    {
        assert(second >= 0xb7);
        if (second == 0xb7 && third < 0xb0)
            return 0;
        if (second < 0xbe)
            return 3;
        if (second == 0xbf && third <= 0xbd)
            return 3;
    }

    if (first < 0xf0)
        return 0;
    if (mp_end < mp_char + 3)
        return 0;
    // const unsigned char fourth = mp_char[3];

    // [#x10000-#xEFFFF]
    // 0x10000 = 0xf0 0x90 0x80 0x80 utf-8, 0xeffff = 0xf3 0xaf 0xbf 0xbf utf-8,
    // 0xf0000 = 0xf3 0xb0 0x80 0x80 utf-8
    if (first >= 0xf0 && first < 0xf2)
        return 4;
    if (first == 0xf3 && second < 0xb0)
        return 4;

    return 0;
}

int parser_base::is_name_char()
{
    return is_name_char_helper<false>(mp_char, mp_end);
}

int parser_base::is_name_start_char()
{
    return is_name_char_helper<true>(mp_char, mp_end);
}

void parser_base::name(pstring& str)
{
    const char* p0 = mp_char;
    int skip = is_name_start_char();
    if (skip == 0)
    {
        ::std::ostringstream os;
        os << "name must begin with an alphabet, but got this instead '" << cur_char() << "'";
        throw malformed_xml_error(os.str(), offset());
    }
    next(skip);

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
            skip = is_name_char();
            if(skip == 0)
                break;
            next(skip);
            n_total -= skip;
        }

    }
    cur_char_checked(); // check end of xml stream

#else
    for(;;)
    {
        cur_char_checked(); // check end of xml stream
        skip = is_name_char();
        if(skip == 0)
            break;
        next(skip);
    }
#endif

    str = pstring(p0, mp_char-p0);
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

void parser_base::attribute_name(pstring& attr_ns, pstring& attr_name)
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
