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

int ORCUS_CLI_MAIN(int argc, arg_char_t** argv) try
{
    orcus::bootstrap_program();

    orcus::spreadsheet::range_size_t ss{1048576, 16384};
    orcus::spreadsheet::document doc{ss};
    orcus::spreadsheet::import_factory fact(doc);
    orcus::orcus_parquet app(&fact);

    orcus::import_filter_arg_parser<arg_char_t> parser(fact, app, doc);

    if (!parser.parse(argc, argv))
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

