/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_json_cli.hpp"
#include "orcus/json_document_tree.hpp"
#include "orcus/config.hpp"

#ifdef __ORCUS_SPREADSHEET_MODEL
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/orcus_json.hpp"
#endif

#include <iostream>

using namespace std;

namespace orcus { namespace detail {

#ifdef __ORCUS_SPREADSHEET_MODEL

void map_to_sheets_and_dump(std::ostream& os, const file_content& content, const cmd_params& params)
{
    cout << "TODO: implement this." << endl;
    cout << params.map_file.data() << endl;

    spreadsheet::document doc;
    spreadsheet::import_factory factory(doc);
    orcus_json app(&factory);

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
            app.append_sheet(node_name.string_value());
        }
    }

    {
        // Set cell links.
        auto node = root.child("cells");
        for (size_t i = 0, n = node.child_count(); i < n; ++i)
        {
            auto link_node = node.child(i);
            pstring path = link_node.child("path").string_value();
            pstring sheet = link_node.child("sheet").string_value();
            spreadsheet::row_t row = link_node.child("row").numeric_value();
            spreadsheet::col_t col = link_node.child("column").numeric_value();

            cout << "* cell link: (path=" << path
                << "; sheet=" << sheet
                << "; row=" << row
                << "; column=" << col
                << ")" << endl;

            app.set_cell_link(path, sheet, row, col);
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

    app.read_stream(content.data(), content.size());
    doc.dump_check(os);
}

#else

void map_to_sheets_and_dump(
    std::ostream& /*os*/, const file_content& /*content*/, const cmd_params& /*params*/)
{
}

#endif

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
