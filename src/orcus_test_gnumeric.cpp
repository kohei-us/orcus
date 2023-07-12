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

}

int main()
{
    test_gnumeric_import();
    test_gnumeric_column_widths_row_heights();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
