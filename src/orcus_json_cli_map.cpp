/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_json_cli.hpp"
#include "orcus/json_document_tree.hpp"
#include "orcus/config.hpp"

#include <iostream>

using namespace std;

namespace orcus { namespace detail {

void map_to_sheets_and_dump(std::ostream& os, const file_content& content, const cmd_params& params)
{
    cout << "TODO: implement this." << endl;
    cout << params.map_file.data() << endl;

    // Since a typical map file will likely be very small, let's be lazy and
    // load the whole thing into a in-memory tree.
    json::document_tree map_doc;
    json_config jc;
    jc.preserve_object_order = false;
    jc.persistent_string_values = false;
    jc.resolve_references = false;

    map_doc.load(params.map_file.data(), params.map_file.size(), jc);
    json::const_node root = map_doc.get_document_root();

    {
        // Create sheets first.

        auto node = root.child("sheets");
        for (size_t i = 0, n = node.child_count(); i < n; ++i)
        {
            auto node_name = node.child(i);
            cout << "* sheet: " << node_name.string_value() << endl;
        }
    }

    {
        // Set cell links.
        auto node = root.child("cells");
        for (size_t i = 0, n = node.child_count(); i < n; ++i)
        {
            auto link_node = node.child(i);
            cout << "* cell link: (path=" << link_node.child("path").string_value()
                << "; sheet=" << link_node.child("sheet").string_value()
                << "; row=" << link_node.child("row").numeric_value()
                << "; column=" << link_node.child("column").numeric_value()
                << ")" << endl;
        }
    }

    {
        // Set range links.
        auto node = root.child("ranges");
        for (size_t i = 0, n = node.child_count(); i < n; ++i)
        {
            auto link_node = node.child(i);
            cout << "* range link: (sheet=" << link_node.child("sheet").string_value()
                << "; row=" << link_node.child("row").numeric_value()
                << "; column=" << link_node.child("column").numeric_value()
                << ")" << endl;
        }
    }
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
