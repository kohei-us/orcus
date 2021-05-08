/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include "utf8.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <functional>


using namespace orcus;
using std::cout;
using std::endl;

using parse_func_t = std::function<const char*(const char*, const char*)>;

bool check_ascii_ranges(parse_func_t parse, std::vector<std::string> ascii_ranges)
{
    for (const std::string& range : ascii_ranges)
    {
        for (char c = range[0]; c <= range[1]; ++c)
        {
            const char* p = &c;
            const char* p_end = p + 1;
            const char* ret = parse(p, p_end);

            if (ret != p_end)
            {
                cout << "failed to parse '" << c << "'" << endl;
                return false;
            }
        }
    }

    return true;
}

void test_xml_name_start_char()
{
    bool res = check_ascii_ranges(
        parse_utf8_xml_name_start_char,
        { "az", "AZ", "__" }
    );
    assert(res);

    // TODO : implement the rest.
}

void test_xml_name_char()
{
    bool res = check_ascii_ranges(
        parse_utf8_xml_name_char,
        { "az", "AZ", "09", "__", "--", ".." }
    );
    assert(res);

    // TODO : implement the rest.
}

int main(int argc, char** argv)
{
    test_xml_name_start_char();
    test_xml_name_char();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
