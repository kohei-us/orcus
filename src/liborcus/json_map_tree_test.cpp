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
    assert(p->type == json_map_tree::map_node_type::cell_ref);
    assert(p->value.cell_ref->pos == cell_position_t("sheet", 0, 0));

    p = tree.get_link("$[][0]");
    assert(p);
    assert(p->type == json_map_tree::map_node_type::cell_ref);
    assert(p->value.cell_ref->pos == cell_position_t("sheet", 1, 0));
}

void test_link_object_values()
{
    struct entry
    {
        const char* path;
        cell_position_t pos;
    };

    std::vector<entry> entries =
    {
        { "$[]['id']",      cell_position_t("sheet", 2, 3) },
        { "$[]['name']",    cell_position_t("sheet", 2, 4) },
        { "$[]['address']", cell_position_t("sheet", 2, 5) },
    };

    json_map_tree tree;

    for (const entry& e : entries)
        tree.set_cell_link(e.path, e.pos);

    for (const entry& e : entries)
    {
        const json_map_tree::node* p = tree.get_link(e.path);
        assert(p);
        assert(p->type == json_map_tree::map_node_type::cell_ref);
        assert(e.pos == p->value.cell_ref->pos);
    }
}

void test_link_object_root()
{
    json_map_tree tree;

    const char* path = "$['root'][2]";
    cell_position_t pos("sheet", 3, 4);
    tree.set_cell_link(path, pos);

    const json_map_tree::node* p = tree.get_link(path);
    assert(p);
    assert(p->type == json_map_tree::map_node_type::cell_ref);
    assert(p->value.cell_ref->pos == pos);
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
    assert(p->type == json_map_tree::map_node_type::range_field_ref);
    assert(p->value.range_field_ref->column_pos == 0);

    p = tree.get_link("$[][1]");
    assert(p);
    assert(p->type == json_map_tree::map_node_type::range_field_ref);
    assert(p->value.range_field_ref->column_pos == 1);

    p = tree.get_link("$[][2]");
    assert(p);
    assert(p->type == json_map_tree::map_node_type::range_field_ref);
    assert(p->value.range_field_ref->column_pos == 2);

    // Check the range reference data itself.
    const json_map_tree::range_reference_type* ref = p->value.range_field_ref->ref;
    assert(ref->fields.size() == 3);
    assert(ref->pos == pos);

    // Make sure the row group is set.
    p = tree.get_link("$[]");
    assert(p);
    assert(p->type == json_map_tree::map_node_type::array);
    assert(p->row_group == ref);
}

int main()
{
    test_link_array_values();
    test_link_object_values();
    test_link_object_root();
    test_link_range_fields();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
