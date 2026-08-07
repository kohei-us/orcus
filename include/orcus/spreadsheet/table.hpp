/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "auto_filter.hpp"

#include <string_view>

namespace orcus { namespace spreadsheet {

/**
 * Presentation properties of a single column in a table.  The column name
 * is stored in the corresponding ixion::table_t entry in the ixion model
 * context, at the same column position.
 */
struct ORCUS_SPM_DLLPUBLIC table_column_t
{
    std::size_t identifier;
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
 * Presentation properties of a single table.  A table is a range in a
 * spreadsheet that represents a single set of data that can be used as a
 * data source.
 *
 * The core properties of the tables are stored in the ixion::table_t entry of
 * the same name inside ixion::model_context.  Both this type and
 * ixion::table_t include a member named columns; the latter stores the column
 * names.  Both members maintain the same ordering.
 */
struct ORCUS_SPM_DLLPUBLIC table_t
{
    typedef std::vector<table_column_t> columns_type;

    std::size_t identifier;

    std::string_view name;
    std::string_view display_name;

    auto_filter_t filter;
    columns_type columns;
    table_style_t style;

    table_t();
    table_t(const table_t& other) = delete;
    table_t(table_t&& other);
    ~table_t();

    table_t& operator=(const table_t& other) = delete;
    table_t& operator=(table_t&& other);

    void reset();
};

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
