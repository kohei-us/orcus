/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/spreadsheet/auto_filter.hpp"

namespace orcus { namespace spreadsheet {

auto_filter_column_t::auto_filter_column_t() = default;
auto_filter_column_t::auto_filter_column_t(const auto_filter_column_t& other) = default;
auto_filter_column_t::auto_filter_column_t(auto_filter_column_t&& other) = default;
auto_filter_column_t::~auto_filter_column_t() = default;

auto_filter_column_t& auto_filter_column_t::operator=(const auto_filter_column_t& other) = default;
auto_filter_column_t& auto_filter_column_t::operator=(auto_filter_column_t&& other) = default;

void auto_filter_column_t::reset()
{
    match_values.clear();
}

void auto_filter_column_t::swap(auto_filter_column_t& r)
{
    match_values.swap(r.match_values);
}

auto_filter_t::auto_filter_t() : range(ixion::abs_range_t::invalid) {}
auto_filter_t::auto_filter_t(const auto_filter_t& other) = default;
auto_filter_t::auto_filter_t(auto_filter_t&& other) = default;
auto_filter_t::~auto_filter_t() = default;

auto_filter_t& auto_filter_t::operator=(const auto_filter_t& other) = default;
auto_filter_t& auto_filter_t::operator=(auto_filter_t&& other) = default;

void auto_filter_t::reset()
{
    range = ixion::abs_range_t(ixion::abs_range_t::invalid);
    columns.clear();
}

void auto_filter_t::swap(auto_filter_t& r)
{
    std::swap(range, r.range);
    columns.swap(r.columns);
}

void auto_filter_t::commit_column(col_t col, auto_filter_column_t data)
{
    if (col < 0)
        // Invalid column index.  Nothing happens.
        return;

    columns.insert_or_assign(col, std::move(data));
}

table_column_t::table_column_t() : identifier(0), totals_row_function(totals_row_function_t::none) {}

table_column_t::table_column_t(const table_column_t& other) = default;
table_column_t::~table_column_t() = default;

table_column_t& table_column_t::operator=(const table_column_t& other) = default;

void table_column_t::reset()
{
    identifier = 0;
    name = std::string_view{};
    totals_row_label = std::string_view{};
    totals_row_function = totals_row_function_t::none;
}

table_style_t::table_style_t() :
    show_first_column(false),
    show_last_column(false),
    show_row_stripes(false),
    show_column_stripes(false) {}

table_style_t::table_style_t(const table_style_t& other) = default;
table_style_t::~table_style_t() = default;

table_style_t& table_style_t::operator=(const table_style_t& other) = default;

void table_style_t::reset()
{
    name = std::string_view{};
    show_first_column = false;
    show_last_column = false;
    show_row_stripes = false;
    show_column_stripes = false;
}

table_t::table_t() : identifier(0), range(ixion::abs_range_t::invalid), totals_row_count(0) {}
table_t::table_t(const table_t& other) = default;
table_t::table_t(table_t&& other) = default;
table_t::~table_t() = default;

table_t& table_t::operator=(const table_t& other) = default;
table_t& table_t::operator=(table_t&& other) = default;

void table_t::reset()
{
    identifier = 0;
    name = std::string_view{};
    display_name = std::string_view{};
    range = ixion::abs_range_t(ixion::abs_range_t::invalid);
    totals_row_count = 0;
    filter.reset();
    columns.clear();
    style.reset();
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
