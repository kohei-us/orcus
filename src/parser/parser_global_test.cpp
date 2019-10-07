/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include <orcus/parser_global.hpp>

#include <vector>
#include <cmath>
#include <cstring>
#include <limits>

namespace {

struct test_case
{
    const char* str;
    double val;
};

void test_parse_numbers()
{
    std::vector<test_case> test_cases = {
        {"1", 1.0},
        {"1.0", 1.0},
        {"-1.0", -1.0},
        {"2e2", 200.0},
        {"1.2", 1.2},
        {"-0.0001", -0.0001},
        {"-0.0", 0.0},
        {"+.", std::numeric_limits<double>::signaling_NaN()},
        {"+e", std::numeric_limits<double>::signaling_NaN()},
        {"+e1", std::numeric_limits<double>::signaling_NaN()},
        {"+ ",  std::numeric_limits<double>::signaling_NaN()},
        {"- ",  std::numeric_limits<double>::signaling_NaN()}
    };

    for (const test_case& test_data : test_cases)
    {
        const char* str = test_data.str;
        volatile double val = orcus::parse_numeric(str, std::strlen(test_data.str));
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
    test_parse_numbers();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
