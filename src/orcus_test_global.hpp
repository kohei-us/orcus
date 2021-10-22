/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_ORCUS_TEST_GLOBAL_HPP
#define INCLUDED_ORCUS_ORCUS_TEST_GLOBAL_HPP

#include "test_global.hpp"
#include <orcus/spreadsheet/types.hpp>

#include <string>

namespace orcus {

namespace spreadsheet {

class document;

}

namespace test {

std::string get_content_check(const spreadsheet::document& doc);

std::string get_content_as_csv(const spreadsheet::document& doc, spreadsheet::sheet_t sheet_index);

void verify_content(
    const char* filename, size_t line_no, const spreadsheet::document& doc, std::string_view expected);

void verify_value_to_decimals(
    const char* filename, size_t line_no, double expected, double actual, int decimals);

std::string prefix_multiline_string(std::string_view str, std::string_view prefix);

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
