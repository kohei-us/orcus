/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/spreadsheet/pivot.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/global.hpp"
#include "orcus/string_pool.hpp"

#include <ixion/address.hpp>

#include <unordered_map>
#include <cassert>

namespace orcus { namespace spreadsheet {

struct pivot_cache::impl
{
    string_pool& m_string_pool;

    pstring m_src_sheet_name;

    impl(string_pool& sp) : m_string_pool(sp) {}
};

pivot_cache::pivot_cache(string_pool& sp) :
    mp_impl(orcus::make_unique<impl>(sp)) {}

pivot_cache::~pivot_cache() {}

namespace {

constexpr const ixion::sheet_t ignored_sheet = -1;

struct worksheet_range
{
    pstring sheet; /// it must be an interned string with the document.
    ixion::abs_range_t range; /// sheet indices are ignored.

    worksheet_range(pstring _sheet, ixion::abs_range_t _range) :
        sheet(std::move(_sheet)), range(std::move(_range))
    {
        range.first.sheet = ignored_sheet;
        range.last.sheet = ignored_sheet;
    }

    bool operator== (const worksheet_range& other) const
    {
        return sheet == other.sheet && range == other.range;
    }

    struct hash
    {
        pstring::hash ps_hasher;
        ixion::abs_range_t::hash range_hasher;

        size_t operator() (const worksheet_range& v) const
        {
            assert(v.range.first.sheet == ignored_sheet);
            assert(v.range.last.sheet == ignored_sheet);

            size_t n = ps_hasher(v.sheet);
            n ^= range_hasher(v.range);
            return n;
        }
    };
};

using range_map_type = std::unordered_map<worksheet_range, size_t, worksheet_range::hash>;

}

struct pivot_collection::impl
{
    document& m_doc;

    range_map_type m_worksheet_range_map; /// mapping of sheet name & range pair to cache ID.
    std::vector<std::unique_ptr<pivot_cache>> m_caches;

    impl(document& doc) : m_doc(doc) {}
};

pivot_collection::pivot_collection(document& doc) : mp_impl(orcus::make_unique<impl>(doc)) {}

pivot_collection::~pivot_collection() {}

void pivot_collection::insert_worksheet_cache(
    const pstring& sheet_name, const ixion::abs_range_t& range,
    std::unique_ptr<pivot_cache>&& cache)
{
    // Check and see if there is already a cache for this location.  If yes,
    // overwrite the existing cache.

    // sheet name must be interned with the document it belongs to.
    worksheet_range key(mp_impl->m_doc.get_string_pool().intern(sheet_name).first, range);

    auto it = mp_impl->m_worksheet_range_map.find(key);

    if (it != mp_impl->m_worksheet_range_map.end())
    {
        // Overwrite an existing cache.
        size_t cache_id = it->second;
        assert(cache_id < mp_impl->m_caches.size());
        mp_impl->m_caches[cache_id] = std::move(cache);
        return;
    }

    // This is a new worksheet cache.
    size_t cache_id = mp_impl->m_caches.size();
    mp_impl->m_caches.push_back(std::move(cache));
    mp_impl->m_worksheet_range_map.insert(
        range_map_type::value_type(std::move(key), cache_id));
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
