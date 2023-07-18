/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_test_global.hpp"

#include <orcus/orcus_gnumeric.hpp>
#include <orcus/stream.hpp>
#include <orcus/spreadsheet/factory.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/sheet.hpp>
#include <orcus/spreadsheet/auto_filter.hpp>

#include <ixion/address.hpp>
#include <boost/filesystem/path.hpp>
#include <iostream>
#include <sstream>

using namespace orcus;
namespace fs = boost::filesystem;
namespace ss = orcus::spreadsheet;

namespace {

std::vector<fs::path> dirs = {
    SRCDIR"/test/gnumeric/raw-values-1/",
    SRCDIR"/test/gnumeric/formula-cells/",
};

std::unique_ptr<ss::document> load_doc(const fs::path& filepath)
{
    ss::range_size_t ss{1048576, 16384};
    auto doc = std::make_unique<ss::document>(ss);
    ss::import_factory factory(*doc);
    orcus_gnumeric app(&factory);
    app.read_file(filepath.string());

    // Gnumeric doc doesn't cache formula results.
    doc->recalc_formula_cells();

    return doc;
}

void test_gnumeric_import()
{
    ORCUS_TEST_FUNC_SCOPE;

    for (const auto& dir : dirs)
    {
        std::cout << "checking " << dir << "..." << std::endl;

        // Read the input.gnumeric document.
        fs::path filepath = dir / "input.gnumeric";
        auto doc = load_doc(filepath);

        // Dump the content of the model.
        std::ostringstream os;
        doc->dump_check(os);
        std::string check = os.str();

        // Check that against known control.
        filepath = dir / "check.txt";
        file_content control(filepath.string());

        assert(!check.empty());
        assert(!control.empty());

        test::verify_content(__FILE__, __LINE__, control.str(), check);
    }
}

void test_gnumeric_column_widths_row_heights()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath = SRCDIR"/test/gnumeric/column-width-row-height/input.gnumeric";
    auto doc = load_doc(filepath);

    assert(doc->get_sheet_count() == 1);
    const ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

    {
        // column, column width (twips), column span
        const std::tuple<ss::col_t, ss::col_width_t, ss::col_t> expected[] = {
            { 0, 99 * 20, 1 },
            { 1, 57 * 20, 1 },
            { 2, 84 * 20, 1 },
            { 3, 111 * 20, 1 },
            { 4, ss::get_default_column_width(), 1 },
            { 5, 69 * 20, 3 },
            { 10, 120 * 20, 2 },
        };

        for (const auto& [col, cw_expected, span_expected] : expected)
        {
            ss::col_t col_start, col_end;
            ss::col_width_t cw = sh->get_col_width(col, &col_start, &col_end);
            ss::col_t span = col_end - col_start;

            std::cout << "column: " << col << "; expected=" << cw_expected << "; actual=" << cw
                << "; span-expected=" << span_expected << "; span-actual=" << span << std::endl;

            assert(cw == cw_expected);
            assert(span == span_expected);
        }
    }

    {
        // row, row height (twips), row span
        const std::tuple<ss::row_t, ss::row_height_t, ss::row_t> expected[] = {
            { 0, 18 * 20, 3 },
            { 3, 30 * 20, 1 },
            { 4, 42 * 20, 1 },
            { 5, 51 * 20, 1 },
            { 6, ss::get_default_row_height(), 1 },
            { 7, 27 * 20, 3 },
            { 10, ss::get_default_row_height(), 2 },
            { 12, 36 * 20, 2 },
        };

        for (const auto& [row, rh_expected, span_expected] : expected)
        {
            ss::row_t row_start, row_end;
            ss::row_height_t rh = sh->get_row_height(row, &row_start, &row_end);
            ss::row_t span = row_end - row_start;

            std::cout << "row: " << row << "; expected=" << rh_expected << "; actual=" << rh
                << "; span-expected=" << span_expected << "; span-actual=" << span << std::endl;

            assert(rh == rh_expected);
            assert(span == span_expected);
        }
    }
}

