/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/json_parser.hpp>

#include <cstring>

void test_handler()
{
    const char* test_code = "{\"key1\": [1,2,3,4,5], \"key2\": 12.3}";

    orcus::json_handler hdl;
    orcus::json_parser<orcus::json_handler> parser(test_code, hdl);
    parser.parse();
}

int main()
{
    test_handler();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
