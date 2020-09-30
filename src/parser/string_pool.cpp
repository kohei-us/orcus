/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/string_pool.hpp"

#include "orcus/global.hpp"
#include "orcus/pstring.hpp"
#include "orcus/exception.hpp"

#include <iostream>
#include <unordered_set>
#include <vector>
#include <memory>
#include <cassert>
#include <algorithm>

#include <boost/pool/object_pool.hpp>

namespace orcus {

using std::cout;
using std::endl;

using string_set_type = std::unordered_set<pstring, pstring::hash>;
using string_store_type = boost::object_pool<std::string>;
using string_stores_type = std::vector<std::unique_ptr<string_store_type>>;

struct string_pool::impl
{
    string_stores_type m_stores;
    string_set_type m_set;

    impl()
    {
        // first element is the active store used for the current instance.
        m_stores.push_back(std::make_unique<string_store_type>(256, 0));
    }
};

string_pool::string_pool() : mp_impl(std::make_unique<impl>()) {}

string_pool::~string_pool()
{
    clear();
}

std::pair<pstring, bool> string_pool::intern(const char* str)
{
    return intern(str, strlen(str));
}

std::pair<pstring, bool> string_pool::intern(const char* str, size_t n)
{
    if (!n)
        return std::pair<pstring, bool>(pstring(), false);

    string_set_type::const_iterator itr = mp_impl->m_set.find(pstring(str, n));
    if (itr == mp_impl->m_set.end())
    {
        // This string has not been interned.  Intern it.
        string_store_type& store = *mp_impl->m_stores[0];
        std::string* p = store.construct(str, n);
        if (!p)
            throw general_error("failed to intern a new string instance.");

        std::pair<string_set_type::iterator,bool> r =
            mp_impl->m_set.emplace(p->data(), p->size());
        if (!r.second)
            throw general_error("failed to intern a new string instance.");

        const pstring& ps = *r.first;
        assert(ps.size() == n);

        return std::pair<pstring, bool>(ps, true);
    }

    // This string has already been interned.

    const pstring& stored_str = *itr;
    assert(stored_str.size() == n);
    return std::pair<pstring, bool>(stored_str, false);
}

std::pair<pstring, bool> string_pool::intern(const pstring& str)
{
    return intern(str.get(), str.size());
}

std::vector<pstring> string_pool::get_interned_strings() const
{
    std::vector<pstring> sorted;
    sorted.reserve(mp_impl->m_set.size());

    for (const pstring& ps : mp_impl->m_set)
        sorted.push_back(ps);

    std::sort(sorted.begin(), sorted.end());

    return sorted;
}

void string_pool::dump() const
{
    auto sorted = get_interned_strings();

    cout << "interned string count: " << sorted.size() << endl;

    // Dump them all to stdout.
    size_t counter = 0;
    for (const pstring& s : sorted)
        cout << counter++ << ": '" << s << "'" << endl;
}

void string_pool::clear()
{
    mp_impl->m_set.clear();
    mp_impl->m_stores.clear();
}

size_t string_pool::size() const
{
    return mp_impl->m_set.size();
}

void string_pool::swap(string_pool& other)
{
    std::swap(mp_impl, other.mp_impl);
}

void string_pool::merge(string_pool& other)
{
    while (!other.mp_impl->m_stores.empty())
    {
        mp_impl->m_stores.push_back(
            std::move(other.mp_impl->m_stores.back()));
        other.mp_impl->m_stores.pop_back();
    }

    for (const pstring& p : other.mp_impl->m_set)
        mp_impl->m_set.insert(p);

    other.mp_impl->m_set.clear();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
