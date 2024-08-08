/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "auto_filter.hpp"

#include <string_view>

#include <ixion/address.hpp>

namespace orcus { namespace spreadsheet {

/**
 * Single column entry in table.
 */
struct ORCUS_SPM_DLLPUBLIC table_column_t
{
    std::size_t identifier;
    std::string_view name;
    std::string_view totals_row_label;
    totals_row_function_t totals_row_function;

    table_column_t();
    table_column_t(const table_column_t& other);
    ~table_column_t();

    table_column_t& operator=(const table_column_t& other);

    void reset();
};

/**
 * Table style information.
 */
struct ORCUS_SPM_DLLPUBLIC table_style_t
{
    std::string_view name;

    bool show_first_column:1;
    bool show_last_column:1;
    bool show_row_stripes:1;
    bool show_column_stripes:1;

    table_style_t();
    table_style_t(const table_style_t& other);
    ~table_style_t();

    table_style_t& operator=(const table_style_t& other);

    void reset();
};

/**
 * Single table entry.  A table is a range in a spreadsheet that represents
 * a single set of data that can be used as a data source.
 */
struct ORCUS_SPM_DLLPUBLIC table_t
{
    typedef std::vector<table_column_t> columns_type;

    size_t identifier;

    std::string_view name;
    std::string_view display_name;

    ixion::abs_range_t range;

    size_t totals_row_count;

    old::auto_filter_t filter;
    columns_type columns;
    table_style_t style;

    table_t();
    table_t(const table_t& other);
    table_t(table_t&& other);
    ~table_t();

    table_t& operator=(const table_t& other);
    table_t& operator=(table_t&& other);

    void reset();
};

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
