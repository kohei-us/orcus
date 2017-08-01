/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_csv.hpp"
#include "orcus/pstring.hpp"
#include "orcus/global.hpp"
#include "orcus/stream.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/sheet.hpp"

#include "orcus_test_global.hpp"

#include <cstdlib>
#include <cassert>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

using namespace orcus;

namespace {

std::vector<const char*> dirs = {
    SRCDIR"/test/csv/simple-numbers/",
    SRCDIR"/test/csv/normal-quotes/",
    SRCDIR"/test/csv/double-quotes/",
    SRCDIR"/test/csv/quoted-with-delim/",
};

void test_csv_import()
{
    for (const char* dir : dirs)
    {
        std::string path(dir);

        // Read the input.csv document.
        path.append("input.csv");

        std::cout << "checking " << path << "..." << std::endl;

        spreadsheet::document doc;
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
        std::string control = load_file_content(path.c_str());

        assert(!check.empty());
        assert(!control.empty());

        test::verify_content(__FILE__, __LINE__, control, check);

        // Dump the first sheet as csv.
        std::string stream = test::get_content_as_csv(doc, 0);
        assert(!stream.empty());

        // Re-import the dumped csv.
        doc.clear();
        {
            spreadsheet::import_factory factory(doc);
            orcus_csv app(&factory);
            app.read_stream(stream.data(), stream.size());
        }

        // Dump the content of the re-imported model, and make sure it's still
        // identical to the control.
        check = test::get_content_check(doc);
        assert(!check.empty());

        test::verify_content(__FILE__, __LINE__, control, check);
    }
}

}

int main()
{
    try
    {
        test_csv_import();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
