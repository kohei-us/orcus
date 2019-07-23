/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_json_cli.hpp"
#include "orcus/json_document_tree.hpp"
#include "orcus/json_structure_tree.hpp"
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

namespace {

void traverse(json::structure_tree::walker& walker)
{
    json::structure_tree::node_properties node = walker.get_node();
    std::cout << __FILE__ << "#" << __LINE__ << " (detail:traverse): " << node.type << std::endl;

    switch (node.type)
    {
        case json::structure_tree::node_type::array:
        case json::structure_tree::node_type::object:
        case json::structure_tree::node_type::object_key:
        {
            for (size_t i = 0, n = walker.child_count(); i < n; ++i)
            {
                walker.descend(i);
                traverse(walker);
                walker.ascend();
            }
            break;
        }
        case json::structure_tree::node_type::value:
            break;
        case json::structure_tree::node_type::unknown:
            break;
    }
}

} // anonymous namespace

void map_to_sheets_and_dump(const file_content& content, cmd_params& params)
{
    spreadsheet::document doc;
    spreadsheet::import_factory factory(doc);
    orcus_json app(&factory);

    if (params.map_file.empty())
    {
        json::structure_tree structure;
        structure.parse(content.data(), content.size());
        structure.dump_compact(std::cout);
        json::structure_tree::walker walker = structure.get_walker();

        walker.root();
        traverse(walker);
        std::cerr << __FILE__ << "#" << __LINE__ << " (detail:map_to_sheets_and_dump): TODO: implement auto-mapping." << std::endl;
    }
    else
        app.read_map_definition(params.map_file.data(), params.map_file.size());

    app.read_stream(content.data(), content.size());
    doc.dump(params.config->output_format, params.config->output_path);
}

#else

void map_to_sheets_and_dump(const file_content& content, cmd_params& params)
{
    throw std::runtime_error(
        "map mode disabled as the spreadsheet model backend is not available.");
}

#endif

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
