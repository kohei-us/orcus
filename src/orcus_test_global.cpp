/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_test_global.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/sheet.hpp"

#include <sstream>

namespace orcus { namespace test {

assert_error::assert_error(const char* filename, size_t line_no, const char* msg)
{
    std::ostringstream os;
    os << filename << ":" << line_no << ": " << msg;
    m_msg = os.str();
}

const char* assert_error::what() const noexcept
{
    return m_msg.data();
}

std::string get_content_check(const spreadsheet::document& doc)
{
    std::ostringstream os;
    doc.dump_check(os);
    return os.str();
}

std::string get_content_as_csv(const spreadsheet::document& doc, spreadsheet::sheet_t sheet_index)
{
    const spreadsheet::sheet* sh = doc.get_sheet(sheet_index);
    if (!sh)
        return std::string();

    std::ostringstream os;
    sh->dump_csv(os);
    return os.str();
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
