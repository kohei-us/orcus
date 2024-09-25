/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "orcus_test_global.hpp"

#include <orcus/orcus_xlsx.hpp>
#include <orcus/format_detection.hpp>
#include <orcus/stream.hpp>
#include <orcus/config.hpp>
#include <orcus/spreadsheet/factory.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/view.hpp>
#include <orcus/spreadsheet/sheet.hpp>
#include <orcus/spreadsheet/table.hpp>
#include <orcus/spreadsheet/pivot.hpp>
#include <orcus/spreadsheet/styles.hpp>
#include <orcus/spreadsheet/shared_strings.hpp>
#include <orcus/parser_global.hpp>

#include <ixion/model_context.hpp>
#include <ixion/address.hpp>
#include <ixion/formula_name_resolver.hpp>

std::unique_ptr<orcus::spreadsheet::document> load_doc(std::string_view path, bool recalc = true);

void test_xlsx_detection();
void test_xlsx_create_filter();
void test_xlsx_import();
void test_xlsx_merged_cells();
void test_xlsx_date_time();
void test_xlsx_background_fill();
void test_xlsx_number_format();
void test_xlsx_text_alignment();
void test_xlsx_cell_borders_single_cells();
void test_xlsx_cell_borders_directions();
void test_xlsx_cell_borders_colors();
void test_xlsx_hidden_rows_columns();
void test_xlsx_cell_properties();
void test_xlsx_styles_direct_format();
void test_xlsx_styles_column_styles();
void test_xlsx_formatted_text_basic();
void test_xlsx_formatted_text_underline();

// pivot table
void test_xlsx_pivot_two_pivot_caches();
void test_xlsx_pivot_mixed_type_field();
void test_xlsx_pivot_group_field();
void test_xlsx_pivot_group_by_numbers();
void test_xlsx_pivot_group_by_dates();
void test_xlsx_pivot_error_values();

// table / auto filter
void test_xlsx_table_autofilter();
void test_xlsx_table_autofilter_basic_number();
void test_xlsx_table_autofilter_basic_text();
void test_xlsx_table();

// view import
void test_xlsx_view_cursor_per_sheet();
void test_xlsx_view_cursor_split_pane();
void test_xlsx_view_frozen_pane();

// document structure
void test_xlsx_doc_structure_unordered_sheet_positions();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
