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

pivot_cache_item_t::pivot_cache_item_t() : type(item_type::unknown) {}

pivot_cache_item_t::pivot_cache_item_t(const char* p_str, size_t n_str) :
    type(item_type::string)
{
    value.string.p = p_str;
    value.string.n = n_str;
}

pivot_cache_item_t::pivot_cache_item_t(double _numeric) :
    type(item_type::numeric)
{
    value.numeric = _numeric;
}

pivot_cache_item_t::pivot_cache_item_t(bool _boolean) :
    type(item_type::boolean)
{
    value.boolean = _boolean;
}

pivot_cache_item_t::pivot_cache_item_t(const pivot_cache_item_t& other) :
    type(other.type)
{
    switch (type)
    {
        case item_type::blank:
            break;
        case item_type::boolean:
            value.boolean = other.value.boolean;
            break;
        case item_type::datetime:
            // TODO : add this.
            break;
        case item_type::error:
            // TODO : add this.
            break;
        case item_type::numeric:
            value.numeric = other.value.numeric;
            break;
        case item_type::string:
            value.string.p = other.value.string.p;
            value.string.n = other.value.string.n;
            break;
        case item_type::unknown:
            break;
        default:
            ;
    }
}

pivot_cache_item_t::pivot_cache_item_t(pivot_cache_item_t&& other) :
    type(other.type)
{
    other.type = item_type::unknown;

    switch (type)
    {
        case item_type::blank:
            break;
        case item_type::boolean:
            value.boolean = other.value.boolean;
            break;
        case item_type::datetime:
            // TODO : add this.
            break;
        case item_type::error:
            // TODO : add this.
            break;
        case item_type::numeric:
            value.numeric = other.value.numeric;
            break;
        case item_type::string:
            value.string.p = other.value.string.p;
            value.string.n = other.value.string.n;
            break;
        case item_type::unknown:
            break;
        default:
            ;
    }
}

bool pivot_cache_item_t::operator< (const pivot_cache_item_t& other) const
{
    if (type != other.type)
        return type < other.type;

    switch (type)
    {
        case item_type::boolean:
            return value.boolean < other.value.boolean;
        case item_type::numeric:
            return value.numeric < other.value.numeric;
        case item_type::string:
            return pstring(value.string.p, value.string.n) < pstring(other.value.string.p, other.value.string.n);
        case item_type::datetime:
            // TODO : implement this.
            break;
        case item_type::error:
            // TODO : implement this.
            break;
        case item_type::blank:
        case item_type::unknown:
        default:
            ;
    }

    return false;
}

bool pivot_cache_item_t::operator== (const pivot_cache_item_t& other) const
{
    if (type != other.type)
        return false;

    switch (type)
    {
        case item_type::boolean:
            return value.boolean == other.value.boolean;
        case item_type::numeric:
            return value.numeric == other.value.numeric;
        case item_type::string:
            return pstring(value.string.p, value.string.n) == pstring(other.value.string.p, other.value.string.n);
        case item_type::datetime:
            // TODO : implement this.
            break;
        case item_type::error:
            // TODO : implement this.
            break;
        case item_type::blank:
        case item_type::unknown:
            return true;
        default:
            ;
    }

    return false;
}

pivot_cache_group_data_t::pivot_cache_group_data_t(size_t _base_field) :
    base_field(_base_field) {}

pivot_cache_group_data_t::pivot_cache_group_data_t(const pivot_cache_group_data_t& other) :
    base_to_group_indices(other.base_to_group_indices),
    numeric_range(other.numeric_range),
    items(other.items),
    base_field(other.base_field) {}

pivot_cache_group_data_t::pivot_cache_group_data_t(pivot_cache_group_data_t&& other) :
    base_to_group_indices(std::move(other.base_to_group_indices)),
    numeric_range(std::move(other.numeric_range)),
    items(std::move(other.items)),
    base_field(other.base_field) {}

pivot_cache_field_t::pivot_cache_field_t() {}

pivot_cache_field_t::pivot_cache_field_t(const pstring& _name) : name(_name) {}

pivot_cache_field_t::pivot_cache_field_t(const pivot_cache_field_t& other) :
    name(other.name),
    items(other.items),
    min_value(other.min_value),
    max_value(other.max_value),
    group_data(orcus::make_unique<pivot_cache_group_data_t>(*other.group_data)) {}

pivot_cache_field_t::pivot_cache_field_t(pivot_cache_field_t&& other) :
    name(other.name),
    items(std::move(other.items)),
    min_value(std::move(other.min_value)),
    max_value(std::move(other.max_value)),
    group_data(std::move(other.group_data))
{
    other.name.clear();
}

struct pivot_cache::impl
{
    string_pool& m_string_pool;

    pstring m_src_sheet_name;

    pivot_cache::fields_type m_fields;

    impl(string_pool& sp) : m_string_pool(sp) {}
};

pivot_cache::pivot_cache(string_pool& sp) :
    mp_impl(orcus::make_unique<impl>(sp)) {}

pivot_cache::~pivot_cache() {}

void pivot_cache::insert_fields(fields_type fields)
{
    mp_impl->m_fields = std::move(fields);
}

size_t pivot_cache::get_field_count() const
{
    return mp_impl->m_fields.size();
}

const pivot_cache_field_t* pivot_cache::get_field(size_t index) const
{
    return index < mp_impl->m_fields.size() ? &mp_impl->m_fields[index] : nullptr;
}

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

size_t pivot_collection::get_cache_count() const
{
    return mp_impl->m_caches.size();
}

const pivot_cache* pivot_collection::get_cache(
    const pstring& sheet_name, const ixion::abs_range_t& range) const
{
    worksheet_range wr(sheet_name, range);

    auto it = mp_impl->m_worksheet_range_map.find(wr);
    if (it == mp_impl->m_worksheet_range_map.end())
        return nullptr;

    size_t cache_id = it->second;
    assert(cache_id < mp_impl->m_caches.size());
    return mp_impl->m_caches[cache_id].get();
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
