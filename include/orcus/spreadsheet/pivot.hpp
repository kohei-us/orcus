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

namespace ixion {

struct abs_range_t;

}

namespace orcus {

class string_pool;

namespace spreadsheet {

class document;

using pivot_cache_indices_t = std::vector<size_t>;

struct ORCUS_SPM_DLLPUBLIC pivot_cache_record_value_t
{
    using value_type = std::variant<bool, double, std::size_t, std::string_view, date_time_t>;

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

        range_grouping_type() = default;
        range_grouping_type(const range_grouping_type& other) = default;
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
};

class ORCUS_SPM_DLLPUBLIC pivot_cache
{
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

class ORCUS_SPM_DLLPUBLIC pivot_collection
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    pivot_collection(document& doc);
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
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
