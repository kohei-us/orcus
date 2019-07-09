/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "json_map_tree.hpp"

#include <cassert>
#include <iostream>

using namespace orcus;
using namespace std;

void test_link_array_values()
{
    json_map_tree tree;

    cell_position_t pos("sheet", 0, 0);

    tree.set_cell_link("$[0]", pos);
    pos.row = 1;
    tree.set_cell_link("$[][0]", pos);

    const json_map_tree::node* p = tree.get_link("$[0]");
    assert(p);
    assert(p->type == json_map_tree::node_type::cell_ref);
    assert(p->value.cell_ref->pos == cell_position_t("sheet", 0, 0));

    p = tree.get_link("$[][0]");
    assert(p);
    assert(p->type == json_map_tree::node_type::cell_ref);
    assert(p->value.cell_ref->pos == cell_position_t("sheet", 1, 0));
}

void test_link_range_fields()
{
    json_map_tree tree;

    cell_position_t pos("sheet", 1, 2);

    tree.start_range(pos);
    tree.append_field_link("$[][0]");
    tree.append_field_link("$[][1]");
    tree.append_field_link("$[][2]");
    tree.set_range_row_group("$[]");
    tree.commit_range();

    const json_map_tree::node* p = tree.get_link("$[][0]");
    assert(p);
    assert(p->type == json_map_tree::node_type::range_field_ref);
    assert(p->value.range_field_ref->column_pos == 0);

    p = tree.get_link("$[][1]");
    assert(p);
    assert(p->type == json_map_tree::node_type::range_field_ref);
    assert(p->value.range_field_ref->column_pos == 1);

    p = tree.get_link("$[][2]");
    assert(p);
    assert(p->type == json_map_tree::node_type::range_field_ref);
    assert(p->value.range_field_ref->column_pos == 2);

    // Check the range reference data itself.
    const json_map_tree::range_reference_type* ref = p->value.range_field_ref->ref;
    assert(ref->fields.size() == 3);
    assert(ref->pos == pos);
}

int main()
{
    test_link_array_values();
    test_link_range_fields();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
