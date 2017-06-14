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
#include "orcus/spreadsheet/view.hpp"
#include "orcus/spreadsheet/sheet.hpp"
#include "orcus/spreadsheet/auto_filter.hpp"
#include "orcus/spreadsheet/pivot.hpp"

#include <cstdlib>
#include <cassert>
#include <string>
#include <sstream>
#include <set>
#include <cmath>
#include <vector>
#include <iostream>

#include <ixion/address.hpp>
#include <ixion/formula_name_resolver.hpp>

using namespace orcus;
using namespace orcus::spreadsheet;
using namespace std;

namespace {

/**
 * Convenience function to retrieve a pivot cache instance from textural
 * sheet name and range name.
 */
const pivot_cache* get_pivot_cache(
    const pivot_collection& pc, const pstring& sheet_name, const pstring& range_name)
{
    std::unique_ptr<ixion::formula_name_resolver> resolver =
        ixion::formula_name_resolver::get(
            ixion::formula_name_resolver_t::excel_a1, nullptr);

    if (!resolver)
        return nullptr;

    ixion::abs_address_t origin(0,0,0);

    ixion::formula_name_t fn =
        resolver->resolve(range_name.get(), range_name.size(), origin);

    if (fn.type != ixion::formula_name_t::range_reference)
        return nullptr;

    ixion::abs_range_t range = ixion::to_range(fn.range).to_abs(origin);
    return pc.get_cache(sheet_name, range);
}

config test_config;

vector<const char*> dirs = {
    SRCDIR"/test/xlsx/raw-values-1/",
    SRCDIR"/test/xlsx/empty-shared-strings/",
    SRCDIR"/test/xlsx/named-expression/",
    SRCDIR"/test/xlsx/named-expression-sheet-local/",
};

/**
 * Semi-automated import test that goes through all specified directories,
 * and in each directory, reads the input.xlsx file, dumps its output and
 * checks it against the check.txt content.
 */
void test_xlsx_import()
{
    for (const char* dir : dirs)
    {
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

void test_xlsx_merged_cells()
{
    string path(SRCDIR"/test/xlsx/merged-cells/simple.xlsx");

    document doc;
    import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const sheet* sheet1 = doc.get_sheet("Sheet1");
    assert(sheet1);

    spreadsheet::range_t merge_range = sheet1->get_merge_cell_range(0, 1);
    assert(merge_range.first.column == 1);
    assert(merge_range.last.column == 2);
    assert(merge_range.first.row == 0);
    assert(merge_range.last.row == 0);

    merge_range = sheet1->get_merge_cell_range(0, 3);
    assert(merge_range.first.column == 3);
    assert(merge_range.last.column == 5);
    assert(merge_range.first.row == 0);
    assert(merge_range.last.row == 0);

    merge_range = sheet1->get_merge_cell_range(1, 0);
    assert(merge_range.first.column == 0);
    assert(merge_range.last.column == 0);
    assert(merge_range.first.row == 1);
    assert(merge_range.last.row == 2);

    merge_range = sheet1->get_merge_cell_range(3, 0);
    assert(merge_range.first.column == 0);
    assert(merge_range.last.column == 0);
    assert(merge_range.first.row == 3);
    assert(merge_range.last.row == 5);

    merge_range = sheet1->get_merge_cell_range(2, 2);
    assert(merge_range.first.column == 2);
    assert(merge_range.last.column == 5);
    assert(merge_range.first.row == 2);
    assert(merge_range.last.row == 5);
}

void test_xlsx_date_time()
{
    string path(SRCDIR"/test/xlsx/date-time/input.xlsx");

    document doc;
    import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const sheet* sheet1 = doc.get_sheet("Sheet1");
    assert(sheet1);

    // B1 contains date-only value.
    date_time_t dt = sheet1->get_date_time(0, 1);
    assert(dt == date_time_t(2016, 12, 14));

    // B2 contains date-time value with no fraction seconds.
    dt = sheet1->get_date_time(1, 1);
    assert(dt == date_time_t(2002, 2, 3, 12, 34, 45));

    // B3 contains date-time value with fraction second (1992-03-04 08:34:33.555)
    dt = sheet1->get_date_time(2, 1);
    assert(dt.year == 1992);
    assert(dt.month == 3);
    assert(dt.day == 4);
    assert(dt.hour == 8);
    assert(dt.minute == 34);
    assert(std::floor(dt.second) == 33.0);

    // Evalutate the fraction second as milliseconds.
    double ms = dt.second * 1000.0;
    ms -= std::floor(dt.second) * 1000.0;
    ms = std::round(ms);
    assert(ms == 555.0);
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
    const pivot_cache* cache = get_pivot_cache(pc, "Data", "B2:C6");
    assert(cache);
    assert(cache->get_field_count() == 2);

    // Test the content of this cache.
    const pivot_cache_field_t* fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "F1");

    // This field should contain 4 string items 'A', 'B', 'C' and 'D'.
    std::set<pivot_cache_item_t> expected =
    {
        pivot_cache_item_t(ORCUS_ASCII("A")),
        pivot_cache_item_t(ORCUS_ASCII("B")),
        pivot_cache_item_t(ORCUS_ASCII("C")),
        pivot_cache_item_t(ORCUS_ASCII("D")),
    };

    std::set<pivot_cache_item_t> actual(fld->items.begin(), fld->items.end());
    assert(actual == expected);

    fld = cache->get_field(1);
    assert(fld);
    assert(fld->name == "D1");

    // This is a pure numeric field with min and max values specified.
    assert(fld->min_value && *fld->min_value == 1.0);
    assert(fld->max_value && *fld->max_value == 4.0);
    assert(fld->items.empty());

    {
        // Check the records.
        pivot_cache::records_type expected =
        {
            { pivot_cache_record_value_t(size_t(0)), pivot_cache_record_value_t(1.0) },
            { pivot_cache_record_value_t(size_t(1)), pivot_cache_record_value_t(2.0) },
            { pivot_cache_record_value_t(size_t(2)), pivot_cache_record_value_t(3.0) },
            { pivot_cache_record_value_t(size_t(3)), pivot_cache_record_value_t(4.0) },
        };

        assert(expected == cache->get_all_records());
    }

    // F10:G14 on the same sheet.
    cache = get_pivot_cache(pc, "Data", "F10:G14");
    assert(cache);
    assert(cache->get_field_count() == 2);

    // This field should contain 4 string items 'W', 'X', 'Y' and 'Z' but not
    // necessarily in this order.
    fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "F2");

    expected =
    {
        pivot_cache_item_t(ORCUS_ASCII("W")),
        pivot_cache_item_t(ORCUS_ASCII("X")),
        pivot_cache_item_t(ORCUS_ASCII("Y")),
        pivot_cache_item_t(ORCUS_ASCII("Z")),
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

    {
        // Check the records.
        pivot_cache::records_type expected =
        {
            { pivot_cache_record_value_t(size_t(0)), pivot_cache_record_value_t(4.0) },
            { pivot_cache_record_value_t(size_t(1)), pivot_cache_record_value_t(3.0) },
            { pivot_cache_record_value_t(size_t(2)), pivot_cache_record_value_t(2.0) },
            { pivot_cache_record_value_t(size_t(3)), pivot_cache_record_value_t(1.0) },
        };

        assert(expected == cache->get_all_records());
    }
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
    const pivot_cache* cache = get_pivot_cache(pc, "Data", "B2:C7");
    assert(cache);
    assert(cache->get_field_count() == 2);

    // 1st field
    const pivot_cache_field_t* fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "F1");
    assert(fld->min_value && fld->min_value == 1.0);
    assert(fld->max_value && fld->max_value == 2.0);

    // This field should contain 3 string items 'A', 'B', 'C' and 2 numeric
    // items 1 and 2.
    std::set<pivot_cache_item_t> expected =
    {
        pivot_cache_item_t(ORCUS_ASCII("A")),
        pivot_cache_item_t(ORCUS_ASCII("B")),
        pivot_cache_item_t(ORCUS_ASCII("C")),
        pivot_cache_item_t(1.0),
        pivot_cache_item_t(2.0),
    };

    std::set<pivot_cache_item_t> actual(fld->items.begin(), fld->items.end());
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

    {
        // Check the records.
        pivot_cache::records_type expected =
        {
            { pivot_cache_record_value_t(size_t(0)), pivot_cache_record_value_t(1.1) },
            { pivot_cache_record_value_t(size_t(1)), pivot_cache_record_value_t(1.2) },
            { pivot_cache_record_value_t(size_t(2)), pivot_cache_record_value_t(1.3) },
            { pivot_cache_record_value_t(size_t(3)), pivot_cache_record_value_t(1.4) },
            { pivot_cache_record_value_t(size_t(4)), pivot_cache_record_value_t(1.5) },
        };

        assert(expected == cache->get_all_records());
    }

    // B10:C17 on sheet 'Data'.
    cache = get_pivot_cache(pc, "Data", "B10:C17");
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
        pivot_cache_item_t(ORCUS_ASCII("A")),
        pivot_cache_item_t(ORCUS_ASCII("B")),
        pivot_cache_item_t(ORCUS_ASCII("C")),
        pivot_cache_item_t(1.0),
        pivot_cache_item_t(2.0),
        pivot_cache_item_t(3.5),
        pivot_cache_item_t(5.0),
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

    {
        // Check the records.
        pivot_cache::records_type expected =
        {
            { pivot_cache_record_value_t(size_t(0)), pivot_cache_record_value_t(1.1) },
            { pivot_cache_record_value_t(size_t(1)), pivot_cache_record_value_t(1.2) },
            { pivot_cache_record_value_t(size_t(2)), pivot_cache_record_value_t(1.3) },
            { pivot_cache_record_value_t(size_t(3)), pivot_cache_record_value_t(1.4) },
            { pivot_cache_record_value_t(size_t(4)), pivot_cache_record_value_t(1.5) },
            { pivot_cache_record_value_t(size_t(5)), pivot_cache_record_value_t(1.8) },
            { pivot_cache_record_value_t(size_t(6)), pivot_cache_record_value_t(2.2) },
        };

        assert(expected == cache->get_all_records());
    }
}

void test_xlsx_pivot_group_field()
{
    string path(SRCDIR"/test/xlsx/pivot-table/group-field.xlsx");

    document doc;
    import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const pivot_collection& pc = doc.get_pivot_collection();
    assert(pc.get_cache_count() == 1);

    // B2:C6 on sheet 'Sheet1'.
    const pivot_cache* cache = get_pivot_cache(pc, "Sheet1", "B2:C6");
    assert(cache);
    assert(cache->get_field_count() == 3);

    // First field is labeled 'Key'.
    const pivot_cache_field_t* fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "Key");

