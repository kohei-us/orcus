/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include "json_path.hpp"

#include <orcus/exception.hpp>

#include <iostream>
#include <cassert>
#include <vector>

namespace {

orcus::json_path_parts_t parse_json_path(std::string_view exp)
{
    std::cout << "expression: '" << exp << "'" << std::endl;

    auto parser = orcus::json_path_parser();
    parser.parse(exp);
    return parser.pop_parts();
}

} // anonymous namespace

void test_empty()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto tokens = parse_json_path("");
    assert(tokens.empty());
}

void test_root()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto tokens = parse_json_path("$");

    orcus::json_path_parts_t expected = {
        { orcus::json_path_t::root }
    };

    assert(tokens == expected);
}

void test_object_key()
{
    ORCUS_TEST_FUNC_SCOPE;

    {
        auto tokens = parse_json_path("$.key");

        orcus::json_path_parts_t expected = {
            { orcus::json_path_t::root },
            { "key" },
        };

        assert(tokens == expected);
    }

    {
        auto tokens = parse_json_path("$.key1.key2");

        orcus::json_path_parts_t expected = {
            { orcus::json_path_t::root },
            { "key1" },
            { "key2" },
        };

        assert(tokens == expected);
    }
}

void test_object_key_in_brackets()
{
    ORCUS_TEST_FUNC_SCOPE;

    {
        auto tokens = parse_json_path("$['key']");

        orcus::json_path_parts_t expected = {
            { orcus::json_path_t::root },
            { "key" },
        };

        assert(tokens == expected);
    }

    {
        auto tokens = parse_json_path("$['key'][2]['key.with.dot']");

        orcus::json_path_parts_t expected = {
            { orcus::json_path_t::root },
            { "key" },
            { 2 },
            { "key.with.dot" },
        };

        assert(tokens == expected);
    }

    {
        auto tokens = parse_json_path("$['key'][2]['key.with.dot'].more-key");

        orcus::json_path_parts_t expected = {
            { orcus::json_path_t::root },
            { "key" },
            { 2 },
            { "key.with.dot" },
            { "more-key" },
        };

        assert(tokens == expected);
    }
}

void test_array_index()
{
    ORCUS_TEST_FUNC_SCOPE;

    {
        auto tokens = parse_json_path("$[0]");

        orcus::json_path_parts_t expected = {
            { orcus::json_path_t::root },
            { 0 },
        };

        assert(tokens == expected);
    }

    {
        auto tokens = parse_json_path("$[0][2]");

        orcus::json_path_parts_t expected = {
            { orcus::json_path_t::root },
            { 0 },
            { 2 },
        };

        assert(tokens == expected);
    }

    {
        auto tokens = parse_json_path("$[0][2][12]");

        orcus::json_path_parts_t expected = {
            { orcus::json_path_t::root },
            { 0 },
            { 2 },
            { 12 },
        };

        assert(tokens == expected);
    }

    {
        auto tokens = parse_json_path("$[*]");

        orcus::json_path_parts_t expected = {
            { orcus::json_path_t::root },
            { orcus::json_path_t::array_all },
        };

        assert(tokens == expected);
    }
}

void test_mix()
{
    ORCUS_TEST_FUNC_SCOPE;

    {
        auto tokens = parse_json_path("$.key1[5].key2");

        orcus::json_path_parts_t expected = {
            { orcus::json_path_t::root },
            { "key1" },
            { 5 },
            { "key2" },
        };

        assert(tokens == expected);
    }

    {
        auto tokens = parse_json_path("$[5].key1[11][12].key2");

        orcus::json_path_parts_t expected = {
            { orcus::json_path_t::root },
            { 5 },
            { "key1" },
            { 11 },
            { 12 },
            { "key2" },
        };

        assert(tokens == expected);
    }
}

void test_no_root()
{
    ORCUS_TEST_FUNC_SCOPE;

    {
        auto tokens = parse_json_path("key1");

        orcus::json_path_parts_t expected = {
            { "key1" },
        };

        assert(tokens == expected);
    }

    {
        auto tokens = parse_json_path("key1.key2");

        orcus::json_path_parts_t expected = {
            { "key1" },
            { "key2" },
        };

        assert(tokens == expected);
    }

    {
        auto tokens = parse_json_path("[2].key");

        orcus::json_path_parts_t expected = {
            { 2 },
            { "key" },
        };

        assert(tokens == expected);
    }
}

void test_invalids()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::vector<std::string_view> invalids = {
        "$.",
        "$.key.",
        "$.key..",
        "$[",
        "$[]",
        "$]",
        "$a",
        "$.key['another key",
        "$.key['another key'[0]",
        ".",
        ".key",
    };

    for (const auto& invalid : invalids)
    {
        try
        {
            [[maybe_unused]] auto tokens = parse_json_path(invalid);
            assert(!"an exception was expected but was not thrown");
        }
        catch (const orcus::invalid_arg_error& e)
        {
            // good
            std::cout << "exception: '" << e.what() << "'" << std::endl;
        }
    }
}

int main()
{
    test_empty();
    test_root();
    test_object_key();
    test_object_key_in_brackets();
    test_array_index();
    test_mix();
    test_no_root();
    test_invalids();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
