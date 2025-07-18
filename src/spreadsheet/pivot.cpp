/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/spreadsheet/pivot.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/string_pool.hpp"

#include "filesystem_env.hpp"
#include "pivot_impl.hpp"
#include "debug_state_context.hpp"
#include "debug_state_dumper_pivot.hpp"

#include <unordered_map>
#include <cassert>
#include <sstream>
#include <iostream>
#include <iomanip>

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

pivot_cache_group_data_t::range_grouping_type::range_grouping_type() = default;
pivot_cache_group_data_t::range_grouping_type::range_grouping_type(const range_grouping_type& other) = default;

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

pivot_cache_field_t& pivot_cache_field_t::operator=(pivot_cache_field_t other)
{
    swap(other);
    return *this;
}

void pivot_cache_field_t::swap(pivot_cache_field_t& other) noexcept
{
    name.swap(other.name);
    items.swap(other.items);
    min_value.swap(other.min_value);
    max_value.swap(other.max_value);
    min_date.swap(other.min_date);
    max_date.swap(other.max_date);
    group_data.swap(other.group_data);
}

pivot_item_t::pivot_item_t() = default;
pivot_item_t::pivot_item_t(const pivot_item_t& other) = default;
pivot_item_t::pivot_item_t(pivot_item_t&& other) = default;
pivot_item_t::pivot_item_t(std::size_t i, bool _hidden) : type(item_type::index), value(i), hidden(_hidden) {}
pivot_item_t::pivot_item_t(pivot_field_item_t t) : type(item_type::type), value(t) {}
pivot_item_t::~pivot_item_t() = default;


pivot_item_t& pivot_item_t::operator=(pivot_item_t other)
{
    swap(other);
    return *this;
}

void pivot_item_t::swap(pivot_item_t& other) noexcept
{
    std::swap(type, other.type);
    std::swap(value, other.value);
    std::swap(hidden, other.hidden);
}

pivot_field_t::pivot_field_t() = default;
pivot_field_t::pivot_field_t(const pivot_field_t& other) = default;
pivot_field_t::pivot_field_t(pivot_field_t&& other) = default;
pivot_field_t::~pivot_field_t() = default;

pivot_field_t& pivot_field_t::operator=(pivot_field_t other)
{
    swap(other);
    return *this;
}

void pivot_field_t::swap(pivot_field_t& other) noexcept
{
    std::swap(axis, other.axis);
    std::swap(items, other.items);
}

pivot_ref_rc_field_t::pivot_ref_rc_field_t() = default;
pivot_ref_rc_field_t::pivot_ref_rc_field_t(const pivot_ref_rc_field_t& other) = default;
pivot_ref_rc_field_t::pivot_ref_rc_field_t(pivot_ref_rc_field_t&& other) = default;

pivot_ref_rc_field_t::pivot_ref_rc_field_t(std::size_t _index) :
    type(value_type::index),
    index(_index) {}

pivot_ref_rc_field_t::pivot_ref_rc_field_t(value_type vt) :
    type(vt) {}

pivot_ref_rc_field_t::~pivot_ref_rc_field_t() = default;

pivot_ref_rc_field_t& pivot_ref_rc_field_t::operator=(pivot_ref_rc_field_t other)
{
    swap(other);
    return *this;
}

void pivot_ref_rc_field_t::swap(pivot_ref_rc_field_t& other) noexcept
{
    std::swap(type, other.type);
    std::swap(index, other.index);
}

pivot_ref_page_field_t::pivot_ref_page_field_t() = default;
pivot_ref_page_field_t::pivot_ref_page_field_t(const pivot_ref_page_field_t& other) = default;
pivot_ref_page_field_t::pivot_ref_page_field_t(pivot_ref_page_field_t&& other) = default;
pivot_ref_page_field_t::~pivot_ref_page_field_t() = default;

pivot_ref_page_field_t& pivot_ref_page_field_t::operator=(pivot_ref_page_field_t other)
{
    swap(other);
    return *this;
}

void pivot_ref_page_field_t::swap(pivot_ref_page_field_t& other) noexcept
{
    std::swap(field, other.field);
    std::swap(item, other.item);
}

pivot_ref_data_field_t::pivot_ref_data_field_t() = default;
pivot_ref_data_field_t::pivot_ref_data_field_t(const pivot_ref_data_field_t& other) = default;
pivot_ref_data_field_t::pivot_ref_data_field_t(pivot_ref_data_field_t&& other) = default;
pivot_ref_data_field_t::~pivot_ref_data_field_t() = default;

pivot_ref_data_field_t& pivot_ref_data_field_t::operator=(pivot_ref_data_field_t other)
{
    swap(other);
    return *this;
}

void pivot_ref_data_field_t::swap(pivot_ref_data_field_t& other) noexcept
{
    std::swap(field, other.field);
    std::swap(name, other.name);
    std::swap(subtotal, other.subtotal);
    std::swap(show_data_as, other.show_data_as);
    std::swap(base_field, other.base_field);
    std::swap(base_item, other.base_item);
}