    // This field should contain 4 string items 'A', 'B', 'C' and 'D'.
    std::set<pivot_cache_item_t> expected =
    {
        pivot_cache_item_t(ORCUS_ASCII("A")),
        pivot_cache_item_t(ORCUS_ASCII("B")),
        pivot_cache_item_t(ORCUS_ASCII("C")),
        pivot_cache_item_t(ORCUS_ASCII("D")),
    };

    std::set<pivot_cache_item_t> actual(fld->items.begin(), fld->items.end());
    assert(actual == expected);

    // 2nd field is 'Value' and is a numeric field.
    fld = cache->get_field(1);
    assert(fld);
    assert(fld->name == "Value");
    assert(fld->items.empty());

    assert(fld->min_value);
    assert(*fld->min_value == 1.0);
    assert(fld->max_value);
    assert(*fld->max_value == 4.0);

    // 3rd field is a group field labeled 'Key2'.
    fld = cache->get_field(2);
    assert(fld);
    assert(fld->name == "Key2");
    assert(fld->items.empty());

    const pivot_cache_group_data_t* gd = fld->group_data.get();
    assert(gd);
    assert(gd->base_field == 0);
    assert(gd->items.size() == 2);

    // It should have two items - Group1 and Group2.
    expected =
    {
        pivot_cache_item_t(ORCUS_ASCII("Group1")),
        pivot_cache_item_t(ORCUS_ASCII("Group2")),
    };

