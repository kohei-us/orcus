/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/json_parser_base.hpp"
#include "orcus/cell_buffer.hpp"
#include "numeric_parser.hpp"

#include <cassert>
#include <cmath>

namespace orcus { namespace json {

namespace {

const char* parse_numeric_json(const char* p, const char* p_end, double& value)
{
    using numeric_parser_type = detail::numeric_parser<detail::json_parser_trait>;

    numeric_parser_type parser(p, p_end);
    double v = parser.parse();
    if (!std::isnan(v))
        p = parser.get_char_position();

    value = v;
    return p;
};

} // anonymous namespace

struct parser_base::impl
{
    cell_buffer m_buffer;
};

parser_base::parser_base(std::string_view content) :
    orcus::parser_base(content.data(), content.size()), mp_impl(std::make_unique<impl>())
{

    set_numeric_parser(parse_numeric_json);
}

parser_base::~parser_base() {}

void parser_base::skip_ws()
{
    skip(" \n\r\t");
}

void parser_base::parse_true()
{
    if (!parse_expected("true"))
        throw parse_error("parse_true: boolean 'true' expected.", offset());

    skip_ws();
}

void parser_base::parse_false()
{
    if (!parse_expected("false"))
        throw parse_error("parse_false: boolean 'false' expected.", offset());

    skip_ws();
}

void parser_base::parse_null()
{
    if (!parse_expected("null"))
        throw parse_error("parse_null: null expected.", offset());

    skip_ws();
}

double parser_base::parse_double_or_throw()
{
    double v = parse_double();
    if (std::isnan(v))
        throw parse_error("parse_double_or_throw: failed to parse double precision value.", offset());
    return v;
}

parse_quoted_string_state parser_base::parse_string()
{
    assert(cur_char() == '"');
    size_t max_length = remaining_size();
    const char* p = mp_char;
    parse_quoted_string_state ret = parse_double_quoted_string(p, max_length, mp_impl->m_buffer);
    if (ret.has_control_character)
        throw parse_error("parse_string: string contains a control character.", offset());

    mp_char = p;

    if (ret.str)
        skip_ws();

    return ret;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