void test_gnumeric_auto_filter()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath = SRCDIR"/test/gnumeric/table/autofilter.gnumeric";
    auto doc = load_doc(filepath);

    assert(doc->get_sheet_count() == 1);
    const ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

    const ss::auto_filter_t* af = sh->get_auto_filter_data();
    assert(af);
    ixion::abs_range_t b2_c11{0, 1, 1, 10, 2};
    assert(af->range == b2_c11);
    assert(af->columns.size() == 2);

    auto it = af->columns.begin();
    assert(it->first == 0);
    {
        const ss::auto_filter_column_t& afc = it->second;
        assert(afc.match_values.size() == 1);
        assert(*afc.match_values.begin() == "A");
    }

    ++it;
    assert(it->first == 1);
    {
        const ss::auto_filter_column_t& afc = it->second;
        assert(afc.match_values.size() == 1);
        assert(*afc.match_values.begin() == "1");
    }
}

void test_gnumeric_hidden_rows_columns()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath = SRCDIR"/test/gnumeric/hidden-rows-columns/input.gnumeric";
    auto doc = load_doc(filepath);

    spreadsheet::sheet* sh = doc->get_sheet("Hidden Rows");
    assert(sh);

    spreadsheet::row_t row_start = -1, row_end = -1;

    // Row 1 is visible.
    assert(!sh->is_row_hidden(0, &row_start, &row_end));
    assert(row_start == 0);
    assert(row_end == 1); // the end position is non-inclusive.

    // Rows 2-3 are hidden.
    assert(sh->is_row_hidden(1, &row_start, &row_end));
    assert(row_start == 1);
    assert(row_end == 3); // the end position is non-inclusive.

    // Row 4 is visible.
    assert(!sh->is_row_hidden(3, &row_start, &row_end));
    assert(row_start == 3);
    assert(row_end == 4); // the end position is non-inclusive.

    // Row 5 is hidden.
    assert(sh->is_row_hidden(4, &row_start, &row_end));
    assert(row_start == 4);
    assert(row_end == 5); // the end position is non-inclusive.

    // Rows 6-8 are visible.
    assert(!sh->is_row_hidden(5, &row_start, &row_end));
    assert(row_start == 5);
    assert(row_end == 8); // the end position is non-inclusive.

    // Row 9 is hidden.
    assert(sh->is_row_hidden(8, &row_start, &row_end));
    assert(row_start == 8);
    assert(row_end == 9); // the end position is non-inclusive.

    // The rest of the rows are visible.
    assert(!sh->is_row_hidden(9, &row_start, &row_end));
    assert(row_start == 9);
    assert(row_end == doc->get_sheet_size().rows); // the end position is non-inclusive.

    sh = doc->get_sheet("Hidden Columns");
    assert(sh);

    spreadsheet::col_t col_start = -1, col_end = -1;

    // Columns A-B are visible.
    assert(!sh->is_col_hidden(0, &col_start, &col_end));
    assert(col_start == 0);
    assert(col_end == 2); // non-inclusive

    // Columns C-E are hidden.
    assert(sh->is_col_hidden(2, &col_start, &col_end));
    assert(col_start == 2);
    assert(col_end == 6); // non-inclusive

    // Columns G-J are visible.
    assert(!sh->is_col_hidden(6, &col_start, &col_end));
    assert(col_start == 6);
    assert(col_end == 10); // non-inclusive

    // Column K is hidden.
    assert(sh->is_col_hidden(10, &col_start, &col_end));
    assert(col_start == 10);
    assert(col_end == 11); // non-inclusive

    // The rest of the columns are all visible.
    assert(!sh->is_col_hidden(11, &col_start, &col_end));
    assert(col_start == 11);
    assert(col_end == doc->get_sheet_size().columns); // non-inclusive
}

void test_gnumeric_merged_cells()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath = SRCDIR"/test/gnumeric/merged-cells/input.gnumeric";
    auto doc = load_doc(filepath);

    const spreadsheet::sheet* sheet1 = doc->get_sheet("Sheet1");
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

}

int main()
{
    test_gnumeric_import();
    test_gnumeric_column_widths_row_heights();
    test_gnumeric_auto_filter();
    test_gnumeric_hidden_rows_columns();
    test_gnumeric_merged_cells();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