    actual.clear();
    actual.insert(gd->items.begin(), gd->items.end());
    assert(actual == expected);

    // Group1 should group 'A' and 'B' from the 1st field, and Group2 should
    // group 'C' and 'D'.

    pivot_cache_indices_t expected_group = { 0, 0, 1, 1 };
    assert(gd->base_to_group_indices == expected_group);

    {
        // Check the records.
        pivot_cache::records_type expected =
        {
            { pivot_cache_record_value_t(size_t(0)), pivot_cache_record_value_t(1.0) },
            { pivot_cache_record_value_t(size_t(1)), pivot_cache_record_value_t(2.0) },
            { pivot_cache_record_value_t(size_t(2)), pivot_cache_record_value_t(3.0) },
            { pivot_cache_record_value_t(size_t(3)), pivot_cache_record_value_t(4.0) },
        };

        assert(expected == cache->get_all_records());
    }
}

void test_xlsx_pivot_group_by_numbers()
{
    string path(SRCDIR"/test/xlsx/pivot-table/group-by-numbers.xlsx");

    document doc;
    import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const pivot_collection& pc = doc.get_pivot_collection();
    assert(pc.get_cache_count() == 1);

    // B2:C13 on sheet 'Sheet1'.
    const pivot_cache* cache = get_pivot_cache(pc, "Sheet1", "B2:C13");
    assert(cache);
    assert(cache->get_field_count() == 2);

    // First field is a field with numeric grouping with intervals.
    const pivot_cache_field_t* fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "V1");

    // There should be 11 raw values ranging from 9.78E-2 to 9.82.
    assert(fld->items.size() == 11);
    assert(fld->min_value);
    assert(fld->max_value);
    assert(std::round(*fld->min_value*10000.0) == 978.00); // 9.78E-2
    assert(std::round(*fld->max_value*100.0) == 982.00);   // 9.82

    // We'll just make sure that all 11 items are of numeric type.

    for (const pivot_cache_item_t& item : fld->items)
    {
        assert(item.type == pivot_cache_item_t::item_type::numeric);
        assert(*fld->min_value <= item.value.numeric);
        assert(item.value.numeric <= *fld->max_value);
    }

    // This field is also gruop field with 7 numeric intervals of width 2.
    assert(fld->group_data);
    const pivot_cache_group_data_t& grp = *fld->group_data;
    assert(grp.items.size() == 7);

    pivot_cache_items_t expected =
    {
        pivot_cache_item_t(ORCUS_ASCII("<0")),
        pivot_cache_item_t(ORCUS_ASCII("0-2")),
        pivot_cache_item_t(ORCUS_ASCII("2-4")),
        pivot_cache_item_t(ORCUS_ASCII("4-6")),
        pivot_cache_item_t(ORCUS_ASCII("6-8")),
        pivot_cache_item_t(ORCUS_ASCII("8-10")),
        pivot_cache_item_t(ORCUS_ASCII(">10")),
    };

    assert(grp.items == expected);

    // Check the numeric range properties.
    assert(grp.range_grouping);
    assert(grp.range_grouping->group_by == pivot_cache_group_by_t::range);
    assert(!grp.range_grouping->auto_start);
    assert(!grp.range_grouping->auto_end);
    assert(grp.range_grouping->start == 0.0);
    assert(grp.range_grouping->end == 10.0);
    assert(grp.range_grouping->interval == 2.0);

    // Test the 2nd field. This field is purely a numeric field with no
    // discrete items.

    fld = cache->get_field(1);
    assert(fld);
    assert(fld->name == "V2");
    assert(!fld->group_data);
    assert(fld->items.empty());
    assert(fld->min_value);
    assert(fld->min_value == 1.0);
    assert(fld->max_value);
    assert(fld->max_value == 11.0);
}

