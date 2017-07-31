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
        spreadsheet::document doc;
        {
            spreadsheet::import_factory factory(doc);
            orcus_csv app(&factory);
            app.read_file(path.c_str());
        }

        // Dump the content of the model.
        std::ostringstream os;
        doc.dump_check(os);
        std::string check = os.str();
        os.clear();

        // Check that against known control.
        path = dir;
        path.append("check.txt");
        std::string control = load_file_content(path.c_str());

        assert(!check.empty());
        assert(!control.empty());

        pstring s1(check.data(), check.size()), s2(control.data(), control.size());
        assert(s1.trim() == s2.trim());

        spreadsheet::sheet* sh = doc.get_sheet(0);
        assert(sh);

        // Dump the first sheet as csv.
        sh->dump_csv(os);
        std::string stream = os.str();
        os.clear();

        // Re-import the dumped csv.
        doc.clear();
        {
            spreadsheet::import_factory factory(doc);
            orcus_csv app(&factory);
            app.read_stream(stream.data(), stream.size());
        }

        // Dump the content of the re-imported model, and make sure it's still
        // identical to the control.
        doc.dump_check(os);
        check = os.str();
        os.clear();

        assert(!check.empty());
        s1 = pstring(check.data(), check.size());
        assert(s1.trim() == s2.trim());
    }
}

}

int main()
{
    test_csv_import();
    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
