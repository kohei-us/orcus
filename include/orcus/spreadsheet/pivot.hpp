/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_PIVOT_HPP
#define INCLUDED_ORCUS_SPREADSHEET_PIVOT_HPP

#include "../env.hpp"
#include "../types.hpp"
#include "types.hpp"

#include <memory>
#include <vector>
#include <limits>
#include <variant>
#include <optional>
#include <ostream>

namespace ixion {

struct abs_range_t;
struct abs_rc_range_t;

}

namespace orcus {

class string_pool;

namespace spreadsheet {

class document;
class pivot_collection;

namespace detail {

class debug_state_dumper_pivot_cache;
class debug_state_dumper_pivot_table;

}

using pivot_cache_indices_t = std::vector<size_t>;

struct ORCUS_SPM_DLLPUBLIC pivot_cache_record_value_t
{
    using value_type = std::variant<bool, double, std::size_t, std::string_view, date_time_t, error_value_t>;

    enum class record_type
    {
        unknown = 0,
        boolean,
        date_time,
        character,
        numeric,
        blank,
        error,
        shared_item_index
    };

    record_type type;
    value_type value;

    pivot_cache_record_value_t();
    pivot_cache_record_value_t(std::string_view s);
    pivot_cache_record_value_t(double v);
    pivot_cache_record_value_t(size_t index);

    bool operator== (const pivot_cache_record_value_t& other) const;
    bool operator!= (const pivot_cache_record_value_t& other) const;
};

using pivot_cache_record_t = std::vector<pivot_cache_record_value_t>;

struct ORCUS_SPM_DLLPUBLIC pivot_cache_item_t
{
    using value_type = std::variant<bool, double, std::string_view, date_time_t, error_value_t>;

    enum class item_type
    {
        unknown = 0, boolean, date_time, character, numeric, blank, error
    };

    item_type type;
    value_type value;

    pivot_cache_item_t();
    pivot_cache_item_t(std::string_view s);
    pivot_cache_item_t(double numeric);
    pivot_cache_item_t(bool boolean);
    pivot_cache_item_t(const date_time_t& date_time);
    pivot_cache_item_t(error_value_t error);

    pivot_cache_item_t(const pivot_cache_item_t& other);
    pivot_cache_item_t(pivot_cache_item_t&& other);

    bool operator< (const pivot_cache_item_t& other) const;
    bool operator== (const pivot_cache_item_t& other) const;

    pivot_cache_item_t& operator= (pivot_cache_item_t other);

    void swap(pivot_cache_item_t& other);
};

using pivot_cache_items_t = std::vector<pivot_cache_item_t>;

/**
 * Group data for a pivot cache field.
 */
struct ORCUS_SPM_DLLPUBLIC pivot_cache_group_data_t
{
    struct ORCUS_SPM_DLLPUBLIC range_grouping_type
    {
        pivot_cache_group_by_t group_by = pivot_cache_group_by_t::range;

        bool auto_start = true;
        bool auto_end   = true;

        double start    = 0.0;
        double end      = 0.0;
        double interval = 1.0;

        date_time_t start_date;
        date_time_t end_date;

        range_grouping_type();
        range_grouping_type(const range_grouping_type& other);
    };

    /**
     * Mapping of base field member indices to the group field item indices.
     */
    pivot_cache_indices_t base_to_group_indices;

    std::optional<range_grouping_type> range_grouping;

    /**
     * Individual items comprising the group.
     */
    pivot_cache_items_t items;

    /** 0-based index of the base field. */
    size_t base_field;

    pivot_cache_group_data_t(size_t _base_field);
    pivot_cache_group_data_t(const pivot_cache_group_data_t& other);
    pivot_cache_group_data_t(pivot_cache_group_data_t&& other);

    pivot_cache_group_data_t() = delete;
};

struct ORCUS_SPM_DLLPUBLIC pivot_cache_field_t
{
    /**
     * Field name. It must be interned with the string pool belonging to the
     * document.
     */
    std::string_view name;

    pivot_cache_items_t items;

    std::optional<double> min_value;
    std::optional<double> max_value;

    std::optional<date_time_t> min_date;
    std::optional<date_time_t> max_date;

    std::unique_ptr<pivot_cache_group_data_t> group_data;

    pivot_cache_field_t();
    pivot_cache_field_t(std::string_view _name);
    pivot_cache_field_t(const pivot_cache_field_t& other);
    pivot_cache_field_t(pivot_cache_field_t&& other);

