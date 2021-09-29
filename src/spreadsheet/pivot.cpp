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
#include <sstream>

namespace orcus { namespace spreadsheet {

pivot_cache_record_value_t::pivot_cache_record_value_t() :
    type(record_type::unknown), value(false) {}

pivot_cache_record_value_t::pivot_cache_record_value_t(std::string_view s) :
    type(record_type::character), value(s)
{
}

pivot_cache_record_value_t::pivot_cache_record_value_t(double v) :
    type(record_type::numeric), value(v)
{
}

pivot_cache_record_value_t::pivot_cache_record_value_t(size_t index) :
    type(record_type::shared_item_index), value(index)
{
}

bool pivot_cache_record_value_t::operator== (const pivot_cache_record_value_t& other) const
{
    return type == other.type && value == other.value;
}

bool pivot_cache_record_value_t::operator!= (const pivot_cache_record_value_t& other) const
{
    return !operator==(other);
}

pivot_cache_item_t::pivot_cache_item_t() : type(item_type::unknown) {}

pivot_cache_item_t::pivot_cache_item_t(std::string_view s) :
    type(item_type::character), value(s)
{
}

pivot_cache_item_t::pivot_cache_item_t(double numeric) :
    type(item_type::numeric), value(numeric)
{
}

pivot_cache_item_t::pivot_cache_item_t(bool boolean) :
    type(item_type::boolean), value(boolean)
{
}

pivot_cache_item_t::pivot_cache_item_t(const date_time_t& date_time) :
    type(item_type::date_time), value(date_time)
{
}

pivot_cache_item_t::pivot_cache_item_t(error_value_t error) :
    type(item_type::error), value(error)
{
}

pivot_cache_item_t::pivot_cache_item_t(const pivot_cache_item_t& other) :
    type(other.type), value(other.value)
{
}

pivot_cache_item_t::pivot_cache_item_t(pivot_cache_item_t&& other) :
    type(other.type), value(std::move(other.value))
{
    other.type = item_type::unknown;
    other.value = false;
}

bool pivot_cache_item_t::operator< (const pivot_cache_item_t& other) const
{
    if (type != other.type)
        return type < other.type;

    return value < other.value;
}

bool pivot_cache_item_t::operator== (const pivot_cache_item_t& other) const
{
    return type == other.type && value == other.value;
}

pivot_cache_item_t& pivot_cache_item_t::operator= (pivot_cache_item_t other)
{
    swap(other);
    return *this;
}

void pivot_cache_item_t::swap(pivot_cache_item_t& other)
{
    std::swap(type, other.type);
    std::swap(value, other.value);
}

pivot_cache_group_data_t::pivot_cache_group_data_t(size_t _base_field) :
    base_field(_base_field) {}

pivot_cache_group_data_t::pivot_cache_group_data_t(const pivot_cache_group_data_t& other) :
    base_to_group_indices(other.base_to_group_indices),
    range_grouping(other.range_grouping),
    items(other.items),
    base_field(other.base_field) {}

pivot_cache_group_data_t::pivot_cache_group_data_t(pivot_cache_group_data_t&& other) :
    base_to_group_indices(std::move(other.base_to_group_indices)),
    range_grouping(std::move(other.range_grouping)),
    items(std::move(other.items)),
    base_field(other.base_field) {}

pivot_cache_field_t::pivot_cache_field_t() {}

pivot_cache_field_t::pivot_cache_field_t(std::string_view _name) : name(_name) {}

pivot_cache_field_t::pivot_cache_field_t(const pivot_cache_field_t& other) :
    name(other.name),
    items(other.items),
    min_value(other.min_value),
    max_value(other.max_value),
    min_date(other.min_date),
    max_date(other.max_date),
    group_data(std::make_unique<pivot_cache_group_data_t>(*other.group_data)) {}

pivot_cache_field_t::pivot_cache_field_t(pivot_cache_field_t&& other) :
    name(other.name),
    items(std::move(other.items)),
    min_value(std::move(other.min_value)),
    max_value(std::move(other.max_value)),
    min_date(std::move(other.min_date)),
    max_date(std::move(other.max_date)),
    group_data(std::move(other.group_data))
{
    other.name = std::string_view{};
}

struct pivot_cache::impl
{
    pivot_cache_id_t m_cache_id;

    string_pool& m_string_pool;

    std::string_view m_src_sheet_name;

    pivot_cache::fields_type m_fields;

    pivot_cache::records_type m_records;

