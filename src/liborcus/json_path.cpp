/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "json_path.hpp"
#include <orcus/exception.hpp>
#include <orcus/measurement.hpp>

#include <cassert>
#include <sstream>
#include <iostream>

namespace orcus {

json_path_part_t::json_path_part_t(json_path_t type) : m_type(type)
{
    switch (m_type)
    {
        case json_path_t::root:
        case json_path_t::array_all:
            return;
        default:
            throw invalid_arg_error("only root or array-all token can be value-less");
    }
}

json_path_part_t::json_path_part_t(std::size_t array_index) :
    m_type(json_path_t::array_index), m_value(array_index) {}

json_path_part_t::json_path_part_t(std::string_view object_key) :
    m_type(json_path_t::object_key), m_value(object_key) {}

json_path_t json_path_part_t::type() const
{
    return m_type;
}

std::string_view json_path_part_t::object_key() const
{
    return std::get<std::string_view>(m_value);
}

std::size_t json_path_part_t::array_index() const
{
    return std::get<std::size_t>(m_value);
}

bool json_path_part_t::operator==(const json_path_part_t& other) const
{
    if (m_type != other.m_type)
        return false;

    return m_value == other.m_value;
}

bool json_path_part_t::operator!=(const json_path_part_t& other) const
{
    return !operator==(other);
}

namespace {

std::string_view to_object_key(const char* p_key, const char* p_key_end)
{
    auto key_length = std::distance(p_key, p_key_end);
    if (!key_length)
        throw invalid_arg_error("empty object key");

    std::string_view key(p_key, key_length);
    return key;
}

} // anonymous namespace

void json_path_parser::object_key()
{
    // It should point to the first char after '.' or the end position.

    // Parse until it encounters either '.' or '[', or the stream ends.  When
    // successful, it should point to the character past the last char of the
    // key.

    const char* p_head = mp;

    for (; mp != mp_end; ++mp)
    {
        switch (*mp)
        {
            case '.':
            case '[':
            {
                m_parts.emplace_back(to_object_key(p_head, mp));
                return;
            }
        }
    }

    m_parts.emplace_back(to_object_key(p_head, mp));
}

void json_path_parser::object_key_in_brackets()
{
    // Parse until it counters the closing quote (') followed by a ']'.
    assert(mp != mp_end);
    assert(*mp == '\'');

    ++mp; // skip the opening quote
    const char* p_head = mp;

    for (; mp != mp_end; ++mp)
    {
        if (*mp == '\'')
        {
            m_parts.emplace_back(to_object_key(p_head, mp));
            ++mp; // skip the closing quote
            if (mp == mp_end || *mp != ']')
                throw invalid_arg_error("closing quote in object key in bracket notation must be followed by ']'");

            ++mp; // skip the closing bracket
            return;
        }
    }

    throw invalid_arg_error("object key in bracket notation ended prematurely");
}

void json_path_parser::bracket()
{
    // Parse until it encounters ']'. When successful, it should point to the
    // next char after the ']'.

    assert(mp != mp_end);
    assert(*mp == '[');

    ++mp;

    const char* p_head = mp;

    if (mp != mp_end && *mp == '\'')
    {
        object_key_in_brackets();
        return;
    }

    for (; mp != mp_end; ++mp)
    {
        switch (*mp)
        {
            case ']':
            {
                std::size_t n = std::distance(p_head, mp);
                std::string_view s{p_head, n};

                if (s == "*")
                {
                    m_parts.emplace_back(json_path_t::array_all);
                }
                else
                {
                    auto v = to_long_checked(s);
                    if (!v)
                    {
                        std::ostringstream os;
                        os << "failed to convert to integer: '" << s << "'";
                        throw invalid_arg_error(os.str());
                    }

                    if (*v < 0)
                    {
                        std::ostringstream os;
                        os << "array index must be positive (" << *v << ")";
                        throw invalid_arg_error(os.str());
                    }

                    m_parts.emplace_back(std::size_t(*v));
                }

                ++mp; // skip ']'
                return;
            }
        }
    }

    throw invalid_arg_error("no closing bracket ']' was encountered after opening bracket '['");
}

void json_path_parser::parse(std::string_view expression)
{
    m_parts.clear();

    if (expression.empty())
        return;

    mp = expression.data();
    mp_end = mp + expression.size();

    assert(mp < mp_end);

    switch (*mp)
    {
        case '$':
        {
            m_parts.emplace_back(json_path_t::root);
            ++mp;
            break;
        }
        case '[':
        {
            bracket();
            break;
        }
        default:
            object_key();
    }

    while (mp != mp_end)
    {
        switch (*mp)
        {
            case '.':
            {
                ++mp;
                object_key();
                break;
            }
            case '[':
            {
                bracket();
                break;
            }
            default:
            {
                std::ostringstream os;
                os << "unexpected character '" << *mp << "'";
                throw invalid_arg_error(os.str());
            }
        }
    }
}

json_path_parts_t json_path_parser::pop_parts()
{
    return std::move(m_parts);
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