    pivot_cache_field_t& operator=(pivot_cache_field_t other);

    void swap(pivot_cache_field_t& other) noexcept;
};

struct ORCUS_SPM_DLLPUBLIC pivot_item_t
{
    using value_type = std::variant<std::size_t, pivot_field_item_t>;

    enum class item_type
    {
        unknown = 0,
        index,
        type
    };

    item_type type;
    value_type value;
    bool hidden = false;

    pivot_item_t();
    pivot_item_t(const pivot_item_t& other);
    pivot_item_t(pivot_item_t&& other);
    pivot_item_t(std::size_t i, bool _hidden);
    pivot_item_t(pivot_field_item_t t);
    ~pivot_item_t();

    pivot_item_t& operator=(pivot_item_t other);

    void swap(pivot_item_t& other) noexcept;
};

using pivot_items_t = std::vector<pivot_item_t>;

struct ORCUS_SPM_DLLPUBLIC pivot_field_t
{
    pivot_axis_t axis = pivot_axis_t::unknown;
    pivot_items_t items;

    pivot_field_t();
    pivot_field_t(const pivot_field_t& other);
    pivot_field_t(pivot_field_t&& other);
    ~pivot_field_t();

    pivot_field_t& operator=(pivot_field_t other);

    void swap(pivot_field_t& other) noexcept;
};

using pivot_fields_t = std::vector<pivot_field_t>;

struct ORCUS_SPM_DLLPUBLIC pivot_ref_rc_field_t
{
    enum class value_type { unknown = 0, index, data };

    value_type type = value_type::unknown;
    std::size_t index = 0;

    pivot_ref_rc_field_t();
    pivot_ref_rc_field_t(const pivot_ref_rc_field_t& other);
    pivot_ref_rc_field_t(pivot_ref_rc_field_t&& other);
    pivot_ref_rc_field_t(std::size_t _index);
    pivot_ref_rc_field_t(value_type vt);
    ~pivot_ref_rc_field_t();

    pivot_ref_rc_field_t& operator=(pivot_ref_rc_field_t other);

    void swap(pivot_ref_rc_field_t& other) noexcept;
};

using pivot_ref_rc_fields_t = std::vector<pivot_ref_rc_field_t>;

struct ORCUS_SPM_DLLPUBLIC pivot_ref_page_field_t
{
    std::size_t field = 0;
    std::optional<std::size_t> item;

    pivot_ref_page_field_t();
    pivot_ref_page_field_t(const pivot_ref_page_field_t& other);
    pivot_ref_page_field_t(pivot_ref_page_field_t&& other);
    ~pivot_ref_page_field_t();

    pivot_ref_page_field_t& operator=(pivot_ref_page_field_t other);

    void swap(pivot_ref_page_field_t& other) noexcept;
};

using pivot_ref_page_fields_t = std::vector<pivot_ref_page_field_t>;

struct ORCUS_SPM_DLLPUBLIC pivot_ref_data_field_t
{
    std::size_t field = 0;
    std::string_view name;
    pivot_data_subtotal_t subtotal = pivot_data_subtotal_t::unknown;

    pivot_data_show_data_as_t show_data_as = pivot_data_show_data_as_t::unknown;
    std::size_t base_field = 0;
    std::size_t base_item = 0;

    pivot_ref_data_field_t();
    pivot_ref_data_field_t(const pivot_ref_data_field_t& other);
    pivot_ref_data_field_t(pivot_ref_data_field_t&& other);
    ~pivot_ref_data_field_t();

    pivot_ref_data_field_t& operator=(pivot_ref_data_field_t other);

    void swap(pivot_ref_data_field_t& other) noexcept;
};

using pivot_ref_data_fields_t = std::vector<pivot_ref_data_field_t>;

/**
 * A single row or column item displayed in a pivot table output.
 *
 * Each row or column item consists of zero or more empty labels followed by
 * non-empty labels.
 */
struct ORCUS_SPM_DLLPUBLIC pivot_ref_rc_item_t
{
    pivot_field_item_t type = pivot_field_item_t::unknown;

    /** Number of empty labels that occur before the non-empty labels appear. */
    std::size_t repeat = 0;
    std::vector<std::size_t> items;
    std::optional<std::size_t> data_item;

