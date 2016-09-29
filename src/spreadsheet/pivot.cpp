/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/spreadsheet/pivot.hpp"
#include "orcus/global.hpp"
#include "orcus/string_pool.hpp"

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

}

struct pivot_collection::impl
{
    std::vector<std::unique_ptr<pivot_cache>> m_caches;
};

pivot_collection::pivot_collection() : mp_impl(orcus::make_unique<impl>()) {}

pivot_collection::~pivot_collection() {}

void pivot_collection::insert_worksheet_cache(
    const pstring& sheet_name, const ixion::abs_range_t& range,
    std::unique_ptr<pivot_cache>&& cache)
{
    // TODO: check and see if there is already a cache for this location.  If
    // yes, overwrite the existing cache.

//  size_t cache_id = mp_impl->m_caches.size();

    // TODO: Associate this cache ID with the name and the range.

    mp_impl->m_caches.push_back(std::move(cache));
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
