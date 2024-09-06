/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_xls_xml_test.hpp"

#include <orcus/spreadsheet/auto_filter.hpp>

namespace ss = orcus::spreadsheet;

void test_xls_xml_auto_filter_number()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/table/autofilter-number.xml");
    assert(doc);

    auto* sh = doc->get_sheet("Greater Than");
    assert(sh);

    auto* filter = sh->get_auto_filter_range();
    assert(filter);

    // R3C2:R96C7
    ixion::abs_rc_range_t range_expected;
    range_expected.first.row = 2;
    range_expected.first.column = 1;
    range_expected.last.row = 95;
    range_expected.last.column = 6;
    assert(filter->range == range_expected);

    // TODO : continue on...
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
