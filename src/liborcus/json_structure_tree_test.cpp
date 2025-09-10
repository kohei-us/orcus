/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include <orcus/json_structure_tree.hpp>
#include <orcus/stream.hpp>
#include <orcus/parser_global.hpp>
#include "filesystem_env.hpp"

#include <vector>
#include <sstream>
#include <cassert>
#include <unordered_set>

using namespace orcus;

std::vector<const char*> base_dirs = {
    SRCDIR"/test/json-structure/arrays-in-object/",
    SRCDIR"/test/json-structure/nested-arrays/",
    SRCDIR"/test/json-structure/nested-arrays-mixed/",
    SRCDIR"/test/json-structure/nested-arrays-mixed-2/",
    SRCDIR"/test/json-structure/repeat-objects/",
    SRCDIR"/test/json-structure/repeat-objects-2/",
    SRCDIR"/test/json-structure/multiple-ranges/",
};

/**
 * All json contents under this directory have no value nodes. Since the
 * structure output of a JSON content only dumps value nodes, the output
 * string should be empty when the source content does not have any value
 * nodes.
 */
void test_no_value_nodes()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path base_dir(SRCDIR"/test/json-structure/no-value-nodes");

    for (const fs::path& p : fs::directory_iterator(base_dir))
    {
        if (!fs::is_regular_file(p))
            continue;

        if (p.extension().string() != ".json")
            continue;

        file_content strm(p.string().data());
        json::structure_tree tree;
        tree.parse(strm.str());
        tree.normalize_tree();
        std::ostringstream os;
        tree.dump_compact(os);

        assert(os.str().empty());
    }
}

void test_basic()
{
    ORCUS_TEST_FUNC_SCOPE;

    for (const char* base_dir : base_dirs)
    {
        std::string filepath(base_dir);
        filepath.append("input.json");

        file_content strm(filepath.data());
        assert(!strm.empty());
        json::structure_tree tree;
        tree.parse(strm.str());
        tree.normalize_tree();
        std::ostringstream os;
        tree.dump_compact(os);
        std::string data_content = os.str();

        // Check the dump content against known datum.
        filepath = base_dir;
        filepath.append("check.txt");
        file_content strm_check(filepath.data());
        assert(!strm_check.empty());

        // They should be identical, plus or minus leading/trailing whitespaces.
        std::string_view s1(data_content.data(), data_content.size());
        std::string_view s2 = strm_check.str();
        assert(trim(s1) == trim(s2));
    }
}

void test_automatic_range_detection()
{
    ORCUS_TEST_FUNC_SCOPE;

    using detected_group_type = std::unordered_set<std::string>;
    using detected_groups_type = std::vector<detected_group_type>;

    struct check
    {
        fs::path filepath;
        detected_groups_type expected_groups;
    };

    std::vector<check> checks =
    {
        {
            SRCDIR"/test/json-structure/arrays-in-object/input.json",
            {
                {
                    "row-group:$['rows']",
                    "path:$['rows'][]['name']",
                    "path:$['rows'][]['age']",
                    "path:$['rows'][]['error']",
                }
            }
        },
        {
            SRCDIR"/test/json-structure/repeat-objects/input.json",
            {
                {
                    "path:$[]['name']",
                    "path:$[]['age']",
                    "row-group:$",
                }
            }
        },
        {
            SRCDIR"/test/json-structure/repeat-objects-2/input.json",
            {
                {
                    "path:$[]['name']",
                    "path:$[]['age']",
                    "path:$[]['props']['alpha']",
                    "path:$[]['props']['beta']",
                    "path:$[]['props']['gamma']",
                    "path:$[]['props']['theta']",
                    "row-group:$",
                }
            }
        },
        {
            SRCDIR"/test/json-structure/multiple-ranges/input.json",
            {
                {
                    "path:$['data'][]['category']",
                    "path:$['data'][]['region']",
                    "path:$['data'][]['records'][]['id']",
                    "path:$['data'][]['records'][]['ref']",
                    "row-group:$['data']",
                    "row-group:$['data'][]['records']",
                },
                {
                    "path:$['misc'][][0]",
                    "path:$['misc'][][1]",
                    "path:$['misc'][][2]",
                    "row-group:$['misc']",
                }
            }
        },
    };

    for (const check& c : checks)
    {
        file_content strm(c.filepath.string().data());
        assert(!strm.empty());
        json::structure_tree tree;
        tree.parse(strm.str());

        detected_groups_type observed_groups;

        json::structure_tree::range_handler_type rh = [&observed_groups](json::table_range_t&& range)
        {
            detected_group_type observed;
            for (const std::string& s : range.row_groups)
            {
                std::ostringstream os;
                os << "row-group:" << s;
                observed.insert(os.str());
            }

            for (const std::string& s : range.paths)
            {
                std::ostringstream os;
                os << "path:" << s;
                observed.insert(os.str());
            }

            observed_groups.push_back(std::move(observed));
        };

        tree.process_ranges(rh);

        assert(observed_groups == c.expected_groups);
    }
}

void test_callback()
{
    ORCUS_TEST_FUNC_SCOPE;

    {
        fs::path input{SRCDIR"/test/json-structure/arrays-in-object/input.json"};
        std::cout << input << std::endl;

        orcus::file_content strm{input.string()};

        std::size_t n_called = 0;

        json::structure_tree tree;
        tree.set_callback(
            json::structure_tree::callback_type::on_repeat_node,
            [&n_called](std::any arg) {
                auto nt = std::any_cast<json::structure_tree::node_type>(arg);
                assert(nt == json::structure_tree::node_type::object);
                ++n_called;
            }
        );
        tree.parse(strm.str());

        // It contains a list of 4 objects and the first occurrence doesn't
        // trigger a repeat callback.  So the callback should be called 3 times.
        assert(n_called == 3);
    }

    {
        // this test file contains both repeated objects and arrays.
        fs::path input{SRCDIR"/test/json-structure/multiple-ranges/input.json"};
        std::cout << input << std::endl;

        orcus::file_content strm{input.string()};

        std::unordered_map<json::structure_tree::node_type, std::size_t> n_types = {
            { json::structure_tree::node_type::object, 0 },
            { json::structure_tree::node_type::array, 0 },
        };

        json::structure_tree tree;
        tree.set_callback(
            json::structure_tree::callback_type::on_repeat_node,
            [&n_types](std::any arg) {
                auto nt = std::any_cast<json::structure_tree::node_type>(arg);
                ++n_types[nt];
            }
        );
        tree.parse(strm.str());

        assert(n_types.size() == 2);

        // "data" section contains 7 outer objects and 16 inner objects. Since
        // the first occurrence doesn't trigger a repeat callback, the callback
        // should be called (7-1)+(16-1) -> 21 times on the object type.
        assert(n_types[json::structure_tree::node_type::object] == 21);

        // "misc" section contains a list of 2 repeated arrays.  The callback
        // should be called once only on the second occurrence.
        assert(n_types[json::structure_tree::node_type::array] == 1);
    }
}

int main()
{
    test_no_value_nodes();
    test_basic();
    test_automatic_range_detection();
    test_callback();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
