/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "orcus_test_global.hpp"
#include "filesystem_env.hpp"

#include <orcus/orcus_gnumeric.hpp>
#include <orcus/format_detection.hpp>
#include <orcus/stream.hpp>
#include <orcus/spreadsheet/factory.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/sheet.hpp>
#include <orcus/spreadsheet/auto_filter.hpp>
#include <orcus/spreadsheet/shared_strings.hpp>
#include <orcus/spreadsheet/styles.hpp>
#include <orcus/types.hpp>

#include <ixion/address.hpp>
#include <ixion/model_context.hpp>
#include <iostream>
#include <sstream>

std::unique_ptr<orcus::spreadsheet::document> load_doc(const fs::path& filepath);

void test_gnumeric_detection();
void test_gnumeric_create_filter();
void test_gnumeric_import();
void test_gnumeric_column_widths_row_heights();
void test_gnumeric_hidden_rows_columns();
void test_gnumeric_merged_cells();
void test_gnumeric_text_alignment();
void test_gnumeric_cell_properties_wrap_and_shrink();
void test_gnumeric_background_fill();
void test_gnumeric_colored_text();
void test_gnumeric_text_formats_basic();
void test_gnumeric_text_formats_underline();
void test_gnumeric_cell_borders_single_cells();
void test_gnumeric_cell_borders_directions();
void test_gnumeric_cell_borders_colors();
void test_gnumeric_number_format();

void test_gnumeric_auto_filter_multi_rules();
void test_gnumeric_auto_filter_number();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
