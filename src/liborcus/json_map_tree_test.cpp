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

int main()
{
    test_link_array_values();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
