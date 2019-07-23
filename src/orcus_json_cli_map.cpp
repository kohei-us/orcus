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
#include <cassert>

using namespace std;

namespace orcus { namespace detail {

#ifdef __ORCUS_SPREADSHEET_MODEL

namespace {

class StructureMapper
{
    json::structure_tree::walker m_walker;
    size_t m_repeat_count;

public:
    StructureMapper(const json::structure_tree::walker& walker) :
        m_walker(walker),
        m_repeat_count(0) {}

    void run()
    {
        m_walker.root();
        traverse(0);

        assert(!m_repeat_count);
    }
private:
    void traverse(size_t pos)
    {
        json::structure_tree::node_properties node = m_walker.get_node();
        std::cout << __FILE__ << "#" << __LINE__ << " (detail:traverse): " << node.type << std::endl;

        if (node.repeat)
            ++m_repeat_count;

        if (m_repeat_count && node.type == json::structure_tree::node_type::value)
        {
            std::cerr << __FILE__ << "#" << __LINE__ << " (StructureMapper:traverse): path = " << m_walker.build_path() << std::endl;
        }

        for (size_t i = 0, n = m_walker.child_count(); i < n; ++i)
        {
            m_walker.descend(i);
            traverse(i);
            m_walker.ascend();
        }

        if (node.repeat)
            --m_repeat_count;
    }
};


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

        StructureMapper mapper(structure.get_walker());
        mapper.run();
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