void test_xlsx_pivot_group_by_dates()
{
    string path(SRCDIR"/test/xlsx/pivot-table/group-by-dates.xlsx");

    document doc;
    import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const pivot_collection& pc = doc.get_pivot_collection();
    assert(pc.get_cache_count() == 1);

    const pivot_cache* cache = get_pivot_cache(pc, "Sheet1", "B2:C14");
    assert(cache);

    // First field is a date field.
    const pivot_cache_field_t* fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "Date");

    // Minimum and maximum date values.
    assert(fld->min_date);
    assert(*fld->min_date == date_time_t(2014, 1, 1));
    assert(fld->max_date);
    assert(*fld->max_date == date_time_t(2014, 12, 2));

    pivot_cache_items_t expected =
    {
        pivot_cache_item_t(date_time_t(2014, 1, 1)),
        pivot_cache_item_t(date_time_t(2014, 2, 1)),
        pivot_cache_item_t(date_time_t(2014, 3, 1)),
        pivot_cache_item_t(date_time_t(2014, 4, 1)),
        pivot_cache_item_t(date_time_t(2014, 5, 1)),
        pivot_cache_item_t(date_time_t(2014, 6, 1)),
        pivot_cache_item_t(date_time_t(2014, 7, 1)),
        pivot_cache_item_t(date_time_t(2014, 8, 1)),
        pivot_cache_item_t(date_time_t(2014, 9, 1)),
        pivot_cache_item_t(date_time_t(2014, 10, 1)),
        pivot_cache_item_t(date_time_t(2014, 11, 1)),
        pivot_cache_item_t(date_time_t(2014, 12, 1)),
    };

    pivot_cache_items_t actual(fld->items.begin(), fld->items.end());
    assert(actual == expected);

    // This field is grouped by month.

    assert(fld->group_data);
    const pivot_cache_group_data_t& gd = *fld->group_data;

    expected =
    {
        pivot_cache_item_t(ORCUS_ASCII("<1/1/2014")),
        pivot_cache_item_t(ORCUS_ASCII("Jan")),
        pivot_cache_item_t(ORCUS_ASCII("Feb")),
        pivot_cache_item_t(ORCUS_ASCII("Mar")),
        pivot_cache_item_t(ORCUS_ASCII("Apr")),
        pivot_cache_item_t(ORCUS_ASCII("May")),
        pivot_cache_item_t(ORCUS_ASCII("Jun")),
        pivot_cache_item_t(ORCUS_ASCII("Jul")),
        pivot_cache_item_t(ORCUS_ASCII("Aug")),
        pivot_cache_item_t(ORCUS_ASCII("Sep")),
        pivot_cache_item_t(ORCUS_ASCII("Oct")),
        pivot_cache_item_t(ORCUS_ASCII("Nov")),
        pivot_cache_item_t(ORCUS_ASCII("Dec")),
        pivot_cache_item_t(ORCUS_ASCII(">12/2/2014")),
    };

    assert(gd.items == expected);

    assert(gd.range_grouping);
    assert(gd.range_grouping->group_by == pivot_cache_group_by_t::months);

    assert(gd.range_grouping->start_date == date_time_t(2014,1,1));
    assert(gd.range_grouping->end_date == date_time_t(2014,12,2));

    // The 2nd field is a simple numeric field.
    fld = cache->get_field(1);
    assert(fld);
    assert(fld->name == "Value");
    assert(fld->min_value == 1.0);
    assert(fld->max_value == 12.0);

    // The 3rd field is an extra group field.
    fld = cache->get_field(2);
    assert(fld);
    assert(fld->name == "Quarters");
    assert(fld->group_data);
    const pivot_cache_group_data_t& gd_qtrs = *fld->group_data;
    assert(gd_qtrs.base_field == 0);

    assert(gd_qtrs.range_grouping);
    assert(gd_qtrs.range_grouping->group_by == pivot_cache_group_by_t::quarters);
    assert(gd_qtrs.range_grouping->start_date == date_time_t(2014,1,1));
    assert(gd_qtrs.range_grouping->end_date == date_time_t(2014,12,2));

    expected =
    {
        pivot_cache_item_t(ORCUS_ASCII("<1/1/2014")),
        pivot_cache_item_t(ORCUS_ASCII("Qtr1")),
        pivot_cache_item_t(ORCUS_ASCII("Qtr2")),
        pivot_cache_item_t(ORCUS_ASCII("Qtr3")),
        pivot_cache_item_t(ORCUS_ASCII("Qtr4")),
        pivot_cache_item_t(ORCUS_ASCII(">12/2/2014")),
    };

    assert(gd_qtrs.items == expected);
}

