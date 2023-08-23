/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/xml_structure_tree.hpp>
#include <orcus/xml_namespace.hpp>
#include <orcus/stream.hpp>
#include <orcus/parser_global.hpp>

#include <cstdlib>
#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>

#include "filesystem_env.hpp"

using namespace std;
using namespace orcus;

const fs::path test_base_dir(SRCDIR"/test/xml-structure");

std::vector<fs::path> base_dirs = {
    test_base_dir / "basic-1/",
    test_base_dir / "basic-2/",
    test_base_dir / "basic-3/",
    test_base_dir / "attribute-1/",
    test_base_dir / "nested-repeat-1/",
    test_base_dir / "namespace-default/"
};

struct loaded_tree
{
    xmlns_context cxt;
    file_content strm;
    xml_structure_tree tree;

    loaded_tree(xmlns_repository& repo) :
        cxt(repo.create_context()),
        tree(cxt) {}

    loaded_tree(const loaded_tree&) = delete;
};

std::unique_ptr<loaded_tree> load_tree(xmlns_repository& repo, fs::path& filepath)
{
    auto ret = std::make_unique<loaded_tree>(repo);
    ret->strm.load(filepath.string().data());
    assert(!ret->strm.empty());
    xml_structure_tree tree(ret->cxt);
    tree.parse(ret->strm.str());
    ret->tree.swap(tree);

    return ret;
}

void test_basic()
{
    xmlns_repository xmlns_repo;

    for (const fs::path& base_dir : base_dirs)
    {
        fs::path filepath = base_dir / "input.xml";
        cout << filepath << endl;
        auto lt = load_tree(xmlns_repo, filepath);

        std::ostringstream os;
        lt->tree.dump_compact(os);
        string data_content = os.str();
        cout << "--" << endl;
        cout << data_content;

        // Check the dump content against known datum.
        filepath = base_dir / "check.txt";
        file_content strm_check(filepath.string().data());
        assert(!strm_check.empty());

        // They should be identical, plus or minus leading/trailing whitespaces.
        std::string_view s1(data_content.data(), data_content.size());
        std::string_view s2 = strm_check.str();
        assert(trim(s1) == trim(s2));
    }
}

void test_walker()
{
    xmlns_repository xmlns_repo;

    {
        fs::path filepath = base_dirs[0] / "input.xml";
        auto lt = load_tree(xmlns_repo, filepath);

        // Get walker from the tree.
        xml_structure_tree::entity_names_type elem_names;
        xml_structure_tree::walker wkr = lt->tree.get_walker();

        // Root element.
        xml_structure_tree::element elem = wkr.root();
        assert(elem.name.name == "root");
        assert(!elem.repeat);

        // Get names of child elements. There should only one one and it should be 'entry'.
        elem_names = wkr.get_children();
        assert(elem_names.size() == 1);
        xml_structure_tree::entity_name elem_name = elem_names.front();
        assert(elem_name.name == "entry");

        // Descend into 'entry'.
        elem = wkr.descend(elem_name);
        assert(elem.name.name == "entry");
        assert(elem.repeat);

        // Element 'entry' should have 2 child elements 'name' and 'id' in this order.
        elem_names = wkr.get_children();
        assert(elem_names.size() == 2);
        assert(elem_names[0].name == "name");
        assert(elem_names[1].name == "id");

        // Descend into 'name'.
        elem_name = elem_names[0];
        elem = wkr.descend(elem_name);
        assert(elem.name.name == "name");
        assert(!elem.repeat);

        // This is a leaf element. It should have no child elements.
        xml_structure_tree::entity_names_type test_names = wkr.get_children();
        assert(test_names.empty());

        // Move up to 'entry'.
        elem = wkr.ascend();
        assert(elem.name.name == "entry");
        assert(elem.repeat);

        // Move down to 'id'.
        elem = wkr.descend(elem_names[1]);
        assert(elem.name.name == "id");
        assert(!elem.repeat);

        // Move up to 'entry' again.
        elem = wkr.ascend();
        assert(elem.name.name == "entry");
        assert(elem.repeat);

        // Move up to 'root'.
        elem = wkr.ascend();
        assert(elem.name.name == "root");
        assert(!elem.repeat);
    }

    {
        fs::path filepath = base_dirs[3] / "input.xml"; // attribute-1
        auto lt = load_tree(xmlns_repo, filepath);

        // Get walker from the tree.
        xml_structure_tree::entity_names_type elem_names;
        xml_structure_tree::walker wkr = lt->tree.get_walker();

        // Root element.
        xml_structure_tree::element elem = wkr.root();
        assert(elem.name.name == "root");
        assert(!elem.repeat);

        // Check attributes of root, which should have 'version' and 'type' in this order.
        xml_structure_tree::entity_names_type names = wkr.get_attributes();
        assert(names.size() == 2);
        assert(names[0].name == "version");
        assert(names[1].name == "type");

        // Root element should have only one child element 'entry'.
        names = wkr.get_children();
        assert(names.size() == 1);
        assert(names[0].name == "entry");
        elem = wkr.descend(names[0]);
        assert(elem.name.name == "entry");
        assert(elem.repeat);

        // The 'entry' element should have 3 attributes 'attr1', 'attr2', and 'attr3'.
        names = wkr.get_attributes();
        assert(names.size() == 3);
        assert(names[0].name == "attr1");
        assert(names[1].name == "attr2");
        assert(names[2].name == "attr3");
    }
}

