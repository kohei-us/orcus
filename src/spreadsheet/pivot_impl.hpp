/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <orcus/spreadsheet/pivot.hpp>
#include <orcus/string_pool.hpp>
#include <ixion/address.hpp>

#include <cassert>

namespace orcus { namespace spreadsheet {

namespace detail {

constexpr const ixion::sheet_t ignored_sheet = -1;

struct worksheet_range
{
    std::string_view sheet; /// it must be an interned string with the document.
    ixion::abs_range_t range; /// sheet indices are ignored.

    worksheet_range(std::string_view _sheet, ixion::abs_range_t _range) :
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
        std::hash<std::string_view> ps_hasher;
        ixion::abs_range_t::hash range_hasher;

        std::size_t operator() (const worksheet_range& v) const;
    };
};

using range_map_type = std::unordered_map<worksheet_range, std::unordered_set<pivot_cache_id_t>, worksheet_range::hash>;
using name_map_type = std::unordered_map<std::string_view, std::unordered_set<pivot_cache_id_t>>;
using caches_type = std::unordered_map<pivot_cache_id_t, std::unique_ptr<pivot_cache>>;

}

struct pivot_cache::impl
{
    pivot_cache_id_t m_cache_id;

    string_pool& m_string_pool;

    std::string_view m_src_sheet_name;

    pivot_cache::fields_type m_fields;
    pivot_cache::records_type m_records;

    impl(pivot_cache_id_t cache_id, string_pool& sp);
};

struct pivot_collection::impl
{
    document& m_doc;

    detail::range_map_type m_worksheet_range_map; /// mapping of sheet name & range pair to cache ID.
    detail::name_map_type m_table_map; /// mapping of table name to cache ID.
    detail::caches_type m_caches;

    impl(document& doc);

    void ensure_unique_cache(pivot_cache_id_t cache_id);
};

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