pivot_ref_rc_item_t::pivot_ref_rc_item_t() = default;
pivot_ref_rc_item_t::pivot_ref_rc_item_t(const pivot_ref_rc_item_t& other) = default;
pivot_ref_rc_item_t::pivot_ref_rc_item_t(pivot_ref_rc_item_t&& other) = default;
pivot_ref_rc_item_t::~pivot_ref_rc_item_t() = default;

pivot_ref_rc_item_t& pivot_ref_rc_item_t::operator=(pivot_ref_rc_item_t other)
{
    swap(other);
    return *this;
}

void pivot_ref_rc_item_t::swap(pivot_ref_rc_item_t& other) noexcept
{
    std::swap(type, other.type);
    std::swap(repeat, other.repeat);
    items.swap(other.items);
    std::swap(data_item, other.data_item);
}

pivot_cache::pivot_cache(pivot_cache_id_t cache_id, string_pool& sp) :
    mp_impl(std::make_unique<impl>(cache_id, sp)) {}

pivot_cache::~pivot_cache() {}

void pivot_cache::insert_fields(fields_type fields)
{
    mp_impl->fields = std::move(fields);
}

void pivot_cache::insert_records(records_type records)
{
    mp_impl->records = std::move(records);
}

size_t pivot_cache::get_field_count() const
{
    return mp_impl->fields.size();
}

const pivot_cache_field_t* pivot_cache::get_field(size_t index) const
{
    return index < mp_impl->fields.size() ? &mp_impl->fields[index] : nullptr;
}

pivot_cache_id_t pivot_cache::get_id() const
{
    return mp_impl->cache_id;
}

const pivot_cache::records_type& pivot_cache::get_all_records() const
{
    return mp_impl->records;
}

pivot_table::pivot_table(string_pool& pool) : mp_impl(std::make_unique<impl>(pool)) {}

pivot_table::pivot_table(pivot_table&& other) : mp_impl(std::move(other.mp_impl)) {}

pivot_table::~pivot_table() = default;

pivot_table& pivot_table::operator=(pivot_table&& other)
{
    mp_impl = std::move(other.mp_impl);
    return *this;
}

std::string_view pivot_table::get_name() const
{
    return mp_impl->name;
}

void pivot_table::set_name(std::string_view name)
{
    mp_impl->name = mp_impl->pool.intern(name).first;
}

void pivot_table::set_cache_id(pivot_cache_id_t cache_id)
{
    mp_impl->cache_id = cache_id;
}

void pivot_table::set_range(const ixion::abs_rc_range_t& range)
{
    mp_impl->range = range;
}

void pivot_table::set_pivot_fields(pivot_fields_t fields)
{
    mp_impl->fields = std::move(fields);
}

void pivot_table::set_row_fields(pivot_ref_rc_fields_t fields)
{
    mp_impl->row_fields = std::move(fields);
}

void pivot_table::set_column_fields(pivot_ref_rc_fields_t fields)
{
    mp_impl->column_fields = std::move(fields);
}

void pivot_table::set_page_fields(pivot_ref_page_fields_t fields)
{
    mp_impl->page_fields = std::move(fields);
}

void pivot_table::set_data_fields(pivot_ref_data_fields_t fields)
{
    mp_impl->data_fields = std::move(fields);
}

void pivot_table::set_row_items(pivot_ref_rc_items_t items)
{
    mp_impl->row_items = items;
}

void pivot_table::set_column_items(pivot_ref_rc_items_t items)
{
    mp_impl->column_items = items;
}

pivot_collection::pivot_collection(document& doc) : mp_impl(std::make_unique<impl>(doc)) {}

pivot_collection::~pivot_collection() = default;