void test_xlsx_pivot_error_values()
{
    string path(SRCDIR"/test/xlsx/pivot-table/error-values.xlsx");

    document doc;
    import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const pivot_collection& pc = doc.get_pivot_collection();
    assert(pc.get_cache_count() == 1);

    const pivot_cache* cache = get_pivot_cache(pc, "Sheet1", "B2:C6");
    assert(cache);

    const pivot_cache_field_t* fld = cache->get_field(0);

    assert(fld);
    assert(fld->name == "F1");

    // This field should contain 4 string items 'A', 'B', 'C' and 'D'.
    std::set<pivot_cache_item_t> expected =
    {
        pivot_cache_item_t(ORCUS_ASCII("A")),
        pivot_cache_item_t(ORCUS_ASCII("B")),
        pivot_cache_item_t(ORCUS_ASCII("C")),
        pivot_cache_item_t(ORCUS_ASCII("D")),
    };

    std::set<pivot_cache_item_t> actual(fld->items.begin(), fld->items.end());
    assert(actual == expected);

    fld = cache->get_field(1);

    assert(fld);
    assert(fld->name == "F2");

    expected =
    {
        pivot_cache_item_t(spreadsheet::error_value_t::div0),
        pivot_cache_item_t(spreadsheet::error_value_t::name),
    };

    actual.clear();
    actual.insert(fld->items.begin(), fld->items.end());

    assert(actual == expected);
}

