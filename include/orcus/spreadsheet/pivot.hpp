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

namespace ixion {

struct abs_range_t;

}

namespace orcus {

class string_pool;

namespace spreadsheet {

class document;

struct ORCUS_SPM_DLLPUBLIC pivot_cache_field
{
    /**
     * Field name. It must be interned with the string pool belonging to the
     * document.
     */
    pstring name;

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
