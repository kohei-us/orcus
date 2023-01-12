/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/yaml_parser.hpp>

#include <string>

void test_handler()
{
    constexpr std::string_view test_code =
    "section-one:\n"
    "  - item 1\n"
    "  - item 2\n"
    "\n";

    orcus::yaml_handler hdl;
    orcus::yaml_parser<orcus::yaml_handler> parser(test_code, hdl);
    parser.parse();
}

int main()
{
    test_handler();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
