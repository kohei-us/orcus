/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_xlsx.hpp"
#include "orcus/pstring.hpp"
#include "orcus/global.hpp"
#include "orcus/stream.hpp"
#include "orcus/config.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/sheet.hpp"
#include "orcus/spreadsheet/auto_filter.hpp"
#include "orcus/spreadsheet/pivot.hpp"

#include <cstdlib>
#include <cassert>
#include <string>
#include <iostream>
#include <sstream>
#include <set>
#include <cmath>

#include <ixion/address.hpp>

using namespace orcus;
using namespace orcus::spreadsheet;
using namespace std;

namespace {

config test_config;

const char* dirs[] = {
    SRCDIR"/test/xlsx/raw-values-1/",
    SRCDIR"/test/xlsx/empty-shared-strings/",
};

/**
 * Semi-automated import test that goes through all specified directories,
 * and in each directory, reads the input.xlsx file, dumps its output and
 * checks it against the check.txt content.
 */
void test_xlsx_import()
{
    size_t n = sizeof(dirs)/sizeof(dirs[0]);
    for (size_t i = 0; i < n; ++i)
    {
        const char* dir = dirs[i];
        string path(dir);

        // Read the input.xlsx document.
        path.append("input.xlsx");
        spreadsheet::document doc;
        spreadsheet::import_factory factory(doc);
        orcus_xlsx app(&factory);
        app.read_file(path.c_str());

        // Dump the content of the model.
        ostringstream os;
        doc.dump_check(os);
        string check = os.str();

        // Check that against known control.
        path = dir;
        path.append("check.txt");
        string control = load_file_content(path.c_str());

        assert(!check.empty());
        assert(!control.empty());

        pstring s1(&check[0], check.size()), s2(&control[0], control.size());
        assert(s1.trim() == s2.trim());
    }
}

void test_xlsx_table_autofilter()
{
    string path(SRCDIR"/test/xlsx/table/autofilter.xlsx");
    document doc;
    import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.read_file(path.c_str());

    const sheet* sh = doc.get_sheet(0);
    assert(sh);
    const auto_filter_t* af = sh->get_auto_filter_data();
    assert(af);

    // Autofilter is over B2:C11.
    assert(af->range.first.column == 1);
    assert(af->range.first.row == 1);
    assert(af->range.last.column == 2);
    assert(af->range.last.row == 10);

    // Check the match values of the 1st column filter criterion.
    auto_filter_t::columns_type::const_iterator it = af->columns.find(0);
    assert(it != af->columns.end());

    const auto_filter_column_t* afc = &it->second;
    assert(afc->match_values.count("A") > 0);
    assert(afc->match_values.count("C") > 0);

    // And the 2nd column.
    it = af->columns.find(1);
    assert(it != af->columns.end());
    afc = &it->second;
    assert(afc->match_values.count("1") > 0);
}

void test_xlsx_table()
{
    string path(SRCDIR"/test/xlsx/table/table-1.xlsx");
    document doc;
    import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.read_file(path.c_str());

    pstring name("Table1");
    const table_t* p = doc.get_table(name);
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

    const table_column_t* tcol = &p->columns[0];
    assert(tcol);
    assert(tcol->identifier == 1);
    assert(tcol->name == "Category");
    assert(tcol->totals_row_label == "Total");
    assert(tcol->totals_row_function == totals_row_function_t::none);

    tcol = &p->columns[1];
    assert(tcol);
    assert(tcol->identifier == 2);
    assert(tcol->name == "Value");
    assert(tcol->totals_row_label.empty());
    assert(tcol->totals_row_function == totals_row_function_t::sum);

    const auto_filter_t& filter = p->filter;

    // Auto filter range is C3:D8.
    range.last.row = 7;
    assert(filter.range == range);

    assert(filter.columns.size() == 1);
    const auto_filter_column_t& afc = filter.columns.begin()->second;
    assert(afc.match_values.size() == 4);
    assert(afc.match_values.count("A") > 0);
    assert(afc.match_values.count("C") > 0);
    assert(afc.match_values.count("D") > 0);
    assert(afc.match_values.count("E") > 0);

    // Check table style.
    const table_style_t& style = p->style;
    assert(style.name == "TableStyleLight9");
    assert(style.show_first_column == false);
    assert(style.show_last_column == false);
    assert(style.show_row_stripes == true);
    assert(style.show_column_stripes == false);
}

void test_xlsx_pivot_two_pivot_caches()
{
    string path(SRCDIR"/test/xlsx/pivot-table/two-pivot-caches.xlsx");

    document doc;
    import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const pivot_collection& pc = doc.get_pivot_collection();
    assert(pc.get_cache_count() == 2);

    // B2:C6 on sheet 'Data'.
    ixion::abs_range_t range;
    range.first.column = 1;
    range.first.row = 1;
    range.last.column = 2;
    range.last.row = 5;
    const pivot_cache* cache = pc.get_cache("Data", range);
    assert(cache);
    assert(cache->get_field_count() == 2);

    // Test the content of this cache.
    const pivot_cache_field* fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "F1");

