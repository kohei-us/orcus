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

namespace spreadsheet {

class document;

}

namespace test {

class assert_error : public std::exception
{
    std::string m_msg;

public:
    assert_error(const char* file_name, size_t line_no, const char* msg);

    virtual const char* what() const noexcept override;
};

std::string get_content_check(const spreadsheet::document& doc);

std::string get_content_as_csv(const spreadsheet::document& doc, spreadsheet::sheet_t sheet_index);

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