void test_walker_path()
{
    fs::path filepath = base_dirs[0] / "input.xml";
    xmlns_repository xmlns_repo;
    auto lt = load_tree(xmlns_repo, filepath);

    // Get walker from the tree.
    xml_structure_tree::entity_names_type elem_names;
    xml_structure_tree::walker wkr = lt->tree.get_walker();
    wkr.root();
    assert("/root" == wkr.get_path());

    elem_names = wkr.get_children();
    xml_structure_tree::entity_name elem_name = elem_names.front();
    wkr.descend(elem_name);
    assert("/root/entry" == wkr.get_path());

    xml_structure_tree::element element = wkr.move_to("/root/entry");
    assert(element.name.name == "entry");

    try
    {
        wkr.move_to("/root/not-there");
        assert(false);
    }
    catch (...)
    {
    }

    try
    {
        wkr.move_to("something_different");
        assert(false);
    }
    catch (...)
    {
    }

    try
    {
        wkr.move_to("/non-exist");
        assert(false);
    }
    catch (...)
    {
    }
}

void test_element_contents()
{
    xmlns_repository xmlns_repo;

    {
        fs::path filepath = test_base_dir / "attribute-1" / "input.xml";
        auto lt = load_tree(xmlns_repo, filepath);
        auto wkr = lt->tree.get_walker();
        auto elem = wkr.move_to("/root/entry");
        assert(wkr.to_string(elem.name) == "entry");
        assert(elem.repeat);
        assert(!elem.has_content);
    }

    {
        fs::path filepath = test_base_dir / "basic-1" / "input.xml";
        auto lt = load_tree(xmlns_repo, filepath);
        auto wkr = lt->tree.get_walker();
        auto elem = wkr.move_to("/root/entry/name");
        assert(wkr.to_string(elem.name) == "name");
        assert(!elem.repeat);
        assert(elem.has_content);

        elem = wkr.move_to("/root/entry/id");
        assert(wkr.to_string(elem.name) == "id");
        assert(!elem.repeat);
        assert(elem.has_content);
    }

    {
        fs::path filepath = test_base_dir / "nested-repeat-1" / "input.xml";
        auto lt = load_tree(xmlns_repo, filepath);
        auto wkr = lt->tree.get_walker();
        auto elem = wkr.move_to("/root/mode/insert/command");
        assert(wkr.to_string(elem.name) == "command");
        assert(!elem.repeat);
        assert(!elem.has_content);
    }
}

int main()
{
    test_basic();
    test_walker();
    test_walker_path();
    test_element_contents();

    return EXIT_SUCCESS;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
