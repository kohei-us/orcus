/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_ORCUS_TEST_GLOBAL_HPP
#define INCLUDED_ORCUS_ORCUS_TEST_GLOBAL_HPP

#include "test_global.hpp"
#include <orcus/spreadsheet/document_types.hpp>
#include <orcus/spreadsheet/auto_filter.hpp>
#include <ixion/formula_name_resolver.hpp>

#include <string>
#include <optional>
#include <set>

namespace orcus {

namespace spreadsheet {

class document;
struct font_t;
struct table_t;

}

namespace test {

std::string get_content_check(const spreadsheet::document& doc);

std::string get_content_as_csv(const spreadsheet::document& doc, spreadsheet::sheet_t sheet_index);

void verify_content(
    const char* filename, size_t line_no, const spreadsheet::document& doc, std::string_view expected);

void verify_value_to_decimals(
    const char* filename, size_t line_no, double expected, double actual, int decimals);

std::string prefix_multiline_string(std::string_view str, std::string_view prefix);

bool set(const std::optional<bool>& v);

/**
 * Check if we can consider the strikethrough attribute to be "set" given the
 * mapping we do between Excel's and Gnumeric's attribute, which is just
 * boolean, and how we internally store, which consists of 4 separate
 * attributes.
 */
bool strikethrough_set(const spreadsheet::strikethrough_t& st);

const spreadsheet::font_t* get_font(
    const spreadsheet::document& doc, spreadsheet::sheet_t sheet, spreadsheet::row_t row, spreadsheet::col_t col);

bool check_cell_text(
    const spreadsheet::document& doc, spreadsheet::sheet_t sheet, spreadsheet::row_t row, spreadsheet::col_t col,
    std::string_view expected);

const spreadsheet::format_runs_t* get_format_runs(
    const spreadsheet::document& doc, spreadsheet::sheet_t sheet, spreadsheet::row_t row, spreadsheet::col_t col);

class rc_range_resolver
{
    std::unique_ptr<ixion::formula_name_resolver> m_resolver;

public:
    explicit rc_range_resolver(ixion::formula_name_resolver_t type);

    ixion::abs_rc_range_t operator()(std::string_view addr) const;
};

class range_resolver
{
    std::unique_ptr<ixion::formula_name_resolver> m_resolver;

public:
    explicit range_resolver(ixion::formula_name_resolver_t type, const ixion::model_context& cxt);

    ixion::abs_range_t operator()(std::string_view addr) const;
};

/**
 * Stores individual filter items for a single field in an Excel document.
 *
 * Note that this can only be used when the filter items for the field is
 * imported as a filter node with filter items as its children.  Some single
 * rule item such as top10, dynamic filter and multi-match value set are
 * not imported as a filter node with children.
 */
struct excel_field_filter_items
{
    /**
     * Filter items that belong to a specified field.
     */
    std::set<spreadsheet::filter_item_t> items;

    /**
     * Connector used for the filter items within the field.
     */
    spreadsheet::auto_filter_node_op_t connector;

    bool contains(const spreadsheet::filter_item_t& expected) const;
    std::size_t size() const;

    /**
     * Fetch the filter items stored for a specified field.
     *
     * @param filter      Parent auto-filter instance.
     * @param field_index Index of the field to fetch the filter items for.
     */
    static excel_field_filter_items get(
        const spreadsheet::auto_filter_t& filter, spreadsheet::col_t field_index);
};

std::shared_ptr<const spreadsheet::table_t> get_table_from_sheet(
    const spreadsheet::document& doc, std::string_view sheet_name, std::string_view table_name);

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
