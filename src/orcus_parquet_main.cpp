/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_parquet.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/factory.hpp"

#include "orcus_filter_global.hpp"

#include <iostream>

int main(int argc, char** argv) try
{
    orcus::spreadsheet::range_size_t ss{1048576, 16384};
    orcus::spreadsheet::document doc{ss};
    orcus::spreadsheet::import_factory fact(doc);
    orcus::orcus_parquet app(&fact);

    if (parse_import_filter_args(argc, argv, fact, app, doc))
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

