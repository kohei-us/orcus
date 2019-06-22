/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/json_structure_tree.hpp"
#include "orcus/stream.hpp"

#include <vector>
#include <sstream>
#include <cassert>

using namespace orcus;

std::vector<const char*> base_dirs = {
    SRCDIR"/test/json-structure/arrays-in-object/",
    SRCDIR"/test/json-structure/nested-arrays/",
    SRCDIR"/test/json-structure/nested-arrays-mixed/",
    SRCDIR"/test/json-structure/repeat-objects/",
    SRCDIR"/test/json-structure/repeat-objects-2/",
};

void test_basic()
{
    for (const char* base_dir : base_dirs)
    {
        std::string filepath(base_dir);
        filepath.append("input.json");

        file_content strm(filepath.data());
        assert(!strm.empty());
        json::structure_tree tree;
        tree.parse(strm.data(), strm.size());
        std::ostringstream os;
        tree.dump_compact(os);
        std::string data_content = os.str();

        // Check the dump content against known datum.
        filepath = base_dir;
        filepath.append("check.txt");
        file_content strm_check(filepath.data());
        assert(!strm_check.empty());

        // They should be identical, plus or minus leading/trailing whitespaces.
        pstring s1(data_content.data(), data_content.size());
        pstring s2 = strm_check.str();
        assert(s1.trim() == s2.trim());
    }
}

int main()
{
    test_basic();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