void pivot_collection::insert_worksheet_cache(
    std::string_view sheet_name, const ixion::abs_range_t& range,
    std::unique_ptr<pivot_cache>&& cache)
{
    // First, ensure that no caches exist for the cache ID.
    pivot_cache_id_t cache_id = cache->get_id();
    mp_impl->ensure_unique_cache(cache_id);

    // Check and see if there is already a cache for this location.  If yes,
    // overwrite the existing cache.
    mp_impl->caches[cache_id] = std::move(cache);

    detail::worksheet_range key(sheet_name, range);

    auto& range_map = mp_impl->worksheet_range_map;
    auto it = range_map.find(key);

    if (it == range_map.end())
    {
        // sheet name must be interned with the document it belongs to.
        key.sheet = mp_impl->doc.get_string_pool().intern(key.sheet).first;
        range_map.insert(detail::range_map_type::value_type(std::move(key), {cache_id}));
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

    mp_impl->caches[cache_id] = std::move(cache);

    auto& name_map = mp_impl->table_map;
    auto it = name_map.find(table_name);

    if (it == name_map.end())
    {
        // First cache to be associated with this name.
        std::string_view table_name_interned =
            mp_impl->doc.get_string_pool().intern(table_name).first;
        name_map.insert(detail::name_map_type::value_type(table_name_interned, {cache_id}));
        return;
    }

    auto& id_set = it->second;
    id_set.insert(cache_id);
}

void pivot_collection::insert_pivot_table(pivot_table pt)
{
    mp_impl->pivot_tables.push_back(std::move(pt));
}

size_t pivot_collection::get_cache_count() const
{
    return mp_impl->caches.size();
}

const pivot_cache* pivot_collection::get_cache(
    std::string_view sheet_name, const ixion::abs_range_t& range) const
{
    detail::worksheet_range wr(sheet_name, range);

    auto it = mp_impl->worksheet_range_map.find(wr);
    if (it == mp_impl->worksheet_range_map.end())
        return nullptr;

    // Pick the first cache ID.
    assert(!it->second.empty());
    pivot_cache_id_t cache_id = *it->second.cbegin();
    return mp_impl->caches[cache_id].get();
}

namespace {

template<typename _CachesT, typename _CacheT>
_CacheT* get_cache_impl(_CachesT& caches, pivot_cache_id_t cache_id)
{
    auto it = caches.find(cache_id);
    return it == caches.end() ? nullptr : it->second.get();
}

fs::path to_path(const pivot_collection::outdir_type& outdir)
{
    switch (outdir.index())
    {
        case 0:
            return std::get<std::string_view>(outdir);
        case 1:
            return std::get<std::u16string_view>(outdir);
    }

    std::ostringstream os;
    os << "invalid outdir_type in pivot_collection (" << outdir.index() << ")";
    throw std::invalid_argument(os.str());
}

} // anonymous namespace

pivot_cache* pivot_collection::get_cache(pivot_cache_id_t cache_id)
{
    return get_cache_impl<detail::caches_type, pivot_cache>(mp_impl->caches, cache_id);
}

const pivot_cache* pivot_collection::get_cache(pivot_cache_id_t cache_id) const
{
    return get_cache_impl<const detail::caches_type, const pivot_cache>(mp_impl->caches, cache_id);
}

void pivot_collection::dump_debug_state(const outdir_type& outdir) const
{
    auto output_dir = to_path(outdir);

    for (const auto& [id, cache] : mp_impl->caches)
    {
        auto this_dir = output_dir;
        this_dir /= "pivot-caches";
        fs::create_directories(this_dir);

        detail::debug_state_dumper_pivot_cache dumper(*cache->mp_impl);
        dumper.dump(this_dir);
    }

    std::size_t pos = 0;

    for (const auto& table : mp_impl->pivot_tables)
    {
        auto this_path = output_dir / "pivot-tables";
        fs::create_directories(this_path);

        std::ostringstream os;
        os << "pivot-" << pos++ << ".yaml";
        this_path /= os.str();

        detail::debug_state_context cxt;
        detail::debug_state_dumper_pivot_table dumper(cxt, *table.mp_impl);
        dumper.dump(this_path, mp_impl->caches);
    }
}

std::ostream& operator<<(std::ostream& os, const pivot_cache_item_t& item)
{
    switch (item.type)
    {
        case pivot_cache_item_t::item_type::unknown:
            os << "(unknown)";
            break;
        case pivot_cache_item_t::item_type::boolean:
            os << std::boolalpha << std::get<bool>(item.value);
            break;
        case pivot_cache_item_t::item_type::date_time:
            os << std::get<date_time_t>(item.value).to_string();
            break;
        case pivot_cache_item_t::item_type::character:
            os << std::get<std::string_view>(item.value);
            break;
        case pivot_cache_item_t::item_type::numeric:
            os << std::get<double>(item.value);
            break;
        case pivot_cache_item_t::item_type::blank:
            os << "(blank)";
            break;
        case pivot_cache_item_t::item_type::error:
            os << std::get<error_value_t>(item.value);
            break;
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const pivot_cache_record_value_t& v)
{

    switch (v.type)
    {
        case pivot_cache_record_value_t::record_type::unknown:
        {
            os << "(unknown)";
            break;
        }
        case pivot_cache_record_value_t::record_type::boolean:
        {
            os << std::boolalpha << std::get<bool>(v.value);
            break;
        }
        case pivot_cache_record_value_t::record_type::date_time:
        {
            os << std::get<date_time_t>(v.value).to_string();
            break;
        }
        case pivot_cache_record_value_t::record_type::character:
        {
            os << std::get<std::string_view>(v.value);
            break;
        }
        case pivot_cache_record_value_t::record_type::numeric:
        {
            os << std::get<double>(v.value);
            break;
        }
        case pivot_cache_record_value_t::record_type::blank:
        {
            os << "(blank)";
            break;
        }
        case pivot_cache_record_value_t::record_type::error:
        {
            os << std::get<error_value_t>(v.value);
            break;
        }
        case pivot_cache_record_value_t::record_type::shared_item_index:
        {
            os << '(' << std::get<std::size_t>(v.value) << ')';
            break;
        }
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const pivot_item_t& v)
{
    switch (v.type)
    {
        case pivot_item_t::item_type::index:
        {
            os << "(index: " << std::get<std::size_t>(v.value);
            break;
        }
        case pivot_item_t::item_type::type:
        {
            os << "(type: " << std::get<pivot_field_item_t>(v.value);
            break;
        }
        case pivot_item_t::item_type::unknown:
        {
            os << "(unknown";
            break;
        }
    }

    os << "; hidden: " << std::boolalpha << v.hidden << ")";
    return os;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