void test_xlsx_view_cursor_per_sheet()
{
    string path(SRCDIR"/test/xlsx/view/cursor-per-sheet.xlsx");

    document doc;
    spreadsheet::view view(doc);
    spreadsheet::import_factory factory(doc, view);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    // Sheet3 should be active.
    assert(view.get_active_sheet() == 2);

    const spreadsheet::sheet_view* sv = view.get_sheet_view(0);
    assert(sv);

    spreadsheet::iface::import_reference_resolver* resolver = factory.get_reference_resolver();
    assert(resolver);

    // On Sheet1, the cursor should be set to C4.
    spreadsheet::range_t expected = resolver->resolve_range(ORCUS_ASCII("C4"));

    spreadsheet::range_t actual = sv->get_selection(spreadsheet::sheet_pane_t::top_left);
    assert(expected == actual);

    sv = view.get_sheet_view(1);
    assert(sv);

    // On Sheet2, the cursor should be set to D8.
    expected = resolver->resolve_range(ORCUS_ASCII("D8"));
    actual = sv->get_selection(spreadsheet::sheet_pane_t::top_left);
    assert(expected == actual);

    sv = view.get_sheet_view(2);
    assert(sv);

    // On Sheet3, the cursor should be set to D2.
    expected = resolver->resolve_range(ORCUS_ASCII("D2"));
    actual = sv->get_selection(spreadsheet::sheet_pane_t::top_left);
    assert(expected == actual);

    sv = view.get_sheet_view(3);
    assert(sv);

    // On Sheet4, the cursor should be set to C5:E8.
    expected = resolver->resolve_range(ORCUS_ASCII("C5:E8"));
    actual = sv->get_selection(spreadsheet::sheet_pane_t::top_left);
    assert(expected == actual);
}

struct expected_selection
{
    spreadsheet::sheet_pane_t pane;
    const char* sel;
    size_t sel_n;
};

