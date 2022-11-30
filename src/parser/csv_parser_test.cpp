/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/csv_parser.hpp>

#include <cstring>

void test_handler()
{
    const char* test_code = "1,2,3,4,5\n6,7,8,9,10\n";

    orcus::csv_handler hdl;
    orcus::csv::parser_config config;
    orcus::csv_parser<orcus::csv_handler> parser(test_code, hdl, config);
    parser.parse();
}

int main()
{
    test_handler();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
