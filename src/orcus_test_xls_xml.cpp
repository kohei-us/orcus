/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_xls_xml.hpp"
#include "orcus/pstring.hpp"
#include "orcus/global.hpp"
#include "orcus/stream.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/spreadsheet/document.hpp"

#include <cassert>
#include <string>
#include <sstream>
#include <vector>

using namespace orcus;
using namespace std;

namespace {

std::vector<const char*> dirs = {
    SRCDIR"/test/xls-xml/basic/",
    SRCDIR"/test/xls-xml/empty-rows/",
};

void test_xls_xml_import()
{
    for (const char* dir : dirs)
    {
        string path(dir);

        // Read the input.xml document.
        path.append("input.xml");
        spreadsheet::document doc;
        spreadsheet::import_factory factory(doc);
        orcus_xls_xml app(&factory);
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

}

int main()
{
    test_xls_xml_import();
    return EXIT_SUCCESS;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
