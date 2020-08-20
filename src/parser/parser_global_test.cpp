/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include <orcus/parser_global.hpp>
#include <orcus/cell_buffer.hpp>

#include <vector>
#include <cmath>
#include <cstring>
#include <limits>

namespace {

void test_parse_numbers()
{
    struct test_case
    {
        const char* str;
        double val;
    };

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

void test_parse_double_quoted_strings()
{
    struct test_case
    {
        std::string input;
        const char* expected_p;
        size_t expected_n;
    };

    std::vector<test_case> test_cases = {
        { "\"", nullptr, orcus::parse_quoted_string_state::error_no_closing_quote },
        { "\"\"", "", 0 },
        { "\"a\"", "a", 1 },

    };

    for (const test_case& tc : test_cases)
    {
        orcus::cell_buffer buf;
        const char* p = tc.input.data();
        size_t n = tc.input.size();
        orcus::parse_quoted_string_state ret = orcus::parse_double_quoted_string(p, n, buf);

        if (tc.expected_p)
        {
            std::string expected(tc.expected_p, tc.expected_n);
            std::string actual(ret.str, ret.length);
            assert(expected == actual);
        }
        else
        {
            assert(ret.str == nullptr);
            assert(ret.length == tc.expected_n);
        }
    }

}

}

int main()
{
    test_parse_numbers();
    test_parse_double_quoted_strings();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
