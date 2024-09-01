/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"

#include <orcus/spreadsheet/auto_filter.hpp>
#include <iostream>

namespace ss = orcus::spreadsheet;

void test_filter_value()
{
    ORCUS_TEST_FUNC_SCOPE;

    ss::filter_value_t empty;
    assert(empty.type() == ss::filter_value_t::value_type::empty);

    ss::filter_value_t numeric(345.0);
    assert(numeric.type() == ss::filter_value_t::value_type::numeric);
    assert(numeric.numeric() == 345.0);

    ss::filter_value_t str("some string");
    assert(str.type() == ss::filter_value_t::value_type::string);
    assert(str.string() == "some string");

    str = ss::filter_value_t{"other string"};
    assert(str.type() == ss::filter_value_t::value_type::string);
    assert(str.string() == "other string");

    assert(numeric == ss::filter_value_t{345.0});
    assert(str != empty);
    assert(str != numeric);
    str = empty;
    assert(str.type() == ss::filter_value_t::value_type::empty);
    assert(str == empty);
}

void test_filter_node()
{
    ORCUS_TEST_FUNC_SCOPE;

    ss::filter_node_t root{ss::auto_filter_node_op_t::op_or};

    // x > 40 or x < 5 or (12 <= x <= 24)

    ss::col_t field = 1;
    root.item_store.emplace_back(field, ss::auto_filter_op_t::greater, 40.0);
    root.children.push_back(&root.item_store.back());

    root.item_store.emplace_back(field, ss::auto_filter_op_t::less, 5.0);
    root.children.push_back(&root.item_store.back());

    {
        root.node_store.emplace_back(ss::auto_filter_node_op_t::op_and);
        auto& node = root.node_store.back();

        node.item_store.emplace_back(field, ss::auto_filter_op_t::greater_equal, 12.0);
        node.children.push_back(&node.item_store.back());

        node.item_store.emplace_back(field, ss::auto_filter_op_t::less_equal, 24.0);
        node.children.emplace_back(&node.item_store.back());

        root.children.push_back(&node);
    }
}

int main()
{
    test_filter_value();
    test_filter_node();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

