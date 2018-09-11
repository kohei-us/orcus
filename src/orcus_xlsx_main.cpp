/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_xlsx.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/view.hpp"

#include "orcus_filter_global.hpp"

#include <iostream>

using namespace orcus;

int main(int argc, char** argv)
{
    try
    {
        spreadsheet::document doc;
        spreadsheet::view view(doc);
        spreadsheet::import_factory fact(doc, view);
        orcus_xlsx app(&fact);

        if (parse_import_filter_args(argc, argv, fact, app, doc))
            return EXIT_FAILURE;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
