/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include "xml_map_tree.hpp"
#include "orcus/xml_namespace.hpp"

#include <cstdlib>
#include <cassert>
#include <iostream>

using namespace orcus;

void test_path_insertion()
{
    ORCUS_TEST_FUNC_SCOPE;

    xmlns_repository repo;
    xml_map_tree tree(repo);
    xml_map_tree::cell_position ref;
    ref.sheet = std::string_view{"test"};
    ref.row = 2;
    ref.col = 1;

    // Single cell links
    tree.set_cell_link("/data/elem1", ref);
    const xml_map_tree::linkable* p0 = tree.get_link("/data/elem1");
    assert(p0 && p0->node_type == xml_map_tree::linkable_node_type::element);
    const xml_map_tree::element* p = static_cast<const xml_map_tree::element*>(p0);
    assert(p->ref_type == xml_map_tree::reference_type::cell);
    assert(p->cell_ref->pos.sheet == "test");
    assert(p->cell_ref->pos.row == 2);
    assert(p->cell_ref->pos.col == 1);

    const xml_map_tree::element* elem1 = p;

    ref.row = 3;
    ref.col = 2;
    tree.set_cell_link("/data/elem2", ref);
    p0 = tree.get_link("/data/elem2");
    assert(p0 && p0->node_type == xml_map_tree::linkable_node_type::element);
    p = static_cast<const xml_map_tree::element*>(p0);
    assert(p && p->ref_type == xml_map_tree::reference_type::cell);
    assert(p->cell_ref->pos.sheet == "test");
    assert(p->cell_ref->pos.row == 3);
    assert(p->cell_ref->pos.col == 2);

    // The link in elem1 should be unchanged.
    p0 = tree.get_link("/data/elem1");
    assert(p0 == elem1);

    ref.sheet = std::string_view{"test2"};
    ref.row = 10;
    ref.col = 5;
    tree.set_cell_link("/data/meta/title", ref);
    p0 = tree.get_link("/data/meta/title");
    assert(p0 && p0->node_type == xml_map_tree::linkable_node_type::element);
    p = static_cast<const xml_map_tree::element*>(p0);
    assert(p && p->ref_type == xml_map_tree::reference_type::cell);
    assert(p->cell_ref->pos.sheet == "test2");
    assert(p->cell_ref->pos.row == 10);
    assert(p->cell_ref->pos.col == 5);

    // Range field links
    ref.row = 5;
    ref.col = 0;
    ref.sheet = std::string_view{"test3"};
    tree.start_range(ref);
    tree.append_range_field_link("/data/entries/entry/id", std::string_view{});
    tree.append_range_field_link("/data/entries/entry/name", std::string_view{});
    tree.append_range_field_link("/data/entries/entry/score", std::string_view{});
    tree.set_range_row_group("/data/entries/entry");
    tree.commit_range();
    p0 = tree.get_link("/data/entries/entry/id");
    assert(p0 && p0->node_type == xml_map_tree::linkable_node_type::element);
    p = static_cast<const xml_map_tree::element*>(p0);
    assert(p && p->ref_type == xml_map_tree::reference_type::range_field);
    assert(p->field_ref->ref->pos.sheet == "test3");
    assert(p->field_ref->ref->pos.row == 5);
    assert(p->field_ref->ref->pos.col == 0);
    assert(p->field_ref->column_pos == 0);

    p0 = tree.get_link("/data/entries/entry/name");
    assert(p0 && p0->node_type == xml_map_tree::linkable_node_type::element);
    p = static_cast<const xml_map_tree::element*>(p0);
    assert(p && p->ref_type == xml_map_tree::reference_type::range_field);
    assert(p->field_ref->ref->pos.sheet == "test3");
    assert(p->field_ref->ref->pos.row == 5);
    assert(p->field_ref->ref->pos.col == 0);
    assert(p->field_ref->column_pos == 1);

    p0 = tree.get_link("/data/entries/entry/score");
    assert(p0 && p0->node_type == xml_map_tree::linkable_node_type::element);
    p = static_cast<const xml_map_tree::element*>(p0);
    assert(p && p->ref_type == xml_map_tree::reference_type::range_field);
    assert(p->field_ref->ref->pos.sheet == "test3");
    assert(p->field_ref->ref->pos.row == 5);
    assert(p->field_ref->ref->pos.col == 0);
    assert(p->field_ref->column_pos == 2);
}

void test_attr_path_insertion()
{
    ORCUS_TEST_FUNC_SCOPE;

    xmlns_repository repo;
    xml_map_tree tree(repo);
    xml_map_tree::cell_position ref;
    ref.sheet = std::string_view{"test"};
    ref.row = 2;
    ref.col = 3;

    // 'attr1' is an attribute of 'elem'.
    tree.set_cell_link("/root/elem/@attr1", ref);
    const xml_map_tree::linkable* p = tree.get_link("/root/elem/@attr1");
    assert(p && p->node_type == xml_map_tree::linkable_node_type::attribute);
    const xml_map_tree::attribute* attr = static_cast<const xml_map_tree::attribute*>(p);
    assert(attr->ref_type == xml_map_tree::reference_type::cell);
    assert(attr->cell_ref->pos.sheet == "test");
    assert(attr->cell_ref->pos.row == 2);
    assert(attr->cell_ref->pos.col == 3);

    // Insert another attribute in the same element.
    ref.sheet = std::string_view{"test2"};
    ref.row = 11;
    ref.col = 4;
    tree.set_cell_link("/root/elem/@attr2", ref);
    p = tree.get_link("/root/elem/@attr2");
    assert(p && p->node_type == xml_map_tree::linkable_node_type::attribute);
    attr = static_cast<const xml_map_tree::attribute*>(p);
    assert(attr->ref_type == xml_map_tree::reference_type::cell);
    assert(attr->cell_ref->pos.sheet == "test2");
    assert(attr->cell_ref->pos.row == 11);
    assert(attr->cell_ref->pos.col == 4);

    // At this point, /root/elem is not linked.
    p = tree.get_link("/root/elem");
    assert(!p);

    // Now, link /root/elem.
    ref.sheet = std::string_view{"test3"};
    ref.row = 4;
    ref.col = 6;
    tree.set_cell_link("/root/elem", ref);
    p = tree.get_link("/root/elem");
    assert(p && p->node_type == xml_map_tree::linkable_node_type::element);
    const xml_map_tree::element* elem = static_cast<const xml_map_tree::element*>(p);
    assert(elem->elem_type == xml_map_tree::element_type::linked);
    assert(elem->ref_type == xml_map_tree::reference_type::cell);
    assert(elem->cell_ref->pos.sheet == "test3");
    assert(elem->cell_ref->pos.row == 4);
    assert(elem->cell_ref->pos.col == 6);
}

