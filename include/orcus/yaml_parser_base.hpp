/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_YAML_PARSER_BASE_HPP
#define INCLUDED_ORCUS_YAML_PARSER_BASE_HPP

#include "orcus/parser_base.hpp"
#include "orcus/pstring.hpp"

#include <memory>
#include <cassert>

namespace orcus { namespace yaml {

class ORCUS_PSR_DLLPUBLIC parse_error : public ::orcus::parse_error
{
public:
    parse_error(const std::string& msg, std::ptrdiff_t offset);

    static void throw_with(const char* msg_before, char c, const char* msg_after, std::ptrdiff_t offset);
    static void throw_with(const char* msg_before, const char* p, size_t n, const char* msg_after, std::ptrdiff_t offset);
};

namespace detail {

enum class scope_t
{
    unset,
    sequence,
    map,
    multi_line_string
};

enum class keyword_t
{
    unknown,
    boolean_true,
    boolean_false,
    null
};

enum class parse_token_t
{
    unknown,

    // handler tokens (tokens associated with handler events)

    begin_parse,
    end_parse,
    begin_document,
    end_document,
    begin_sequence,
    end_sequence,
    begin_map,
    end_map,
    begin_map_key,
    end_map_key,
    string,
    number,
    boolean_true,
    boolean_false,
    null,

    // non-handler tokens

    begin_sequence_element
};

}

class ORCUS_PSR_DLLPUBLIC parser_base : public ::orcus::parser_base
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

protected:

    // The entire line is empty.
    static const size_t parse_indent_blank_line;

    // End of stream has reached while parsing in the indent part of a line.
    static const size_t parse_indent_end_of_stream;

    static const size_t scope_empty;

    struct key_value
    {
        pstring key;
        pstring value;
    };

    parser_base() = delete;
    parser_base(const parser_base&) = delete;
    parser_base& operator=(const parser_base&) = delete;

    parser_base(const char* p, size_t n);
    ~parser_base();

    void push_parse_token(detail::parse_token_t t);

    detail::parse_token_t get_last_parse_token() const;

    /**
     * Get the offset position of the last character of the current line
     * without comment or trailing whitespaces (if present).  Call this only
     * after the current line has been parsed to the end, that is, only after
     * parse_to_end_of_line() has been called.
     *
     * @return offset position of the last character of the current line.
     */
    size_t offset_last_char_of_line() const;

    /**
     * Parse the prefix indent part of a line.
     *
     * @return number of whitespace characters encountered.
     */
    size_t parse_indent();

    /**
     * Once a non-whitespace character is reached, parse until the end of the
     * line.
     */
    pstring parse_to_end_of_line();

    /**
     * Upon encountering a '#', skip until either the line-feed or the
     * end-of-stream is reached.
     */
    void skip_comment();

    void reset_on_new_line();

    size_t get_scope() const;

    void push_scope(size_t scope_width);

    void clear_scopes();

    detail::scope_t get_scope_type() const;

    void set_scope_type(detail::scope_t type);

    /**
     * Pop the current scope and return the new scope width after the pop.
     *
     * @return new scope width after the pop.
     */
    size_t pop_scope();

    void push_line_back(const char* p, size_t n);

    pstring pop_line_front();

    bool has_line_buffer() const;

    size_t get_line_buffer_count() const;

    pstring merge_line_buffer();

    /**
     * Get the hash value of current document, or nullptr if a document has
     * not started.
     *
     * @return hash value of current document.
     */
    const char* get_doc_hash() const;

    /**
     * Set the hash value representing the current document.  For now the
     * memory address of the first character of the document is used as its
     * hash value.
     *
     * @param hash hash value of a document.
     */
    void set_doc_hash(const char* hash);

    detail::keyword_t parse_keyword(const char* p, size_t len);

    key_value parse_key_value(const char* p, size_t len);

    pstring parse_single_quoted_string_value(const char*& p, size_t max_length);

    pstring parse_double_quoted_string_value(const char*& p, size_t max_length);

    void skip_blanks(const char*& p, size_t len);

    void start_literal_block();

    bool in_literal_block() const;

    void handle_line_in_literal(size_t indent);

    void handle_line_in_multi_line_string();
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
