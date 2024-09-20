/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_xlsx_test.hpp"

namespace ss = orcus::spreadsheet;

void test_xlsx_table_autofilter()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string_view path(SRCDIR"/test/xlsx/table/autofilter.xlsx");
    std::unique_ptr<ss::document> doc = load_doc(path);

    const ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

#if 0 // FIXME
    const ss::old::auto_filter_t* af = sh->get_auto_filter_data();
    assert(af);

    // Autofilter is over B2:C11.
    assert(af->range.first.column == 1);
    assert(af->range.first.row == 1);
    assert(af->range.last.column == 2);
    assert(af->range.last.row == 10);

    // Check the match values of the 1st column filter criterion.
    auto it = af->columns.find(0);
    assert(it != af->columns.end());

    const ss::old::auto_filter_column_t* afc = &it->second;
    assert(afc->match_values.count("A") > 0);
    assert(afc->match_values.count("C") > 0);

    // And the 2nd column.
    it = af->columns.find(1);
    assert(it != af->columns.end());
    afc = &it->second;
    assert(afc->match_values.count("1") > 0);
#endif
}

void test_xlsx_table()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string_view path(SRCDIR"/test/xlsx/table/table-1.xlsx");
    std::unique_ptr<ss::document> doc = load_doc(path);

    std::string_view name("Table1");
    const ss::table_t* p = doc->get_table(name);
    assert(p);
    assert(p->identifier == 1);
    assert(p->name == name);
    assert(p->display_name == name);
    assert(p->totals_row_count == 1);

    // Table range is C3:D9.
    ixion::abs_range_t range;
    range.first.column = 2;
    range.first.row = 2;
    range.first.sheet = 0;
    range.last.column = 3;
    range.last.row = 8;
    range.last.sheet = 0;
    assert(p->range == range);

    // Table1 has 2 table columns.
    assert(p->columns.size() == 2);

    const ss::table_column_t* tcol = &p->columns[0];
    assert(tcol);
    assert(tcol->identifier == 1);
    assert(tcol->name == "Category");
    assert(tcol->totals_row_label == "Total");
    assert(tcol->totals_row_function == ss::totals_row_function_t::none);

    tcol = &p->columns[1];
    assert(tcol);
    assert(tcol->identifier == 2);
    assert(tcol->name == "Value");
    assert(tcol->totals_row_label.empty());
    assert(tcol->totals_row_function == ss::totals_row_function_t::sum);

#if 0 // FIXME
    const auto& filter = p->filter_old;

    // Auto filter range is C3:D8.
    range.last.row = 7;
    assert(filter.range == range);

    assert(filter.columns.size() == 1);
    const ss::old::auto_filter_column_t& afc = filter.columns.begin()->second;
    assert(afc.match_values.size() == 4);
    assert(afc.match_values.count("A") > 0);
    assert(afc.match_values.count("C") > 0);
    assert(afc.match_values.count("D") > 0);
    assert(afc.match_values.count("E") > 0);
#endif

    // Check table style.
    const ss::table_style_t& style = p->style;
    assert(style.name == "TableStyleLight9");
    assert(style.show_first_column == false);
    assert(style.show_last_column == false);
    assert(style.show_row_stripes == true);
    assert(style.show_column_stripes == false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
