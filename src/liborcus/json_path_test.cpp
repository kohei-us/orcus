/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include "json_path.hpp"

#include <iostream>
#include <cassert>

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

int main()
{
    test_empty();
    test_root();
    test_object_key();
    test_array_index();
    test_mix();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
