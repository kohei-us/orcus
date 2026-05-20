/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/csv_parser.hpp>

#include <cassert>
#include <cstring>
#include <string>
#include <vector>

void test_handler()
{
    const char* test_code = "1,2,3,4,5\n6,7,8,9,10\n";

    orcus::csv_handler hdl;
    orcus::csv::parser_config config;
    orcus::csv_parser<orcus::csv_handler> parser(test_code, hdl, config);
    parser.parse();
}

namespace {

class recording_handler : public orcus::csv_handler
{
public:
    std::vector<std::string> cells;

    void cell(std::string_view value, bool /*transient*/)
    {
        cells.emplace_back(value);
    }
};

}

void test_truncated_quoted_cell()
{
    // A quoted cell with no closing quote should not read past the end of the
    // source buffer. The reported cell text should stop at the last byte of
    // the input.
    std::string input("\"abc");

    recording_handler hdl;
    orcus::csv::parser_config config;
    config.text_qualifier = '"';
    orcus::csv_parser<recording_handler> parser(input, hdl, config);
    parser.parse();

    assert(hdl.cells.size() == 1);
    assert(hdl.cells[0] == "abc");
}

int main()
{
    test_handler();
    test_truncated_quoted_cell();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
