/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_xls_xml.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/view.hpp"

#include "orcus_filter_global.hpp"

#include <iostream>

using namespace orcus;

int ORCUS_CLI_MAIN(int argc, arg_char_t** argv) try
{
    bootstrap_program();

    spreadsheet::range_size_t ss{1048576, 16384};
    spreadsheet::document doc{ss};
    spreadsheet::view view(doc);
    spreadsheet::import_factory fact(doc, view);
    orcus_xls_xml app(&fact);

    import_filter_arg_parser<arg_char_t> parser(fact, app, doc);

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
