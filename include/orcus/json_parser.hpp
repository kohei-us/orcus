/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_JSON_PARSER_HPP
#define INCLUDED_ORCUS_JSON_PARSER_HPP

#include "orcus/json_parser_base.hpp"

#include <cassert>
#include <cmath>

namespace orcus {

class json_handler
{
public:
    /**
     * Called when the parsing begins.
     */
    void begin_parse() {}

    /**
     * Called when the parsing ends.
     */
    void end_parse() {}

    /**
     * Called when the opening brace of an array is encountered.
     */
    void begin_array() {}

    /**
     * Called when the closing brace of an array is encountered.
     */
    void end_array() {}

    /**
     * Called when the opening curly brace of an object is encountered.
     */
    void begin_object() {}

    /**
     * Called when a key value string of an object is encountered.
     *
     * @param p pointer to the first character of the key value string.
     * @param len length of the key value string.
     * @param transient true if the string value is stored in a temporary
     *                  buffer which is not guaranteed to hold the string
     *                  value after the end of this callback. When false, the
     *                  pointer points to somewhere in the JSON stream being
     *                  parsed.
     */
    void object_key(const char* p, size_t len, bool transient) {}

    /**
     * Called when the closing curly brace of an object is encountered.
     */
    void end_object() {}

    /**
     * Called when a boolean 'true' keyword is encountered.
     */
    void boolean_true() {}

    /**
     * Called when a boolean 'false' keyword is encountered.
     */
    void boolean_false() {}

    /**
     * Called when a 'null' keyword is encountered.
     */
    void null() {}

    /**
     * Called when a string value is encountered.
     *
     * @param p pointer to the first character of the string value.
     * @param len length of the string value.
     * @param transient true if the string value is stored in a temporary
     *                  buffer which is not guaranteed to hold the string
     *                  value after the end of this callback. When false, the
     *                  pointer points to somewhere in the JSON stream being
     *                  parsed.
     */
    void string(const char* p, size_t len, bool transient) {}

    /**
     * Called when a numeric value is encountered.
     *
     * @param val numeric value.
     */
    void number(double val) {}
};

/**
 * Low-level JSON parser.  The caller must provide a handler class to
 * receive callbacks.
 */
template<typename _Handler>
class json_parser : public json::parser_base
{
public:
    typedef _Handler handler_type;

    /**
     * Constructor.
     *
     * @param p pointer to a string stream containing JSON string.
     * @param n size of the stream.
     * @param hdl handler class instance.
     */
    json_parser(const char* p, size_t n, handler_type& hdl);

