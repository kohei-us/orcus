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
    orcus::test::stack_printer __sp__(__func__);

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

void test_parse_integers()
{
    orcus::test::stack_printer __sp__(__func__);

    std::string_view test_str = "-100";

    long value;
    const char* p = test_str.data();
    const char* p_end = p + test_str.size();
    const char* p_last = orcus::parse_integer(p, p_end, value);

    assert(value == -100);
    assert(p_last == p_end);

    --p_end;
    p_last = orcus::parse_integer(p, p_end, value);
    assert(value == -10);
    assert(p_last == p_end);

    --p_end;
    p_last = orcus::parse_integer(p, p_end, value);
    assert(value == -1);
    assert(p_last == p_end);

    test_str = "13.4";  // the parsing should end on the '.'
    p = test_str.data();
    p_end = p + test_str.size();
    p_last = orcus::parse_integer(p, p_end, value);
    assert(value == 13);
    assert(p_last == p + 2);

    // What if the p_end points to an earlier address than the p ...
    std::swap(p, p_end);
    assert(p > p_end);
    p_last = orcus::parse_integer(p, p_end, value);
    assert(p == p_last);

    // Empty char range
    p = test_str.data();
    p_end = p;
    p_last = orcus::parse_integer(p, p_end, value);
    assert(p_last == p);
}

void test_parse_double_quoted_strings()
{
    orcus::test::stack_printer __sp__(__func__);

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

void test_trim()
{
    // test for trimming.
    std::string s1("test"), s2("  test"), s3("   test  "), s4("test   ");
    std::string_view sv1(s1), sv2(s2), sv3(s3), sv4(s4);
    assert(sv1 != sv2);
    assert(sv1 != sv3);
    assert(sv2 != sv3);
    assert(sv1 != sv4);

    std::string_view trimmed = orcus::trim(sv1);
    assert(sv1 == trimmed); // nothing to trim.
    assert(sv1 == orcus::trim(sv2));
    assert(sv1 == orcus::trim(sv3));
    assert(sv1 == orcus::trim(sv4));
    assert(sv1.size() == orcus::trim(sv2).size());
    assert(sv1.size() == orcus::trim(sv3).size());
}

}

int main()
{
    test_parse_numbers();
    test_parse_integers();
    test_parse_double_quoted_strings();
    test_trim();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