void test_tree_walk()
{
    ORCUS_TEST_FUNC_SCOPE;

    xmlns_repository repo;
    xml_map_tree tree(repo);
    xml_map_tree::cell_position ref;
    ref.sheet = std::string_view{"test"};
    ref.row = 2;
    ref.col = 1;

    tree.set_cell_link("/data/header/title", ref);
    xml_map_tree::walker walker = tree.get_tree_walker();
    walker.reset();

    // Root element.
    const xml_map_tree::element* elem = walker.push_element({XMLNS_UNKNOWN_ID, "data"});
    assert(elem);
    assert(elem->name.name == "data");
    assert(elem->elem_type == xml_map_tree::element_type::unlinked);

    elem = walker.push_element({XMLNS_UNKNOWN_ID, "header"});
    assert(elem);
    assert(elem->name.name == "header");
    assert(elem->elem_type == xml_map_tree::element_type::unlinked);

    elem = walker.push_element({XMLNS_UNKNOWN_ID, "title"});
    assert(elem);
    assert(elem->name.name == "title");
    assert(elem->ref_type == xml_map_tree::reference_type::cell);

    elem = walker.pop_element({XMLNS_UNKNOWN_ID, "title"});
    assert(elem);
    assert(elem->name.name == "header");
    assert(elem->elem_type == xml_map_tree::element_type::unlinked);

    elem = walker.pop_element({XMLNS_UNKNOWN_ID, "header"});
    assert(elem);
    assert(elem->name.name == "data");
    assert(elem->elem_type == xml_map_tree::element_type::unlinked);

    elem = walker.pop_element({XMLNS_UNKNOWN_ID, "data"});
    assert(!elem);
}

void test_tree_walk_namespace()
{
    ORCUS_TEST_FUNC_SCOPE;

    xmlns_repository repo;
    xml_map_tree tree(repo);
    xml_map_tree::cell_position ref;
    ref.sheet = std::string_view{"data"};
    ref.row = 1;
    ref.col = 2;

    tree.set_namespace_alias("a", "http://some-namespace", false);
    tree.set_namespace_alias("skip", "http://namespace-to-skip", false);
    tree.set_cell_link("/a:table/a:title", ref);
    tree.start_range(ref);
    ref.row = 2;
    ref.col = 0;
    tree.append_range_field_link("/a:table/a:rows/a:row/a:city", std::string_view{});
    ++ref.col;
    tree.append_range_field_link("/a:table/a:rows/a:row/a:population", std::string_view{});
    ++ref.col;
    tree.append_range_field_link("/a:table/a:rows/a:row/a:year", std::string_view{});
    tree.set_range_row_group("/a:table/a:rows/a:row");
    tree.commit_range();

    xmlns_id_t ns_a = tree.get_namespace("a");
    assert(ns_a != XMLNS_UNKNOWN_ID);
    xmlns_id_t ns_skip = tree.get_namespace("skip");
    assert(ns_skip != XMLNS_UNKNOWN_ID);

    xml_map_tree::walker walker = tree.get_tree_walker();
    walker.reset();

    // Root element.  This is not linked.
    const xml_map_tree::element* elem = walker.push_element({ns_a, "table"});
    assert(elem);
    assert(elem->name.ns == ns_a);
    assert(elem->name.name == "table");
    assert(elem->node_type == xml_map_tree::linkable_node_type::element);
    assert(elem->elem_type == xml_map_tree::element_type::unlinked);
    assert(elem->ref_type == xml_map_tree::reference_type::unknown);

    // Intentionally push a foreign element.
    const xml_map_tree::element* elem_old = elem;
    elem = walker.push_element({ns_skip, "foo"});
    assert(!elem);
    elem = walker.pop_element({ns_skip, "foo"});
    assert(elem == elem_old);

    // Push a foreign element and a valid element under it.  A valid element
    // placed under a foreign element should be invalid.
    elem_old = elem;
    elem = walker.push_element({ns_skip, "foo"});
    assert(!elem);
    elem = walker.push_element({ns_a, "title"});
    assert(!elem);
    elem = walker.pop_element({ns_a, "title"});
    assert(!elem);
    elem = walker.pop_element({ns_skip, "foo"});
    assert(elem == elem_old);
}

int main()
{
    test_path_insertion();
    test_attr_path_insertion();
    test_tree_walk();
    test_tree_walk_namespace();
    return EXIT_SUCCESS;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
