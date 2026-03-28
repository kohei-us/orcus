/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"

#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/sheet.hpp>
#include <orcus/spreadsheet/shared_strings.hpp>

namespace ss = orcus::spreadsheet;

void test_sheet()
{
    ORCUS_TEST_FUNC_SCOPE;

    ss::range_size_t ssize{200, 10};
    ss::document doc{ssize};

    auto* sh = doc.append_sheet("Sheet 1");
    assert(sh);
    sh->set_string(2, 1, "string value");
    auto sid = sh->get_string_identifier(2, 1);
    const auto* s = doc.get_shared_strings().get_string(sid);
    assert(s);
    assert(*s == "string value");
}

int main()
{
    test_sheet();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
