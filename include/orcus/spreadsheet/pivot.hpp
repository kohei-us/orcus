/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_PIVOT_HPP
#define INCLUDED_ORCUS_SPREADSHEET_PIVOT_HPP

#include "orcus/env.hpp"
#include "orcus/pstring.hpp"

#include <memory>
#include <vector>

#include <boost/optional.hpp>

namespace ixion {

struct abs_range_t;

}

namespace orcus {

class string_pool;

namespace spreadsheet {

class document;

enum class pivot_cache_item_t
{
    unknown = 0, boolean, datetime, string, numeric, blank, error
};

struct ORCUS_SPM_DLLPUBLIC pivot_cache_item
{
    pivot_cache_item_t type;

    union
    {
        struct
        {
            // This must point to an interned string instance. May to be
            // null-terminated.
            const char* p;

            size_t n; // Length of the string value.

        } string;

        double numeric;
        bool boolean;

        // TODO : probably more to add, esp for datetime and error values.

    } value;

    pivot_cache_item();
    pivot_cache_item(const char* p_str, size_t n_str);
    pivot_cache_item(double _numeric);
    pivot_cache_item(bool _boolean);

    pivot_cache_item(const pivot_cache_item& other);
    pivot_cache_item(pivot_cache_item&& other);

    bool operator< (const pivot_cache_item& other) const;
    bool operator== (const pivot_cache_item& other) const;
};

struct ORCUS_SPM_DLLPUBLIC pivot_cache_field
{
    using items_type = std::vector<pivot_cache_item>;

    /**
     * Field name. It must be interned with the string pool belonging to the
     * document.
     */
    pstring name;

    items_type items;

    boost::optional<double> min_value;
    boost::optional<double> max_value;

    pivot_cache_field();
    pivot_cache_field(const pstring& _name);
    pivot_cache_field(const pivot_cache_field& other);
    pivot_cache_field(pivot_cache_field&& other);
};

class ORCUS_SPM_DLLPUBLIC pivot_cache
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    using fields_type = std::vector<pivot_cache_field>;

    pivot_cache(string_pool& sp);
    ~pivot_cache();

    /**
     * Bulk-insert all the fields in one step. Note that this will replace any
     * pre-existing fields if any.
     *
     * @param fields field instances to move into storage.
     */
    void insert_fields(fields_type fields);

    size_t get_field_count() const;

    /**
     * Retrieve a field data by its index.
     *
     * @param index index of the field to retrieve.
     *
     * @return pointer to the field instance, or nullptr if the index is
     *         out-of-range.
     */
    const pivot_cache_field* get_field(size_t index) const;
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
     */
    void insert_worksheet_cache(
        const pstring& sheet_name, const ixion::abs_range_t& range, std::unique_ptr<pivot_cache>&& cache);

    /**
     * Count the number of pivot caches currently stored.
     *
     * @return number of pivot caches currently stored in the document.
     */
    size_t get_cache_count() const;

    const pivot_cache* get_cache(
        const pstring& sheet_name, const ixion::abs_range_t& range) const;
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
