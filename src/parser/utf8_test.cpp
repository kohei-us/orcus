/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>
#include <vector>
#include <string>
#include <cassert>

#include "utf8.hpp"

using namespace orcus;
using std::cout;
using std::endl;

void test_xml_name_start_char()
{
    std::vector<std::string> ascii_ranges = { "az", "AZ", "__" };

    for (const std::string& range : ascii_ranges)
    {
        for (char c = range[0]; c <= range[1]; ++c)
        {
            const char* p = &c;
            const char* p_end = p + 1;
            const char* ret = parse_utf8_xml_name_start_char(p, p_end);

            if (ret != p_end)
            {
                cout << "failed to parse '" << c << "'" << endl;
                assert(false);
            }
        }
    }

    // TODO : implement the rest.
}

void test_xml_name_char()
{
    std::vector<std::string> ascii_ranges = { "az", "AZ", "09", "__", "--", ".." };

    for (const std::string& range : ascii_ranges)
    {
        for (char c = range[0]; c <= range[1]; ++c)
        {
            const char* p = &c;
            const char* p_end = p + 1;
            const char* ret = parse_utf8_xml_name_char(p, p_end);

            if (ret != p_end)
            {
                cout << "failed to parse '" << c << "'" << endl;
                assert(false);
            }
        }
    }

    // TODO : implement the rest.
}

int main(int argc, char** argv)
{
    test_xml_name_start_char();
    test_xml_name_char();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
