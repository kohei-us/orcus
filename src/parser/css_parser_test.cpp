/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/css_parser.hpp>

#include <cstring>

void test_handler()
{
    const char* test_code = "p { background-color: white; }";
    size_t n = strlen(test_code);

    orcus::css_handler hdl;
    orcus::css_parser<orcus::css_handler> parser(test_code, n, hdl);
    parser.parse();
}

int main()
{
    test_handler();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
