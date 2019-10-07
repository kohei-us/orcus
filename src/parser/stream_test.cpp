/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include "orcus/stream.hpp"

#include <cstdlib>
#include <string>
#include <vector>

using namespace std;
using namespace orcus;

void test_stream_create_error_output()
{
    string output = create_parse_error_output("{}", 1);
    cout << output << endl;
    const char* expected = "1:2: {}\n      ^";
    assert(output == expected);
}

void test_stream_locate_first_different_char()
{
    struct test_case
    {
        const char* left;
        const char* right;
        size_t expected;
    };

    std::vector<test_case> test_cases = {
        { "", "a", 0 },
        { "a", "", 0 },
        { "", "", 0 },
        { " ", "b", 0 },
        { "abc", "abc", 3 },
        { "abcd", "abce", 3 },
        { "abc", "bbc", 0 },
        { "abc", "acc", 1 },
    };

    for (const test_case& tc : test_cases)
    {
        size_t actual = locate_first_different_char(tc.left, tc.right);
        assert(actual == tc.expected);
    }
}

int main()
{
    test_stream_create_error_output();
    test_stream_locate_first_different_char();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
