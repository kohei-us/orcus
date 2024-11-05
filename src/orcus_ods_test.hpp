/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "orcus_test_global.hpp"
#include <orcus/orcus_ods.hpp>
#include <orcus/format_detection.hpp>
#include <orcus/stream.hpp>
#include <orcus/parser_global.hpp>
#include <orcus/spreadsheet/factory.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/sheet.hpp>
#include <orcus/spreadsheet/shared_strings.hpp>
#include <orcus/spreadsheet/styles.hpp>

#include <cstdlib>
#include <cassert>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#include "filesystem_env.hpp"

std::unique_ptr<orcus::spreadsheet::document> load_doc(const fs::path& filepath);

void test_ods_detection();
void test_ods_create_filter();

void test_ods_import_cell_values();
void test_ods_import_column_widths_row_heights();
void test_ods_import_formatted_text();
void test_ods_import_number_formats();
void test_ods_import_cell_properties();
void test_ods_import_styles_direct_format();
void test_ods_import_styles_column_styles();
void test_ods_import_styles_asian_complex();
void test_ods_import_styles_text_underlines();

void test_ods_autofilter_multi_conditions();
void test_ods_autofilter_text_comparisons();
void test_ods_autofilter_largest_smallest();


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
