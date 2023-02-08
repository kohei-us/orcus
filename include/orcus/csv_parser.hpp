/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_CSV_PARSER_HPP
#define ORCUS_CSV_PARSER_HPP

#include "csv_parser_base.hpp"

namespace orcus {

class csv_handler
{
public:
    /**
     * Called when the parser starts parsing a stream.
     */
    void begin_parse() {}

    /**
     * Called when the parser finishes parsing a stream.
     */
    void end_parse() {}

    /**
     * Called at the beginning of every row.
     */
    void begin_row() {}

    /**
     * Called at the end of every row.
     */
    void end_row() {}

    /**
     * Called after every cell is parsed.
     *
     * @param value cell content.
     * @param transient when true, the text content has been converted and is
     *                  stored in a temporary buffer.  In such case, there is
     *                  no guarantee that the text content remain available
     *                  after the end of the call.  When this value is false,
     *                  the text content is guaranteed to be valid so long as
     *                  the original CSV stream content is valid.
     */
    void cell(std::string_view value, bool transient)
    {
        (void)value; (void)transient;
    }
};

/**
 * Parser for CSV documents.
 *
 * @tparam HandlerT Hanlder type with member functions for event callbacks.
 *         Refer to csv_handler.
 */
template<typename HandlerT>
class csv_parser : public csv::parser_base
{
public:
    typedef HandlerT handler_type;

    csv_parser(std::string_view content, handler_type& hdl, const csv::parser_config& config);
    void parse();

private:

    // handlers
    void row();
    void cell();
    void quoted_cell();

    void parse_cell_with_quote(const char* p0, size_t len0);

    /**
     * Push cell value to the handler.
     */
    void push_cell_value(const char* p, size_t n);

private:
    handler_type& m_handler;
};

template<typename _Handler>
csv_parser<_Handler>::csv_parser(
    std::string_view content, handler_type& hdl, const csv::parser_config& config) :
    csv::parser_base(content, config), m_handler(hdl) {}

template<typename _Handler>
void csv_parser<_Handler>::parse()
{
#if ORCUS_DEBUG_CSV
    for (const char* p = mp_begin; p < mp_end; ++p)
        std::cout << *p;
    std::cout << std::endl;
#endif

    m_handler.begin_parse();
    while (has_char())
        row();
    m_handler.end_parse();
}

template<typename _Handler>
void csv_parser<_Handler>::row()
{
    m_handler.begin_row();
    while (true)
    {
        if (is_text_qualifier(cur_char()))
            quoted_cell();
        else
            cell();

        if (!has_char())
        {
            m_handler.end_row();
            return;
        }

        char c = cur_char();
        if (c == '\n')
        {
            next();
#if ORCUS_DEBUG_CSV
            cout << "(LF)" << endl;
#endif
            m_handler.end_row();
            return;
        }

        if (!is_delim(c))
            throw orcus::parse_error("expected a delimiter", offset());

        next();

        if (m_config.trim_cell_value)
            skip_blanks();

        if (!has_char())
        {
            m_handler.end_row();
            return;
        }
    }
}

template<typename _Handler>
void csv_parser<_Handler>::cell()
{
    const char* p = mp_char;
    size_t len = 0;
    char c = cur_char();
    while (c != '\n' && !is_delim(c))
    {
        ++len;
        next();
        if (!has_char())
            break;
        c = cur_char();
    }

    if (!len)
        p = nullptr;

    push_cell_value(p, len);
}

template<typename _Handler>
void csv_parser<_Handler>::quoted_cell()
{
#if ORCUS_DEBUG_CSV
    cout << "--- quoted cell" << endl;
#endif
    char c = cur_char();
    assert(is_text_qualifier(c));
    next(); // Skip the opening quote.
    if (!has_char())
        return;

    const char* p0 = mp_char;
    size_t len = 1;
    for (; has_char(); next(), ++len)
    {
        c = cur_char();
#if ORCUS_DEBUG_CSV
        cout << "'" << c << "'" << endl;
#endif
        if (!is_text_qualifier(c))
            continue;

        // current char is a quote. Check if the next char is also a text
        // qualifier.

        if (has_next() && is_text_qualifier(peek_char()))
        {
            next();
            parse_cell_with_quote(p0, len);
            return;
        }

        // Closing quote.
        m_handler.cell({p0, len-1}, false);
        next();
        skip_blanks();
        return;
    }

    // Stream ended prematurely.  Handle it gracefully.
    m_handler.cell({p0, len}, false);
}

template<typename _Handler>
void csv_parser<_Handler>::parse_cell_with_quote(const char* p0, size_t len0)
{
#if ORCUS_DEBUG_CSV
    using namespace std;
    cout << "--- parse cell with quote" << endl;
#endif
    assert(is_text_qualifier(cur_char()));

    // Push the preceding chars to the temp buffer.
    m_cell_buf.reset();
    m_cell_buf.append(p0, len0);

    // Parse the rest, until the closing quote.
    next();
    const char* p_cur = mp_char;
    size_t cur_len = 0;
    for (; has_char(); next(), ++cur_len)
    {
        char c = cur_char();
#if ORCUS_DEBUG_CSV
        cout << "'" << c << "'" << endl;
#endif
        if (!is_text_qualifier(c))
            continue;

        if (has_next() && is_text_qualifier(peek_char()))
        {
            // double quotation.  Copy the current segment to the cell buffer.
            m_cell_buf.append(p_cur, cur_len);

            next(); // to the 2nd quote.
            p_cur = mp_char;
            cur_len = 0;
            continue;
        }

        // closing quote.  Flush the current segment to the cell
        // buffer, push the value to the handler, and exit normally.
        m_cell_buf.append(p_cur, cur_len);

        m_handler.cell(m_cell_buf.str(), true);
        next();
        skip_blanks();
        return;
    }

    // Stream ended prematurely.
    throw parse_error("stream ended prematurely while parsing quoted cell.", offset());
}

template<typename _Handler>
void csv_parser<_Handler>::push_cell_value(const char* p, size_t n)
{
    size_t len = n;

    if (m_config.trim_cell_value)
    {
        // Trim any leading blanks.
        for (size_t i = 0; i < n; ++i, --len, ++p)
        {
            if (!is_blank(*p))
                break;
        }

        // Trim any trailing blanks.
        if (len)
        {
            const char* p_end = p + (len-1);
            for (; p != p_end; --p_end, --len)
            {
                if (!is_blank(*p_end))
                    break;
            }
        }
    }

    m_handler.cell({p, len}, false);
#if ORCUS_DEBUG_CSV
    if (len)
        cout << "(cell:'" << std::string(p, len) << "')" << endl;
    else
        cout << "(cell:'')" << endl;
#endif
}

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
