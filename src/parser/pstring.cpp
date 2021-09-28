/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pstring.hpp"
#include "orcus/string_pool.hpp"
#include "orcus/parser_global.hpp"

#include <cassert>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

namespace orcus {

size_t pstring::hash::operator() (const pstring& val) const
{
    uint32_t hash_val = 0; // use 32-bit int for better performance
    const char* p = val.get();

    for (uint32_t i = 0, n = val.size(); i < n; ++i, ++p)
    {
        hash_val *= 0x01000193;
        hash_val ^= *p;
    }

    return hash_val;
}

pstring::pstring(const char* _pos) :
    m_pos(_pos),
    m_size(_pos ? std::strlen(_pos) : 0)
{
}

bool pstring::operator== (std::string_view r) const
{
    if (m_pos == r.data())
        return m_size == r.size();

    if (m_size != r.size())
        // lengths differ.
        return false;

    return std::equal(m_pos, m_pos + m_size, r.data());
}

bool pstring::operator== (const pstring& r) const
{
    if (m_pos == r.m_pos)
        return m_size == r.m_size;

    if (m_size != r.m_size)
        // lengths differ.
        return false;

    return std::equal(m_pos, m_pos + m_size, r.m_pos);
}

bool pstring::operator< (const pstring& r) const
{
    return std::lexicographical_compare(m_pos, m_pos + m_size, r.m_pos, r.m_pos + r.m_size);
}

bool pstring::operator== (const char* _str) const
{
    size_t n = std::strlen(_str);
    if (n != m_size)
        // lengths differ.
        return false;

    if (!m_size)
        // both are empty strings.
        return true;

    return std::memcmp(_str, m_pos, n) == 0;
}

pstring pstring::trim() const
{
    const char* p = m_pos;
    const char* p_end = p + m_size;

    // Find the first non-space character.
    p = std::find_if_not(p, p_end, is_blank);

    if (p == p_end)
    {
        // This string is empty.
        return pstring();
    }

    // Find the last non-space character.
    auto last = std::find_if_not(std::reverse_iterator(p_end), std::reverse_iterator(p), is_blank);

    return pstring(p, last.base() - p);
}

void pstring::resize(size_t new_size)
{
    m_size = new_size;
}

std::string operator+ (const std::string& left, const pstring& right)
{
    std::string ret = left;
    return ret += right;
}

std::string& operator+= (std::string& left, const pstring& right)
{
    if (!right.empty())
    {
        left.append(right.get(), right.size());
    }
    return left;
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
