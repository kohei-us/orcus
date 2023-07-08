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

#include <boost/filesystem/path.hpp>
#include <iostream>
#include <sstream>

using namespace orcus;
namespace fs = boost::filesystem;

namespace {

std::vector<fs::path> dirs = {
    SRCDIR"/test/gnumeric/raw-values-1/",
    SRCDIR"/test/gnumeric/formula-cells/",
};

void test_gnumeric_import()
{
    ORCUS_TEST_FUNC_SCOPE;

    for (const auto& dir : dirs)
    {
        std::cout << "checking " << dir << "..." << std::endl;

        // Read the input.gnumeric document.
        fs::path filepath = dir / "input.gnumeric";
        spreadsheet::range_size_t ss{1048576, 16384};
        spreadsheet::document doc{ss};
        spreadsheet::import_factory factory(doc);
        orcus_gnumeric app(&factory);
        app.read_file(filepath.string());

        // Gnumeric doc doesn't cache formula results.
        doc.recalc_formula_cells();

        // Dump the content of the model.
        std::ostringstream os;
        doc.dump_check(os);
        std::string check = os.str();

        // Check that against known control.
        filepath = dir / "check.txt";
        file_content control(filepath.string());

        assert(!check.empty());
        assert(!control.empty());

        test::verify_content(__FILE__, __LINE__, control.str(), check);
    }
}

}

int main()
{
    test_gnumeric_import();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
