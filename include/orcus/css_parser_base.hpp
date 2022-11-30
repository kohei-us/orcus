/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CSS_PARSER_BASE_HPP
#define INCLUDED_CSS_PARSER_BASE_HPP

#include "orcus/env.hpp"
#include "orcus/css_types.hpp"
#include "orcus/exception.hpp"
#include "orcus/parser_base.hpp"

#include <string>
#include <exception>

namespace orcus { namespace css {

class ORCUS_PSR_DLLPUBLIC parser_base : public ::orcus::parser_base
{
public:
    parser_base(std::string_view content);

protected:

    void identifier(const char*& p, size_t& len, std::string_view extra = std::string_view{});
    uint8_t parse_uint8();

    /**
     * Parse an unquoted property value until one of non-value characters is
     * reached.
     *
     * @return parsed value segment.
     */
    std::string_view parse_value();
    double parse_percent();
    double parse_double_or_throw();

    void literal(const char*& p, size_t& len, char quote);
    void skip_to(const char*& p, size_t& len, char c);

    /**
     * Skip until one of specified characters or a blank character is reached.
     *
     * @param p pointer to the first character of the skipped character array.
     * @param len length of the skipped character array.
     * @param chars one or more characters that can end the skipping.
     */
    void skip_to_or_blank(const char*& p, size_t& len, std::string_view chars);
    void skip_blanks();
    void skip_blanks_reverse();
    void shrink_stream();
    bool skip_comment();
    void comment();
    void skip_comments_and_blanks();
    void set_combinator(char c, css::combinator_t combinator);
    void reset_before_block();

protected:
    size_t m_simple_selector_count;
    combinator_t m_combinator;
};


}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