    impl(pivot_cache_id_t cache_id, string_pool& sp) :
        m_cache_id(cache_id), m_string_pool(sp) {}
};

pivot_cache::pivot_cache(pivot_cache_id_t cache_id, string_pool& sp) :
    mp_impl(std::make_unique<impl>(cache_id, sp)) {}

pivot_cache::~pivot_cache() {}

void pivot_cache::insert_fields(fields_type fields)
{
    mp_impl->m_fields = std::move(fields);
}

void pivot_cache::insert_records(records_type records)
{
    mp_impl->m_records = std::move(records);
}

size_t pivot_cache::get_field_count() const
{
    return mp_impl->m_fields.size();
}

const pivot_cache_field_t* pivot_cache::get_field(size_t index) const
{
    return index < mp_impl->m_fields.size() ? &mp_impl->m_fields[index] : nullptr;
}

pivot_cache_id_t pivot_cache::get_id() const
{
    return mp_impl->m_cache_id;
}

const pivot_cache::records_type& pivot_cache::get_all_records() const
{
    return mp_impl->m_records;
}

namespace {

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

using range_map_type = std::unordered_map<worksheet_range, std::unordered_set<pivot_cache_id_t>, worksheet_range::hash>;
using name_map_type = std::unordered_map<std::string_view, std::unordered_set<pivot_cache_id_t>>;

using caches_type = std::unordered_map<pivot_cache_id_t, std::unique_ptr<pivot_cache>>;

}

struct pivot_collection::impl
{
    document& m_doc;

    range_map_type m_worksheet_range_map; /// mapping of sheet name & range pair to cache ID.
    name_map_type m_table_map; /// mapping of table name to cache ID.

    caches_type m_caches;

    impl(document& doc) : m_doc(doc) {}

    void ensure_unique_cache(pivot_cache_id_t cache_id)
    {
        if (m_caches.count(cache_id) > 0)
        {
            std::ostringstream os;
            os << "Pivot cache with the ID of " << cache_id << " already exists.";
            throw std::invalid_argument(os.str());
        }
    }
};

pivot_collection::pivot_collection(document& doc) : mp_impl(std::make_unique<impl>(doc)) {}

pivot_collection::~pivot_collection() {}

void pivot_collection::insert_worksheet_cache(
    std::string_view sheet_name, const ixion::abs_range_t& range,
    std::unique_ptr<pivot_cache>&& cache)
{
    // First, ensure that no caches exist for the cache ID.
    pivot_cache_id_t cache_id = cache->get_id();
    mp_impl->ensure_unique_cache(cache_id);

    // Check and see if there is already a cache for this location.  If yes,
    // overwrite the existing cache.
    mp_impl->m_caches[cache_id] = std::move(cache);

    worksheet_range key(sheet_name, range);

    range_map_type& range_map = mp_impl->m_worksheet_range_map;
    auto it = range_map.find(key);

    if (it == range_map.end())
    {
        // sheet name must be interned with the document it belongs to.
        key.sheet = mp_impl->m_doc.get_string_pool().intern(key.sheet).first;
        range_map.insert(range_map_type::value_type(std::move(key), {cache_id}));
        return;
    }

    auto& id_set = it->second;
    id_set.insert(cache_id);
}

void pivot_collection::insert_worksheet_cache(
    std::string_view table_name, std::unique_ptr<pivot_cache>&& cache)
{
    // First, ensure that no caches exist for the cache ID.
    pivot_cache_id_t cache_id = cache->get_id();
    mp_impl->ensure_unique_cache(cache_id);

    mp_impl->m_caches[cache_id] = std::move(cache);

    name_map_type& name_map = mp_impl->m_table_map;
    auto it = name_map.find(table_name);

    if (it == name_map.end())
    {
        // First cache to be associated with this name.
        std::string_view table_name_interned =
            mp_impl->m_doc.get_string_pool().intern(table_name).first;
        name_map.insert(name_map_type::value_type(table_name_interned, {cache_id}));
        return;
    }

    auto& id_set = it->second;
    id_set.insert(cache_id);
}

size_t pivot_collection::get_cache_count() const
{
    return mp_impl->m_caches.size();
}

const pivot_cache* pivot_collection::get_cache(
    std::string_view sheet_name, const ixion::abs_range_t& range) const
{
    worksheet_range wr(sheet_name, range);

    auto it = mp_impl->m_worksheet_range_map.find(wr);
    if (it == mp_impl->m_worksheet_range_map.end())
        return nullptr;

    // Pick the first cache ID.
    assert(!it->second.empty());
    pivot_cache_id_t cache_id = *it->second.cbegin();
    return mp_impl->m_caches[cache_id].get();
}

namespace {

template<typename _CachesT, typename _CacheT>
_CacheT* get_cache_impl(_CachesT& caches, pivot_cache_id_t cache_id)
{
    auto it = caches.find(cache_id);
    return it == caches.end() ? nullptr : it->second.get();
}

}

pivot_cache* pivot_collection::get_cache(pivot_cache_id_t cache_id)
{
    return get_cache_impl<caches_type, pivot_cache>(mp_impl->m_caches, cache_id);
}

const pivot_cache* pivot_collection::get_cache(pivot_cache_id_t cache_id) const
{
    return get_cache_impl<const caches_type, const pivot_cache>(mp_impl->m_caches, cache_id);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