    /**
     * Call this method to start parsing.
     */
    void parse();

private:
    void root_value();
    void value();
    void array();
    void end_array();
    void object();
    void number();
    void string();

private:
    handler_type& m_handler;
};

template<typename _Handler>
json_parser<_Handler>::json_parser(
    const char* p, size_t n, handler_type& hdl) :
    json::parser_base(p, n), m_handler(hdl) {}

template<typename _Handler>
void json_parser<_Handler>::parse()
{
    m_handler.begin_parse();

    skip_ws();
    if (has_char())
        root_value();
    else
        throw json::parse_error("parse: no json content could be found in file", offset());

    if (has_char())
        throw json::parse_error("parse: unexpected trailing string segment.", offset());

    m_handler.end_parse();
}

template<typename _Handler>
void json_parser<_Handler>::root_value()
{
    char c = cur_char();

    switch (c)
    {
        case '[':
            array();
        break;
        case '{':
            object();
        break;
        default:
            json::parse_error::throw_with(
                "root_value: either '[' or '{' was expected, but '", cur_char(), "' was found.", offset());
    }
}

template<typename _Handler>
void json_parser<_Handler>::value()
{
    char c = cur_char();
    if (is_numeric(c))
    {
        number();
        return;
    }

    switch (c)
    {
        case '-':
            number();
        break;
        case '[':
            array();
        break;
        case '{':
            object();
        break;
        case 't':
            parse_true();
            m_handler.boolean_true();
        break;
        case 'f':
            parse_false();
            m_handler.boolean_false();
        break;
        case 'n':
            parse_null();
            m_handler.null();
        break;
        case '"':
            string();
        break;
        default:
            json::parse_error::throw_with("value: failed to parse '", cur_char(), "'.", offset());
    }
}

template<typename _Handler>
void json_parser<_Handler>::array()
{
    assert(cur_char() == '[');

    m_handler.begin_array();
    for (next(); has_char(); next())
    {
        skip_ws();

        if (cur_char() == ']')
        {
            end_array();
            return;
        }

        value();
        skip_ws();

        if (has_char())
        {
            switch (cur_char())
            {
                case ']':
                    end_array();
                    return;
                case ',':
                    if (next_char() == ']')
                    {
                        json::parse_error::throw_with(
                            "array: ']' expected but '", cur_char(), "' found.", offset() );
                    }
                    continue;
                default:
                    json::parse_error::throw_with(
                        "array: either ']' or ',' expected, but '", cur_char(), "' found.", offset());
            }
        }
        else
        {
            // needs to be handled here,
            // we would call next() before checking again with has_char() which
            // is already past the end
            break;
        }
    }

    throw json::parse_error("array: failed to parse array.", offset());
}

template<typename _Handler>
void json_parser<_Handler>::end_array()
{
    m_handler.end_array();
    next();
    skip_ws();
}

template<typename _Handler>
void json_parser<_Handler>::object()
{
    assert(cur_char() == '{');

    bool require_new_key = false;
    m_handler.begin_object();
    for (next(); has_char(); next())
    {
        skip_ws();
        if (!has_char())
            throw json::parse_error("object: stream ended prematurely before reaching a key.", offset());

        switch (cur_char())
        {
            case '}':
                if (require_new_key)
                {
                    json::parse_error::throw_with(
                        "object: new key expected, but '", cur_char(), "' found.", offset());
                }
                m_handler.end_object();
                next();
                skip_ws();
                return;
            case '"':
                break;
            default:
                json::parse_error::throw_with(
                    "object: '\"' was expected, but '", cur_char(), "' found.", offset());
        }
        require_new_key = false;

        parse_quoted_string_state res = parse_string();
        if (!res.str)
        {
            // Parsing was unsuccessful.
            if (res.length == parse_quoted_string_state::error_no_closing_quote)
                throw json::parse_error("object: stream ended prematurely before reaching the closing quote of a key.", offset());
            else if (res.length == parse_quoted_string_state::error_illegal_escape_char)
                json::parse_error::throw_with(
                    "object: illegal escape character '", cur_char(), "' in key value.", offset());
            else
                throw json::parse_error("object: unknown error while parsing a key value.", offset());
        }

        m_handler.object_key(res.str, res.length, res.transient);

        skip_ws();
        if (cur_char() != ':')
            json::parse_error::throw_with(
                "object: ':' was expected, but '", cur_char(), "' found.", offset());

        next();
        skip_ws();

        if (!has_char())
            throw json::parse_error("object: stream ended prematurely before reaching a value.", offset());

        value();

        skip_ws();
        if (!has_char())
            throw json::parse_error("object: stream ended prematurely before reaching either '}' or ','.", offset());

        switch (cur_char())
        {
            case '}':
                m_handler.end_object();
                next();
                skip_ws();
                return;
            case ',':
                require_new_key = true;
                continue;
            default:
                json::parse_error::throw_with(
                    "object: either '}' or ',' expected, but '", cur_char(), "' found.", offset());
        }
    }

    throw json::parse_error("object: closing '}' was never reached.", offset());
}

template<typename _Handler>
void json_parser<_Handler>::number()
{
    assert(is_numeric(cur_char()) || cur_char() == '-');

    double val = parse_double_or_throw();
    m_handler.number(val);
    skip_ws();
}

template<typename _Handler>
void json_parser<_Handler>::string()
{
    parse_quoted_string_state res = parse_string();
    if (res.str)
    {
        m_handler.string(res.str, res.length, res.transient);
        return;
    }

    // Parsing was unsuccessful.
    if (res.length == parse_quoted_string_state::error_no_closing_quote)
        throw json::parse_error("string: stream ended prematurely before reaching the closing quote.", offset());
    else if (res.length == parse_quoted_string_state::error_illegal_escape_char)
        json::parse_error::throw_with("string: illegal escape character '", cur_char(), "'.", offset());
    else
        throw json::parse_error("string: unknown error.", offset());
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
