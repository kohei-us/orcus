/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xls_filter_utils.hpp"

#include <cassert>

namespace ss = orcus::spreadsheet;

namespace orcus { namespace detail {

auto xls_filter_value_parser::parse(ss::auto_filter_op_t op, std::string_view value) -> result_type
{
    switch (op)
    {
        case ss::auto_filter_op_t::equal:
        case ss::auto_filter_op_t::not_equal:
            break; // only parse for equality operators
        default:
            return {op, value, false};
    }

    m_buf.clear();

    mp_char = value.data();
    mp_end = mp_char + value.size();

    if (mp_char == mp_end)
        // empty value
        return {op, value};

    bool head_star = *mp_char == '*';
    if (head_star)
    {
        ++mp_char;

        if (mp_char == mp_end)
        {
            m_buf = ".*";
            return {op, m_buf, true};
        }

        value = value.substr(1); // skip the first char
    }

    bool tail_star = *(mp_end-1) == '*';
    if (tail_star && value.size() >= 2 && *(mp_end-2) == '~')
        tail_star = false;

    if (tail_star)
    {
        --mp_end;
        value = value.substr(0, value.size()-1); // skip the last char
    }

    parse_chars();

    if (!m_buf.empty())
        value = m_buf;

    if (head_star)
    {
        if (tail_star)
        {
            if (op == ss::auto_filter_op_t::equal)
                op = ss::auto_filter_op_t::contain;
            else
                op = ss::auto_filter_op_t::not_contain;
        }
        else
        {
            if (op == ss::auto_filter_op_t::equal)
                op = ss::auto_filter_op_t::end_with;
            else
                op = ss::auto_filter_op_t::not_end_with;
        }
    }
    else if (tail_star)
    {
        if (op == ss::auto_filter_op_t::equal)
            op = ss::auto_filter_op_t::begin_with;
        else
            op = ss::auto_filter_op_t::not_begin_with;
    }

    return {op, value, m_regex};
}

void xls_filter_value_parser::parse_chars()
{
    assert(m_buf.empty());

    m_regex = false;

    const char* p0 = nullptr;
    char c = 0;
    char c_prev = 0;

    for (; mp_char != mp_end; ++mp_char, c_prev = c)
    {
        if (!p0)
            p0 = mp_char;

        c = *mp_char;

        switch (c)
        {
            case '?':
            case '*':
            {
                assert(p0);

                if (c_prev == '~')
                {
                    // escaped '*'
                    if (auto len = std::distance(p0, mp_char-1); len)
                        m_buf += std::string_view(p0, len);

                    m_buf += c;
                }
                else
                {
                    // convert this to regex
                    m_regex = true;
                    if (auto len = std::distance(p0, mp_char); len)
                        m_buf += std::string_view(p0, len);

                    m_buf += (c == '*') ? ".*" : ".";
                }

                p0 = nullptr;
                break;
            }
        }
    }

    if (p0 && !m_buf.empty())
    {
        if (auto len = std::distance(p0, mp_char); len)
            m_buf += std::string_view(p0, len);
    }
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
