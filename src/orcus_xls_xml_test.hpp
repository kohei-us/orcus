/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "orcus_test_global.hpp"
#include "filesystem_env.hpp"

#include <orcus/orcus_xls_xml.hpp>
#include <orcus/format_detection.hpp>
#include <orcus/stream.hpp>
#include <orcus/config.hpp>
#include <orcus/parser_global.hpp>
#include <orcus/yaml_document_tree.hpp>
#include <orcus/spreadsheet/factory.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/view.hpp>
#include <orcus/spreadsheet/sheet.hpp>
#include <orcus/spreadsheet/shared_strings.hpp>
#include <orcus/spreadsheet/styles.hpp>
#include <orcus/spreadsheet/config.hpp>

#include <ixion/model_context.hpp>
#include <ixion/address.hpp>
#include <ixion/cell.hpp>

orcus::config get_test_config();

std::unique_ptr<orcus::spreadsheet::document> load_doc_from_filepath(
    const std::string& path,
    bool recalc=true,
    orcus::spreadsheet::formula_error_policy_t error_policy=orcus::spreadsheet::formula_error_policy_t::fail
);

std::unique_ptr<orcus::spreadsheet::document> load_doc_from_stream(const std::string& path);

void test_xls_xml_detection();
void test_xls_xml_create_filter();
void test_xls_xml_import();
void test_xls_xml_merged_cells();
void test_xls_xml_date_time();
void test_xls_xml_bold_and_italic();
void test_xls_xml_colored_text();
void test_xls_xml_formatted_text_basic();
void test_xls_xml_formatted_text_underline();
void test_xls_xml_column_width_row_height();
void test_xls_xml_background_fill();
void test_xls_xml_named_colors();
void test_xls_xml_text_alignment();
void test_xls_xml_cell_borders_single_cells();
void test_xls_xml_cell_borders_directions();
void test_xls_xml_cell_borders_colors();
void test_xls_xml_hidden_rows_columns();
void test_xls_xml_character_set();
void test_xls_xml_number_format();
void test_xls_xml_cell_properties_wrap_and_shrink();
void test_xls_xml_cell_properties_default_style();
void test_xls_xml_cell_properties_locked_and_hidden();
void test_xls_xml_styles_direct_format();
void test_xls_xml_styles_column_styles();
void test_xls_xml_styles_data_offset();

// view import
void test_xls_xml_view_cursor_per_sheet();
void test_xls_xml_view_cursor_split_pane();
void test_xls_xml_view_frozen_pane();

void test_xls_xml_skip_error_cells();
void test_xls_xml_double_bom();

void test_xls_xml_auto_filter_number();
void test_xls_xml_auto_filter_text();
void test_xls_xml_auto_filter_wildcard();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
