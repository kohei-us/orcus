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
    test::stack_printer __sp__(__func__);

    string output = create_parse_error_output("{}", 1);
    cout << output << endl;
    const char* expected = "1:2: {}\n      ^";
    assert(output == expected);
}

void test_stream_locate_first_different_char()
{
    test::stack_printer __sp__(__func__);

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

void test_stream_logical_string_length()
{
    test::stack_printer __sp__(__func__);

    struct check
    {
        std::string_view value;
        std::size_t length;
    };

    constexpr check checks[] = {
        { "東京", 2 },
        { "大阪は暑い", 5 },
        { "New York", 8 },
        { "日本は英語で言うとJapan", 14 },
        { "fabriqué", 8 },
        { "garçon", 6 },
        { "вход", 4 },
        { "выход", 5 },
        { "помогите", 8 },
        { "Nähe", 4 },
    };

    for (auto [value, expected_len] : checks)
    {
        std::size_t len = calc_logical_string_length(value);
        std::cout << "'" << value << "' (length=" << len << ")" << std::endl;
        assert(len == expected_len);
    }
}

void test_stream_locate_line_with_offset()
{
    test::stack_printer __sp__(__func__);

    std::string strm = "one\ntwo\nthree";

    struct check
    {
        std::ptrdiff_t offset;
        line_with_offset expected;
    };

    const std::vector<check> checks = {
        { 0, { "one", 0, 0 } },
        { 1, { "one", 0, 1 } },
        { 2, { "one", 0, 2 } },
        { 3, { "one", 0, 3 } }, // on line break
        { 4, { "two", 1, 0 } },
        { 5, { "two", 1, 1 } },
        { 6, { "two", 1, 2 } },
        { 7, { "two", 1, 3 } }, // on line break
        { 8, { "three", 2, 0 } },
        { 9, { "three", 2, 1 } },
        { 10, { "three", 2, 2 } },
        { 11, { "three", 2, 3 } },
        { 12, { "three", 2, 4 } },
    };

    for (const auto& c : checks)
    {
        auto res = locate_line_with_offset(strm, c.offset);
        assert(res == c.expected);
    }

    try
    {
        auto res = locate_line_with_offset(strm, strm.size());
        assert(!"exception should have been thrown for out-of-bound offset!");
    }
    catch (const std::invalid_argument& e)
    {
        // expected
        cout << "exception thrown as expected: '" << e.what() << "'" << endl;
    }
}

int main()
{
    test_stream_create_error_output();
    test_stream_locate_first_different_char();
    test_stream_logical_string_length();
    test_stream_locate_line_with_offset();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