    pivot_ref_rc_item_t();
    pivot_ref_rc_item_t(const pivot_ref_rc_item_t& other);
    pivot_ref_rc_item_t(pivot_ref_rc_item_t&& other);
    ~pivot_ref_rc_item_t();

    pivot_ref_rc_item_t& operator=(pivot_ref_rc_item_t other);

    void swap(pivot_ref_rc_item_t& other) noexcept;
};

using pivot_ref_rc_items_t = std::vector<pivot_ref_rc_item_t>;

class ORCUS_SPM_DLLPUBLIC pivot_cache
{
    friend class detail::debug_state_dumper_pivot_cache;
    friend class detail::debug_state_dumper_pivot_table;
    friend class pivot_collection;

    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    using fields_type = std::vector<pivot_cache_field_t>;
    using records_type = std::vector<pivot_cache_record_t>;

    pivot_cache(pivot_cache_id_t cache_id, string_pool& sp);
    ~pivot_cache();

    /**
     * Bulk-insert all the fields in one step. Note that this will replace any
     * pre-existing fields if any.
     *
     * @param fields field instances to move into storage.
     */
    void insert_fields(fields_type fields);

    void insert_records(records_type record);

    size_t get_field_count() const;

    /**
     * Retrieve a field data by its index.
     *
     * @param index index of the field to retrieve.
     *
     * @return pointer to the field instance, or nullptr if the index is
     *         out-of-range.
     */
    const pivot_cache_field_t* get_field(size_t index) const;

    pivot_cache_id_t get_id() const;

    const records_type& get_all_records() const;
};

class ORCUS_SPM_DLLPUBLIC pivot_table
{
    friend class detail::debug_state_dumper_pivot_table;
    friend class pivot_collection;

    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    pivot_table(string_pool& pool);
    pivot_table(const pivot_table&) = delete;
    pivot_table(pivot_table&& other);
    ~pivot_table();

    pivot_table& operator=(const pivot_table&) = delete;
    pivot_table& operator=(pivot_table&& other);

    std::string_view get_name() const;
    void set_name(std::string_view name);
    void set_cache_id(pivot_cache_id_t cache_id);
    void set_range(const ixion::abs_rc_range_t& range);
    void set_pivot_fields(pivot_fields_t fields);
    void set_row_fields(pivot_ref_rc_fields_t fields);
    void set_column_fields(pivot_ref_rc_fields_t fields);
    void set_page_fields(pivot_ref_page_fields_t fields);
    void set_data_fields(pivot_ref_data_fields_t fields);
    void set_row_items(pivot_ref_rc_items_t items);
    void set_column_items(pivot_ref_rc_items_t items);
};

class ORCUS_SPM_DLLPUBLIC pivot_collection
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    using outdir_type = std::variant<std::string_view, std::u16string_view>;

    pivot_collection(document& doc);
    pivot_collection(const pivot_collection&) = delete;
    ~pivot_collection();

    /**
     * Insert a new pivot cache associated with a worksheet source.
     *
     * @param sheet_name name of the sheet where the source data is.
     * @param range range of the source data.  Note that the sheet indices are
     *              not used.
     * @param cache pivot cache instance to store.
     */
    void insert_worksheet_cache(
        std::string_view sheet_name, const ixion::abs_range_t& range, std::unique_ptr<pivot_cache>&& cache);

    /**
     * Insert a new pivot cache associated with a table name.
     *
     * @param table_name source table name.
     * @param cache pivot cache instance to store.
     */
    void insert_worksheet_cache(std::string_view table_name, std::unique_ptr<pivot_cache>&& cache);

    void insert_pivot_table(pivot_table pt);

    /**
     * Count the number of pivot caches currently stored.
     *
     * @return number of pivot caches currently stored in the document.
     */
    size_t get_cache_count() const;

    const pivot_cache* get_cache(
        std::string_view sheet_name, const ixion::abs_range_t& range) const;

    pivot_cache* get_cache(pivot_cache_id_t cache_id);

    const pivot_cache* get_cache(pivot_cache_id_t cache_id) const;

    void dump_debug_state(const outdir_type& outdir) const;
};

ORCUS_SPM_DLLPUBLIC std::ostream& operator<<(std::ostream& os, const pivot_cache_item_t& item);
ORCUS_SPM_DLLPUBLIC std::ostream& operator<<(std::ostream& os, const pivot_cache_record_value_t& v);
ORCUS_SPM_DLLPUBLIC std::ostream& operator<<(std::ostream& os, const pivot_item_t& v);

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