    // This field should contain 4 string items 'A', 'B', 'C' and 'D'.
    std::set<pivot_cache_item> expected =
    {
        pivot_cache_item(ORCUS_ASCII("A")),
        pivot_cache_item(ORCUS_ASCII("B")),
        pivot_cache_item(ORCUS_ASCII("C")),
        pivot_cache_item(ORCUS_ASCII("D")),
    };

    std::set<pivot_cache_item> actual(fld->items.begin(), fld->items.end());
    assert(actual == expected);

    fld = cache->get_field(1);
    assert(fld);
    assert(fld->name == "D1");

    // This is a pure numeric field with min and max values specified.
    assert(fld->min_value && *fld->min_value == 1.0);
    assert(fld->max_value && *fld->max_value == 4.0);
    assert(fld->items.empty());

    // F10:G14 on the same sheet.
    range.first.column = 5;
    range.first.row = 9;
    range.last.column = 6;
    range.last.row = 13;
    cache = pc.get_cache("Data", range);
    assert(cache);
    assert(cache->get_field_count() == 2);

    // This field should contain 4 string items 'W', 'X', 'Y' and 'Z' but not
    // necessarily in this order.
    fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "F2");

    expected =
    {
        pivot_cache_item(ORCUS_ASCII("W")),
        pivot_cache_item(ORCUS_ASCII("X")),
        pivot_cache_item(ORCUS_ASCII("Y")),
        pivot_cache_item(ORCUS_ASCII("Z")),
    };

    actual.clear();
    actual.insert(fld->items.begin(), fld->items.end());
    assert(actual == expected);

    fld = cache->get_field(1);
    assert(fld);
    assert(fld->name == "D2");

    // This is a pure numeric field with min and max values specified.
    assert(fld->min_value && *fld->min_value == 1.0);
    assert(fld->max_value && *fld->max_value == 4.0);
    assert(fld->items.empty());
}

void test_xlsx_pivot_mixed_type_field()
{
    string path(SRCDIR"/test/xlsx/pivot-table/mixed-type-field.xlsx");

    document doc;
    import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const pivot_collection& pc = doc.get_pivot_collection();
    assert(pc.get_cache_count() == 2);

    // B2:C7 on sheet 'Data'.
    ixion::abs_range_t range;
    range.first.column = 1;
    range.first.row = 1;
    range.last.column = 2;
    range.last.row = 6;
    const pivot_cache* cache = pc.get_cache("Data", range);
    assert(cache);
    assert(cache->get_field_count() == 2);

    // 1st field
    const pivot_cache_field* fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "F1");
    assert(fld->min_value && fld->min_value == 1.0);
    assert(fld->max_value && fld->max_value == 2.0);

    // This field should contain 3 string items 'A', 'B', 'C' and 2 numeric
    // items 1 and 2.
    std::set<pivot_cache_item> expected =
    {
        pivot_cache_item(ORCUS_ASCII("A")),
        pivot_cache_item(ORCUS_ASCII("B")),
        pivot_cache_item(ORCUS_ASCII("C")),
        pivot_cache_item(1.0),
        pivot_cache_item(2.0),
    };

    std::set<pivot_cache_item> actual(fld->items.begin(), fld->items.end());
    assert(actual == expected);

    // 2nd field should be a nuemric field between 1.1 and 1.5.
    fld = cache->get_field(1);
    assert(fld);
    assert(fld->name == "V1");
    assert(fld->items.empty());

    assert(fld->min_value);
    assert(std::round(*fld->min_value * 100.0) == 110.0); // min = 1.1
    assert(fld->max_value);
    assert(std::round(*fld->max_value * 100.0) == 150.0); // max = 1.5

    // B10:C17 on sheet 'Data'.
    range.first.column = 1;
    range.first.row = 9;
    range.last.column = 2;
    range.last.row = 16;
    cache = pc.get_cache("Data", range);
    assert(cache);
    assert(cache->get_field_count() == 2);

    // 1st field
    fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "F2");
    assert(fld->min_value && fld->min_value == 1.0);
    assert(fld->max_value && fld->max_value == 5.0);

    // This field should contain 3 string items 'A', 'B', 'C' and 4 numeric
    // items 1, 2, 3.5 and 5.
    expected =
    {
        pivot_cache_item(ORCUS_ASCII("A")),
        pivot_cache_item(ORCUS_ASCII("B")),
        pivot_cache_item(ORCUS_ASCII("C")),
        pivot_cache_item(1.0),
        pivot_cache_item(2.0),
        pivot_cache_item(3.5),
        pivot_cache_item(5.0),
    };

    actual.clear();
    actual.insert(fld->items.begin(), fld->items.end());
    assert(actual == expected);

    // 2nd field
    fld = cache->get_field(1);
    assert(fld);
    assert(fld->name == "V2");
    assert(fld->items.empty());

    assert(fld->min_value);
    assert(std::round(*fld->min_value * 100.0) == 110.0); // min = 1.1
    assert(fld->max_value);
    assert(std::round(*fld->max_value * 100.0) == 220.0); // max = 2.2
}

}

int main()
{
    test_config.debug = true;
    test_config.structure_check = true;

    test_xlsx_import();
    test_xlsx_table_autofilter();
    test_xlsx_table();
    test_xlsx_pivot_two_pivot_caches();
    test_xlsx_pivot_mixed_type_field();

    return EXIT_SUCCESS;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
