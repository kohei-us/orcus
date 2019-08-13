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
#include <iostream>

using namespace std;

namespace {

struct test_case
{
    const char* str;
    double val;
    double multiplier;
};

/**
 * To work around double-precision rounding errors, this function compares
 * the two values as integers, after both values get multiplifed by the
 * specified multiplier to make the values large enough to allow integer
 * comparison.
 */
bool compare_equal(double a, double b, double multiplier)
{
    int ia = std::round(a * multiplier * 10);
    int ib = std::round(b * multiplier * 10);

    if (ia != ib)
    {
        cout << "failed integer comparison: (ia = " << ia << "; ib = " << ib << ")" << endl;
    }

    return ia == ib;
}

void test_parse_numbers()
{
    std::vector<test_case> test_cases =
    {
        { "1", 1.0, 1.0 },
        { "1.0", 1.0, 1.0 },
        { "-1.0", -1.0, 1.0 },
        { "2e2", 200.0, 1.0 },
        { "1.2", 1.2, 10.0 },
        { "-0.0001", -0.0001, 10000.0 },
        { "-0.0", 0.0, 1.0 },
        { "+.", std::numeric_limits<double>::signaling_NaN(), 1.0 },
        { "+e", std::numeric_limits<double>::signaling_NaN(), 1.0 },
        { "+e1", std::numeric_limits<double>::signaling_NaN(), 1.0 },
        { "+ ",  std::numeric_limits<double>::signaling_NaN(), 1.0 },
        { "- ",  std::numeric_limits<double>::signaling_NaN(), 1.0 },
    };

    for (const test_case& test_data : test_cases)
    {
        const char* str = test_data.str;
        const double val = orcus::parse_numeric(str, std::strlen(test_data.str));
        if (std::isnan(test_data.val))
        {
            assert(std::isnan(val));
        }
        else
        {
            assert(compare_equal(val, test_data.val, test_data.multiplier));
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
