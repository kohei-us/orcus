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

std::vector<orcus::json_path_part_t> parse_json_path(std::string_view exp)
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
    assert(tokens.size() == 1);
    assert(tokens[0].type() == orcus::json_path_t::root);
}

void test_object_key()
{
    ORCUS_TEST_FUNC_SCOPE;

    {
        auto tokens = parse_json_path("$.key");

        assert(tokens.size() == 2);
        assert(tokens[0].type() == orcus::json_path_t::root);
        assert(tokens[1].type() == orcus::json_path_t::object_key);
        assert(tokens[1].object_key() == "key");
    }

    {
        auto tokens = parse_json_path("$.key1.key2");

        assert(tokens.size() == 3);
        assert(tokens[0].type() == orcus::json_path_t::root);
        assert(tokens[1].type() == orcus::json_path_t::object_key);
        assert(tokens[1].object_key() == "key1");
        assert(tokens[2].type() == orcus::json_path_t::object_key);
        assert(tokens[2].object_key() == "key2");
    }
}

void test_array_index()
{
    ORCUS_TEST_FUNC_SCOPE;

    {
        auto tokens = parse_json_path("$[0]");

        assert(tokens.size() == 2);
        assert(tokens[0].type() == orcus::json_path_t::root);
        assert(tokens[1].type() == orcus::json_path_t::array_index);
        assert(tokens[1].array_index() == 0);
    }

    {
        auto tokens = parse_json_path("$[0][2]");

        assert(tokens.size() == 3);
        assert(tokens[0].type() == orcus::json_path_t::root);
        assert(tokens[1].type() == orcus::json_path_t::array_index);
        assert(tokens[1].array_index() == 0);
        assert(tokens[2].type() == orcus::json_path_t::array_index);
        assert(tokens[2].array_index() == 2);
    }

    {
        auto tokens = parse_json_path("$[0][2][12]");

        assert(tokens.size() == 4);
        assert(tokens[0].type() == orcus::json_path_t::root);
        assert(tokens[1].type() == orcus::json_path_t::array_index);
        assert(tokens[1].array_index() == 0);
        assert(tokens[2].type() == orcus::json_path_t::array_index);
        assert(tokens[2].array_index() == 2);
        assert(tokens[3].type() == orcus::json_path_t::array_index);
        assert(tokens[3].array_index() == 12);
    }
}

int main()
{
    test_empty();
    test_root();
    test_object_key();
    test_array_index();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
