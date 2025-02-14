/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pivot_impl.hpp"

#include <sstream>

namespace orcus { namespace spreadsheet {

namespace detail {

std::size_t worksheet_range::hash::operator()(const worksheet_range& v) const
{
    assert(v.range.first.sheet == ignored_sheet);
    assert(v.range.last.sheet == ignored_sheet);

    size_t n = ps_hasher(v.sheet);
    n ^= range_hasher(v.range);
    return n;
}

} // namespace detail

pivot_cache::impl::impl(pivot_cache_id_t cache_id, string_pool& sp) :
    m_cache_id(cache_id), m_string_pool(sp) {}

pivot_collection::impl::impl(document& doc) : m_doc(doc) {}

void pivot_collection::impl::ensure_unique_cache(pivot_cache_id_t cache_id)
{
    if (m_caches.count(cache_id) > 0)
    {
        std::ostringstream os;
        os << "Pivot cache with the ID of " << cache_id << " already exists.";
        throw std::invalid_argument(os.str());
    }
}

}}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