void test_xlsx_view_cursor_split_pane()
{
    string path(SRCDIR"/test/xlsx/view/cursor-split-pane.xlsx");

    document doc;
    spreadsheet::view view(doc);
    spreadsheet::import_factory factory(doc, view);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    spreadsheet::iface::import_reference_resolver* resolver = factory.get_reference_resolver();
    assert(resolver);

    // Sheet4 should be active.
    assert(view.get_active_sheet() == 3);

    const spreadsheet::sheet_view* sv = view.get_sheet_view(0);
    assert(sv);

    // On Sheet1, the view is split into 4.
    assert(sv->get_active_pane() == spreadsheet::sheet_pane_t::bottom_left);
    assert(sv->get_split_pane().hor_split == 5190.0);
    assert(sv->get_split_pane().ver_split == 1800.0);

    {
        spreadsheet::address_t expected = resolver->resolve_address(ORCUS_ASCII("F6"));
        spreadsheet::address_t actual = sv->get_split_pane().top_left_cell;
        assert(expected == actual);
    }

    std::vector<expected_selection> expected_selections =
    {
        { spreadsheet::sheet_pane_t::top_left,     ORCUS_ASCII("E4")  },
        { spreadsheet::sheet_pane_t::top_right,    ORCUS_ASCII("J2")  },
        { spreadsheet::sheet_pane_t::bottom_left,  ORCUS_ASCII("A8")  },
        { spreadsheet::sheet_pane_t::bottom_right, ORCUS_ASCII("J17") },
    };

    for (const expected_selection& es : expected_selections)
    {
        // cursor in the top-left pane.
        spreadsheet::range_t expected = resolver->resolve_range(es.sel, es.sel_n);
        spreadsheet::range_t actual = sv->get_selection(es.pane);
        assert(expected == actual);
    }

    sv = view.get_sheet_view(1);
    assert(sv);

    // Sheet2 is also split into 4 views.
    assert(sv->get_active_pane() == spreadsheet::sheet_pane_t::top_right);
    assert(sv->get_split_pane().hor_split == 5190.0);
    assert(sv->get_split_pane().ver_split == 2400.0);

    {
        spreadsheet::address_t expected = resolver->resolve_address(ORCUS_ASCII("F8"));
        spreadsheet::address_t actual = sv->get_split_pane().top_left_cell;
        assert(expected == actual);
    }

    expected_selections =
    {
        { spreadsheet::sheet_pane_t::top_left,     ORCUS_ASCII("C2:C6")   },
        { spreadsheet::sheet_pane_t::top_right,    ORCUS_ASCII("H2:L2")   },
        { spreadsheet::sheet_pane_t::bottom_left,  ORCUS_ASCII("B18:C23") },
        { spreadsheet::sheet_pane_t::bottom_right, ORCUS_ASCII("H11:J13") },
    };

    for (const expected_selection& es : expected_selections)
    {
        // cursor in the top-left pane.
        spreadsheet::range_t expected = resolver->resolve_range(es.sel, es.sel_n);
        spreadsheet::range_t actual = sv->get_selection(es.pane);
        assert(expected == actual);
    }

    sv = view.get_sheet_view(2);
    assert(sv);

    // Sheet3 is horizontally split into top and bottom views (top-left and bottom-left).
    assert(sv->get_active_pane() == spreadsheet::sheet_pane_t::bottom_left);
    assert(sv->get_split_pane().hor_split == 0.0);
    assert(sv->get_split_pane().ver_split == 1500.0);

    {
        spreadsheet::address_t expected = resolver->resolve_address(ORCUS_ASCII("A5"));
        spreadsheet::address_t actual = sv->get_split_pane().top_left_cell;
        assert(expected == actual);
    }

    expected_selections =
    {
        { spreadsheet::sheet_pane_t::top_left,     ORCUS_ASCII("D2") },
        { spreadsheet::sheet_pane_t::bottom_left,  ORCUS_ASCII("C9") },
    };

    for (const expected_selection& es : expected_selections)
    {
        // cursor in the top-left pane.
        spreadsheet::range_t expected = resolver->resolve_range(es.sel, es.sel_n);
        spreadsheet::range_t actual = sv->get_selection(es.pane);
        assert(expected == actual);
    }

    sv = view.get_sheet_view(3);
    assert(sv);

    // Sheet4 is vertically split into left and right views (top-left and top-right).
    assert(sv->get_active_pane() == spreadsheet::sheet_pane_t::top_left);
    assert(sv->get_split_pane().hor_split == 4230.0);
    assert(sv->get_split_pane().ver_split == 0.0);

    {
        spreadsheet::address_t expected = resolver->resolve_address(ORCUS_ASCII("E1"));
        spreadsheet::address_t actual = sv->get_split_pane().top_left_cell;
        assert(expected == actual);
    }

    expected_selections =
    {
        { spreadsheet::sheet_pane_t::top_left,  ORCUS_ASCII("B18") },
        { spreadsheet::sheet_pane_t::top_right, ORCUS_ASCII("I11") },
    };

    for (const expected_selection& es : expected_selections)
    {
        // cursor in the top-left pane.
        spreadsheet::range_t expected = resolver->resolve_range(es.sel, es.sel_n);
        spreadsheet::range_t actual = sv->get_selection(es.pane);
        assert(expected == actual);
    }
}

}

int main()
{
    test_config.debug = true;
    test_config.structure_check = true;

    test_xlsx_import();
    test_xlsx_table_autofilter();
    test_xlsx_table();
    test_xlsx_merged_cells();
    test_xlsx_date_time();

    // pivot table
    test_xlsx_pivot_two_pivot_caches();
    test_xlsx_pivot_mixed_type_field();
    test_xlsx_pivot_group_field();
    test_xlsx_pivot_group_by_numbers();
    test_xlsx_pivot_group_by_dates();
    test_xlsx_pivot_error_values();

    // view import
    test_xlsx_view_cursor_per_sheet();
    test_xlsx_view_cursor_split_pane();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
