/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_AUTO_FILTER_HPP
#define INCLUDED_ORCUS_SPREADSHEET_AUTO_FILTER_HPP

#include "types.hpp"
#include "../env.hpp"

#include <map>
#include <unordered_set>

#include <ixion/address.hpp>

namespace orcus { namespace spreadsheet {

/**
 * Data for a single column inside autofilter range.
 */
struct ORCUS_SPM_DLLPUBLIC auto_filter_column_t
{
    using match_values_type = std::unordered_set<std::string_view>;
    match_values_type match_values;

    auto_filter_column_t();
    auto_filter_column_t(const auto_filter_column_t& other);
    auto_filter_column_t(auto_filter_column_t&& other);
    ~auto_filter_column_t();

    auto_filter_column_t& operator=(const auto_filter_column_t& other);
    auto_filter_column_t& operator=(auto_filter_column_t&& other);

    void reset();
    void swap(auto_filter_column_t& r);
};

/**
 * Data for a single autofilter entry.  An autofilter can belong to either a
 * sheet or a table.
 */
struct ORCUS_SPM_DLLPUBLIC auto_filter_t
{
    typedef std::map<col_t, auto_filter_column_t> columns_type;

    ixion::abs_range_t range;

    columns_type columns;

    auto_filter_t();
    auto_filter_t(const auto_filter_t& other);
    auto_filter_t(auto_filter_t&& other);
    ~auto_filter_t();

    auto_filter_t& operator=(const auto_filter_t& other);
    auto_filter_t& operator=(auto_filter_t&& other);

    void reset();
    void swap(auto_filter_t& r);

    /**
     * Set column data to specified column index.
     *
     * @param col column index to associate the data to.
     * @param data column data.
     */
    void commit_column(col_t col, auto_filter_column_t data);
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
