/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/parser_global.hpp>

#include <vector>
#include <cmath>
#include <cstring>
#include <cassert>
#include <limits>

namespace {

void test_just_minus_as_number()
{
    const char* str = "- ";
    double val = orcus::parse_numeric(str, 2);
    assert(std::isnan(val));
}

void test_just_plus_as_number()
{
    const char* str = "+ ";
    double val = orcus::parse_numeric(str, 2);
    assert(std::isnan(val));
}

struct test_case
{
    const char* str;
    double val;
};

void test_simple_numbers()
{
    std::vector<test_case> test_cases = {
        {"1", 1.0},
        {"1.0", 1.0},
        {"-1.0", -1.0},
        {"2e2", 200.0},
        {"1.2", 1.2},
        {"-0.0001", -0.0001},
        {"-0.0", 0.0},
        {"+.", std::numeric_limits<double>::signaling_NaN()}
    };

    for (const test_case& test_data : test_cases)
    {
        const char* str = test_data.str;
        double val = orcus::parse_numeric(str, std::strlen(test_data.str));
        if (std::isnan(test_data.val))
        {
            assert(std::isnan(val));
        }
        else
        {
            assert(val == test_data.val);
        }
    }
}

}

int main()
{
    test_just_minus_as_number();
    test_just_plus_as_number();
    test_simple_numbers();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
