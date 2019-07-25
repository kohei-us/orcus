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
#include <vector>
#include <algorithm>
#include <sstream>

using namespace std;

namespace orcus { namespace detail {

#ifdef __ORCUS_SPREADSHEET_MODEL

namespace {

class structure_mapper
{
    orcus_json& m_app;

    json::structure_tree::walker m_walker;
    size_t m_repeat_count;
    size_t m_range_count;
    std::string m_sheet_name_prefix;

    struct
    {
        std::vector<std::string> paths;
        std::vector<std::string> row_groups;

        void sort()
        {
            std::sort(paths.begin(), paths.end());
            std::sort(row_groups.begin(), row_groups.end());
        }

        void clear()
        {
            paths.clear();
            row_groups.clear();
        }

    } m_current_range;

    bool m_sort_before_push;

public:
    structure_mapper(orcus_json& app, const json::structure_tree::walker& walker) :
        m_app(app),
        m_walker(walker),
        m_repeat_count(0),
        m_range_count(0),
        m_sheet_name_prefix("range-"),
        m_sort_before_push(false) {}

    void run()
    {
        reset();
        traverse(0);
    }

private:

    void reset()
    {
        m_walker.root();
        m_current_range.clear();
        m_repeat_count = 0;
    }

    void push_range()
    {
        if (m_sort_before_push)
            m_current_range.sort();

        // Build sheet name first and insert a new sheet.
        std::ostringstream os_sheet_name;
        os_sheet_name << m_sheet_name_prefix << m_range_count;
        std::string sheet_name = os_sheet_name.str();
        m_app.append_sheet(sheet_name);

        // Push the linked range.
        m_app.start_range(sheet_name, 0, 0, true);

        for (const std::string& s : m_current_range.paths)
            m_app.append_field_link(s, pstring());

        for (const std::string& s : m_current_range.row_groups)
            m_app.set_range_row_group(s);

        m_app.commit_range();

        m_current_range.clear();
        ++m_range_count;
    }

    void traverse(size_t pos)
    {
        json::structure_tree::node_properties node = m_walker.get_node();

        if (node.repeat)
        {
            ++m_repeat_count;
            m_current_range.row_groups.push_back(m_walker.build_path_to_parent());
        }

        if (m_repeat_count && node.type == json::structure_tree::node_type::value)
        {
            for (std::string path : m_walker.build_field_paths())
                m_current_range.paths.push_back(std::move(path));
        }

        for (size_t i = 0, n = m_walker.child_count(); i < n; ++i)
        {
            m_walker.descend(i);
            traverse(i);
            m_walker.ascend();
        }

        if (node.repeat)
        {
            --m_repeat_count;

            if (!m_repeat_count)
                push_range();
        }
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
        // Automatic mapping of JSON to table.
        json::structure_tree structure;
        structure.parse(content.data(), content.size());
        structure.dump_compact(std::cout);

        structure_mapper mapper(app, structure.get_walker());
        mapper.run();
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
