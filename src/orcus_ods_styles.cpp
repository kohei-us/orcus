/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_import_ods.hpp"
#include "orcus/spreadsheet/styles.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/string_pool.hpp"
#include "orcus/stream.hpp"

#include "orcus_filter_global.hpp"

#include <cstdlib>
#include <iostream>

using namespace std;
using namespace orcus;

int main(int argc, char** argv)
{
    if (argc != 2)
        return EXIT_FAILURE;

    string_pool sp;
    spreadsheet::styles styles;
    spreadsheet::import_styles istyles(styles, sp);

    try
    {
        file_content content(argv[1]);
        import_ods::read_styles(content.str(), &istyles);
    }
    catch(std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
