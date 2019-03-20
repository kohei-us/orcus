/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_TEST_GLOBAL_HPP
#define INCLUDED_ORCUS_TEST_GLOBAL_HPP

#include "orcus/spreadsheet/types.hpp"

#include <string>

namespace orcus {

class pstring;

namespace spreadsheet {

class document;

}

namespace test {

class assert_error : public std::exception
{
    std::string m_msg;

public:
    assert_error(const char* filename, size_t line_no, const char* msg);

    virtual const char* what() const noexcept override;
};

std::string get_content_check(const spreadsheet::document& doc);

std::string get_content_as_csv(const spreadsheet::document& doc, spreadsheet::sheet_t sheet_index);

/**
 * Verify the content of a document against a known control.  Both string
 * values passed to this function must be in the content-check format.
 *
 * @param expected string representative of the expected content.
 * @param actual string representative of the actual content.
 */
void verify_content(
    const char* filename, size_t line_no, const pstring& expected, const std::string& actual);

void verify_value_to_decimals(
    const char* filename, size_t line_no, double expected, double actual, int decimals);

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
