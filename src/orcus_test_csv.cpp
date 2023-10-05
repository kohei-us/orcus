/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_test_global.hpp"
#include "orcus/orcus_csv.hpp"
#include <orcus/format_detection.hpp>
#include "orcus/config.hpp"
#include "orcus/stream.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/sheet.hpp"

#include <cstdlib>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

using namespace orcus;
namespace ss = orcus::spreadsheet;

namespace {

std::vector<const char*> dirs = {
    SRCDIR"/test/csv/simple-numbers/",
    SRCDIR"/test/csv/normal-quotes/",
    SRCDIR"/test/csv/double-quotes/",
    SRCDIR"/test/csv/quoted-with-delim/",
};

void test_csv_create_filter()
{
    ORCUS_TEST_FUNC_SCOPE;

    ss::range_size_t ssize{1048576, 16384};
    std::unique_ptr<ss::document> doc = std::make_unique<ss::document>(ssize);
    ss::import_factory factory(*doc);

    auto f = create_filter(format_t::csv, &factory);
    assert(f);
    assert(f->get_name() == "csv");
}

void test_csv_import()
{
    ORCUS_TEST_FUNC_SCOPE;

    for (const char* dir : dirs)
    {
        std::string path(dir);

        // Read the input.csv document.
        path.append("input.csv");

        std::cout << "checking " << path << "..." << std::endl;

        spreadsheet::range_size_t ss{1048576, 16384};
        spreadsheet::document doc{ss};
        {
            spreadsheet::import_factory factory(doc);
            orcus_csv app(&factory);
            app.read_file(path.c_str());
        }

        // Dump the content of the model.
        std::string check = test::get_content_check(doc);

        // Check that against known control.
        path = dir;
        path.append("check.txt");
        file_content control(path.c_str());

        assert(!check.empty());
        assert(!control.empty());

        test::verify_content(__FILE__, __LINE__, control.str(), check);

        // Dump the first sheet as csv.
        std::string stream = test::get_content_as_csv(doc, 0);
        assert(!stream.empty());

        // Re-import the dumped csv.
        doc.clear();
        {
            spreadsheet::import_factory factory(doc);
            orcus_csv app(&factory);
            app.read_stream(stream);
        }

        // Dump the content of the re-imported model, and make sure it's still
        // identical to the control.
        check = test::get_content_check(doc);
        assert(!check.empty());

        test::verify_content(__FILE__, __LINE__, control.str(), check);
    }
}

void test_csv_import_split_sheet()
{
    ORCUS_TEST_FUNC_SCOPE;

    const char* dir = SRCDIR"/test/csv/split-sheet/";

    std::string path(dir);
    path.append("input.csv");

    std::cout << "checking " << path << "..." << std::endl;

    config conf(format_t::csv);
    std::get<config::csv_config>(conf.data).header_row_size = 0;
    std::get<config::csv_config>(conf.data).split_to_multiple_sheets = true;

    // Set the row size to 11 to make sure the split occurs.
    spreadsheet::range_size_t ss{11, 4};
    spreadsheet::document doc{ss};
    {
        spreadsheet::import_factory factory(doc);
        orcus_csv app(&factory);
        app.set_config(conf);

        app.read_file(path.c_str());
    }

    assert(doc.get_sheet_count() == 2);

    // Dump the content of the model.
    std::string check = test::get_content_check(doc);

    // Check that against known control.
    path = dir;
    path.append("check-1.txt");
    file_content control(path.data());

    test::verify_content(__FILE__, __LINE__, control.str(), check);

    // Re-import the same input file, but have the first row repeated on every
    // sheet.
    path = dir;
    path.append("input.csv");
    doc.clear();
    std::get<config::csv_config>(conf.data).header_row_size = 1;
    {
        spreadsheet::import_factory factory(doc);
        orcus_csv app(&factory);
        app.set_config(conf);

        app.read_file(path.c_str());
    }

    assert(doc.get_sheet_count() == 2);

    // Dump the content of the model.
    check = test::get_content_check(doc);

    // Check that against known control.
    path = dir;
    path.append("check-2.txt");
    control.load(path.data());

    test::verify_content(__FILE__, __LINE__, control.str(), check);

    // Re-import it again, but this time disable the splitting.  The data should
    // get trucated on the first sheet.
    std::get<config::csv_config>(conf.data).split_to_multiple_sheets = false;

    path = dir;
    path.append("input.csv");
    doc.clear();

    {
        spreadsheet::import_factory factory(doc);
        orcus_csv app(&factory);
        app.set_config(conf);

        app.read_file(path.c_str());
    }

    assert(doc.get_sheet_count() == 1);

    // Dump the content of the model.
    check = test::get_content_check(doc);

    // Check that against known control.
    path = dir;
    path.append("check-3.txt");
    control.load(path.data());

    test::verify_content(__FILE__, __LINE__, control.str(), check);
}

void test_csv_dump_flat_utf8()
{
    ORCUS_TEST_FUNC_SCOPE;

    constexpr std::string_view src =
        "New York,fabriqué\n"
        "garçon,вход\n"
        "выход,помогите\n"
        "Nähe,San Diego";

    constexpr std::string_view expected =
        "rows: 4  cols: 2\n"
        "+----------+-----------+\n"
        "| New York | fabriqué  |\n"
        "+----------+-----------+\n"
        "| garçon   | вход      |\n"
        "+----------+-----------+\n"
        "| выход    | помогите  |\n"
        "+----------+-----------+\n"
        "| Nähe     | San Diego |\n"
        "+----------+-----------+\n";

    ss::range_size_t ss{1048576, 16384};
    ss::document doc{ss};
    ss::import_factory factory(doc);
    orcus_csv app(&factory);
    app.read_stream(src);

    const ss::sheet* sh = doc.get_sheet(0);
    assert(sh);
    std::ostringstream os;
    sh->dump_flat(os);
    std::string flat_dump = os.str();

    test::verify_content(__FILE__, __LINE__, expected, flat_dump);
}

} // anonymous namespace

int main()
{
    try
    {
        test_csv_create_filter();
        test_csv_import();
        test_csv_import_split_sheet();
        test_csv_dump_flat_utf8();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
