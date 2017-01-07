/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_YAML_PARSER_HPP
#define INCLUDED_ORCUS_YAML_PARSER_HPP

#include "orcus/yaml_parser_base.hpp"
#include "orcus/parser_global.hpp"

namespace orcus {

template<typename _Handler>
class yaml_parser : public yaml::parser_base
{
public:
    typedef _Handler handler_type;

    yaml_parser(const char* p, size_t n, handler_type& hdl);

    void parse();

private:
    size_t end_scope();
    void check_or_begin_document();
    void check_or_begin_map();
    void check_or_begin_sequence();
    void parse_value(const char* p, size_t len);
    void push_value(const char* p, size_t len);
    void parse_line(const char* p, size_t len);
    void parse_map_key(const char* p, size_t len);

    void handler_begin_parse();
    void handler_end_parse();
    void handler_begin_document();
    void handler_end_document();
    void handler_begin_sequence();
    void handler_end_sequence();
    void handler_begin_map();
    void handler_end_map();
    void handler_begin_map_key();
    void handler_end_map_key();
    void handler_string(const char* p, size_t n);
    void handler_number(double val);
    void handler_boolean_true();
    void handler_boolean_false();
    void handler_null();

private:
    handler_type& m_handler;
};

template<typename _Handler>
void yaml_parser<_Handler>::handler_begin_parse()
{
    push_parse_token(yaml::detail::parse_token_t::begin_parse);
    m_handler.begin_parse();
}

template<typename _Handler>
void yaml_parser<_Handler>::handler_end_parse()
{
    push_parse_token(yaml::detail::parse_token_t::end_parse);
    m_handler.end_parse();
}

template<typename _Handler>
void yaml_parser<_Handler>::handler_begin_document()
{
    push_parse_token(yaml::detail::parse_token_t::begin_document);
    m_handler.begin_document();
}

template<typename _Handler>
void yaml_parser<_Handler>::handler_end_document()
{
    push_parse_token(yaml::detail::parse_token_t::end_document);
    m_handler.end_document();
}

template<typename _Handler>
void yaml_parser<_Handler>::handler_begin_sequence()
{
    push_parse_token(yaml::detail::parse_token_t::begin_sequence);
    m_handler.begin_sequence();
}

template<typename _Handler>
void yaml_parser<_Handler>::handler_end_sequence()
{
    push_parse_token(yaml::detail::parse_token_t::end_sequence);
    m_handler.end_sequence();
}

template<typename _Handler>
void yaml_parser<_Handler>::handler_begin_map()
{
    push_parse_token(yaml::detail::parse_token_t::begin_map);
    m_handler.begin_map();
}

template<typename _Handler>
void yaml_parser<_Handler>::handler_end_map()
{
    push_parse_token(yaml::detail::parse_token_t::end_map);
    m_handler.end_map();
}

template<typename _Handler>
void yaml_parser<_Handler>::handler_begin_map_key()
{
    push_parse_token(yaml::detail::parse_token_t::begin_map_key);
    m_handler.begin_map_key();
}

template<typename _Handler>
void yaml_parser<_Handler>::handler_end_map_key()
{
    push_parse_token(yaml::detail::parse_token_t::end_map_key);
    m_handler.end_map_key();
}

template<typename _Handler>
void yaml_parser<_Handler>::handler_string(const char* p, size_t n)
{
    push_parse_token(yaml::detail::parse_token_t::string);
    m_handler.string(p, n);
}

template<typename _Handler>
void yaml_parser<_Handler>::handler_number(double val)
{
    push_parse_token(yaml::detail::parse_token_t::number);
    m_handler.number(val);
}

template<typename _Handler>
void yaml_parser<_Handler>::handler_boolean_true()
{
    push_parse_token(yaml::detail::parse_token_t::boolean_true);
    m_handler.boolean_true();
}

template<typename _Handler>
void yaml_parser<_Handler>::handler_boolean_false()
{
    push_parse_token(yaml::detail::parse_token_t::boolean_false);
    m_handler.boolean_false();
}

template<typename _Handler>
void yaml_parser<_Handler>::handler_null()
{
    push_parse_token(yaml::detail::parse_token_t::null);
    m_handler.null();
}

template<typename _Handler>
yaml_parser<_Handler>::yaml_parser(const char* p, size_t n, handler_type& hdl) :
    yaml::parser_base(p, n), m_handler(hdl) {}

template<typename _Handler>
void yaml_parser<_Handler>::parse()
{
    handler_begin_parse();

    while (has_char())
    {
        reset_on_new_line();

        size_t indent = parse_indent();
        if (indent == parse_indent_end_of_stream)
            break;

        if (indent == parse_indent_blank_line)
            continue;

        size_t cur_scope = get_scope();

        if (cur_scope <= indent)
        {
            if (in_literal_block())
            {
                handle_line_in_literal(indent);
                continue;
            }

            if (has_line_buffer())
            {
                // This line is part of multi-line string.  Push the line to the
                // buffer as-is.
                handle_line_in_multi_line_string();
                continue;
            }
        }

        if (cur_scope == scope_empty)
        {
            if (indent > 0)
                throw yaml::parse_error(
                    "first node of the document should not be indented.", offset());

            push_scope(indent);
        }
        else if (indent > cur_scope)
        {
            push_scope(indent);
        }
        else if (indent < cur_scope)
        {
            // Current indent is less than the current scope level.
            do
            {
                cur_scope = end_scope();
                if (cur_scope < indent)
                    throw yaml::parse_error("parse: invalid indent level.", offset());
            }
            while (indent < cur_scope);
        }

        // Parse the rest of the line.
        pstring line = parse_to_end_of_line();
        line = line.trim();

        assert(!line.empty());
        parse_line(line.get(), line.size());
    }

    // End all remaining scopes.
    size_t cur_scope = get_scope();
    while (cur_scope != scope_empty)
        cur_scope = end_scope();

    if (get_doc_hash())
        handler_end_document();

    handler_end_parse();
}

template<typename _Handler>
size_t yaml_parser<_Handler>::end_scope()
{
    switch (get_scope_type())
    {
        case yaml::detail::scope_t::map:
        {
            if (get_last_parse_token() == yaml::detail::parse_token_t::end_map_key)
                handler_null();

            handler_end_map();
            break;
        }
        case yaml::detail::scope_t::sequence:
        {
            if (get_last_parse_token() == yaml::detail::parse_token_t::begin_sequence_element)
                handler_null();

            handler_end_sequence();
            break;
        }
        case yaml::detail::scope_t::multi_line_string:
        {
            pstring merged = merge_line_buffer();
            handler_string(merged.get(), merged.size());
            break;
        }
        default:
        {
            if (has_line_buffer())
            {
                assert(get_line_buffer_count() == 1);
                pstring line = pop_line_front();
                parse_value(line.get(), line.size());
            }
        }
    }
    return pop_scope();
}

template<typename _Handler>
void yaml_parser<_Handler>::check_or_begin_document()
{
    if (!get_doc_hash())
    {
        set_doc_hash(mp_char);
        handler_begin_document();
    }
}

template<typename _Handler>
void yaml_parser<_Handler>::check_or_begin_map()
{
    switch (get_scope_type())
    {
        case yaml::detail::scope_t::unset:
        {
            check_or_begin_document();
            set_scope_type(yaml::detail::scope_t::map);
            handler_begin_map();
            break;
        }
        case yaml::detail::scope_t::map:
        {
            if (get_last_parse_token() == yaml::detail::parse_token_t::end_map_key)
                handler_null();
            break;
        }
        default:
            ;
    }
}

template<typename _Handler>
void yaml_parser<_Handler>::check_or_begin_sequence()
{
    switch (get_scope_type())
    {
        case yaml::detail::scope_t::unset:
        {
            check_or_begin_document();
            set_scope_type(yaml::detail::scope_t::sequence);
            handler_begin_sequence();
            break;
        }
        case yaml::detail::scope_t::sequence:
        {
            if (get_last_parse_token() == yaml::detail::parse_token_t::begin_sequence_element)
                handler_null();
            break;
        }
        default:
            ;
    }

    push_parse_token(yaml::detail::parse_token_t::begin_sequence_element);
}

template<typename _Handler>
void yaml_parser<_Handler>::parse_value(const char* p, size_t len)
{
    check_or_begin_document();

    const char* p0 = p;
    const char* p_end = p + len;
    double val = parse_numeric(p, len);
    if (p == p_end)
    {
        handler_number(val);
        return;
    }

    yaml::detail::keyword_t kw = parse_keyword(p0, len);

    if (kw != yaml::detail::keyword_t::unknown)
    {
        switch (kw)
        {
            case yaml::detail::keyword_t::null:
                handler_null();
            break;
            case yaml::detail::keyword_t::boolean_true:
                handler_boolean_true();
            break;
            case yaml::detail::keyword_t::boolean_false:
                handler_boolean_false();
            break;
            default:
                ;
        }

        return;
    }

    // Failed to parse it as a number or a keyword.  It must be a string.
    handler_string(p0, len);
}

template<typename _Handler>
void yaml_parser<_Handler>::push_value(const char* p, size_t len)
{
    check_or_begin_document();

    if (has_line_buffer() && get_scope_type() == yaml::detail::scope_t::unset)
        set_scope_type(yaml::detail::scope_t::multi_line_string);

    push_line_back(p, len);
}

template<typename _Handler>
void yaml_parser<_Handler>::parse_line(const char* p, size_t len)
{
    const char* p_end = p + len;
    const char* p0 = p; // Save the original head position.

    if (*p == '-')
    {
        ++p;
        if (p == p_end)
        {
            // List item start.
            check_or_begin_sequence();
            return;
        }

        switch (*p)
        {
            case '-':
            {
                // start of a document
                ++p;
                if (p == p_end)
                    throw yaml::parse_error("parse_line: line ended with '--'.", offset_last_char_of_line());

                if (*p != '-')
                    yaml::parse_error::throw_with(
                        "parse_line: '-' expected but '", *p, "' found.",
                        offset_last_char_of_line() - std::ptrdiff_t(p_end-p));

                ++p; // Skip the '-'.
                set_doc_hash(p);
                handler_begin_document();
                clear_scopes();

                if (p != p_end)
                {
                    skip_blanks(p, p_end-p);

                    // Whatever comes after '---' is equivalent of first node.
                    assert(p != p_end);
                    push_scope(0);
                    parse_line(p, p_end-p);
                }
            }
            break;
            case ' ':
            {
                check_or_begin_sequence();

                // list item start with inline first item content.
                ++p;
                if (p == p_end)
                    throw yaml::parse_error(
                        "parse_line: list item expected, but the line ended prematurely.",
                        offset_last_char_of_line() - std::ptrdiff_t(p_end-p));

                skip_blanks(p, p_end-p);

                size_t scope_width = get_scope() + (p-p0);
                push_scope(scope_width);
                parse_line(p, p_end-p);
            }
            break;
        }

        return;
    }

    // If the line doesn't start with a '-', it must be a dictionary key.
    parse_map_key(p, len);
}

template<typename _Handler>
void yaml_parser<_Handler>::parse_map_key(const char* p, size_t len)
{
    const char* p_end = p + len;
    const char* p0 = p; // Save the original head position.

    switch (*p)
    {
        case '"':
        {
            pstring quoted_str = parse_double_quoted_string_value(p, len);

            if (p == p_end)
            {
                handler_string(quoted_str.get(), quoted_str.size());
                return;
            }

            skip_blanks(p, p_end-p);

            if (*p != ':')
                throw yaml::parse_error(
                    "parse_map_key: ':' is expected after the quoted string key.",
                    offset() - std::ptrdiff_t(p_end-p+1));

            check_or_begin_map();
            handler_begin_map_key();
            handler_string(quoted_str.get(), quoted_str.size());
            handler_end_map_key();

            ++p;  // skip the ':'.
            if (p == p_end)
                return;

            // Skip all white spaces.
            skip_blanks(p, p_end-p);
        }
        break;
        case '\'':
        {
            pstring quoted_str = parse_single_quoted_string_value(p, len);

            if (p == p_end)
            {
                handler_string(quoted_str.get(), quoted_str.size());
                return;
            }

            skip_blanks(p, p_end-p);

            if (*p != ':')
                throw yaml::parse_error(
                    "parse_map_key: ':' is expected after the quoted string key.",
                    offset() - std::ptrdiff_t(p_end-p+1));

            check_or_begin_map();
            handler_begin_map_key();
            handler_string(quoted_str.get(), quoted_str.size());
            handler_end_map_key();

            ++p;  // skip the ':'.
            if (p == p_end)
                return;

            skip_blanks(p, p_end-p);
        }
        break;
        default:
        {
            key_value kv = parse_key_value(p, p_end-p);

            if (kv.key.empty())
            {
                // No map key found.
                if (*p == '|')
                {
                    start_literal_block();
                    return;
                }

                push_value(p, len);
                return;
            }

            check_or_begin_map();
            handler_begin_map_key();
            parse_value(kv.key.get(), kv.key.size());
            handler_end_map_key();

            if (kv.value.empty())
                return;

            p = kv.value.get();
        }
    }

    if (*p == '|')
    {
        start_literal_block();
        return;
    }

    // inline map item.
    if (*p == '-')
        throw yaml::parse_error(
            "parse_map_key: sequence entry is not allowed as an inline map item.",
            offset() - std::ptrdiff_t(p_end-p+1));

    size_t scope_width = get_scope() + (p-p0);
    push_scope(scope_width);
    parse_line(p, p_end-p);
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
