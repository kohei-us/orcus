/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_json.hpp"
#include "orcus/spreadsheet/import_interface.hpp"
#include "orcus/global.hpp"
#include "orcus/json_parser.hpp"
#include "json_map_tree.hpp"

#include <iostream>

namespace orcus {

namespace {

class json_content_handler
{
    json_map_tree::walker m_walker;
    const json_map_tree::node* mp_current_node;

public:
    json_content_handler(const json_map_tree& map_tree) :
        m_walker(map_tree.get_tree_walker()),
        mp_current_node(nullptr) {}

    void begin_parse() {}
    void end_parse() {}

    void begin_array()
    {
        mp_current_node = m_walker.push_node(json_map_tree::input_node_type::array);
    }

    void end_array()
    {
        mp_current_node = m_walker.pop_node(json_map_tree::input_node_type::array);
    }

    void begin_object()
    {
        throw std::runtime_error("WIP: begin_object");
    }

    void object_key(const char* p, size_t len, bool transient)
    {
        throw std::runtime_error("WIP: object_key");
    }

    void end_object()
    {
        throw std::runtime_error("WIP: end_object");
    }

    void boolean_true()
    {
        mp_current_node = m_walker.push_node(json_map_tree::input_node_type::value);
        mp_current_node = m_walker.pop_node(json_map_tree::input_node_type::value);
    }

    void boolean_false()
    {
        mp_current_node = m_walker.push_node(json_map_tree::input_node_type::value);
        mp_current_node = m_walker.pop_node(json_map_tree::input_node_type::value);
    }

    void null()
    {
        mp_current_node = m_walker.push_node(json_map_tree::input_node_type::value);
        mp_current_node = m_walker.pop_node(json_map_tree::input_node_type::value);
    }

    void string(const char* p, size_t len, bool transient)
    {
        mp_current_node = m_walker.push_node(json_map_tree::input_node_type::value);
        mp_current_node = m_walker.pop_node(json_map_tree::input_node_type::value);
    }

    void number(double val)
    {
        mp_current_node = m_walker.push_node(json_map_tree::input_node_type::value);
        mp_current_node = m_walker.pop_node(json_map_tree::input_node_type::value);
    }
};

} // anonymous namespace

struct orcus_json::impl
{
    spreadsheet::iface::import_factory* im_factory;
    spreadsheet::sheet_t sheet_count;
    json_map_tree map_tree;

    impl(spreadsheet::iface::import_factory* _im_factory) :
        im_factory(_im_factory), sheet_count(0) {}
};

orcus_json::orcus_json(spreadsheet::iface::import_factory* im_fact) :
    mp_impl(orcus::make_unique<impl>(im_fact)) {}

orcus_json::~orcus_json() {}

void orcus_json::set_cell_link(const pstring& path, const pstring& sheet, spreadsheet::row_t row, spreadsheet::col_t col)
{
    mp_impl->map_tree.set_cell_link(path, cell_position_t(sheet, row, col));
}

void orcus_json::start_range(const pstring& sheet, spreadsheet::row_t row, spreadsheet::col_t col)
{
    mp_impl->map_tree.start_range(cell_position_t(sheet, row, col));
}

void orcus_json::append_field_link(const pstring& path)
{
    mp_impl->map_tree.append_field_link(path);
}

void orcus_json::set_range_row_group(const pstring& path)
{
    mp_impl->map_tree.set_range_row_group(path);
}

void orcus_json::commit_range()
{
    mp_impl->map_tree.commit_range();
}

void orcus_json::append_sheet(const pstring& name)
{
    if (name.empty())
        return;

    mp_impl->im_factory->append_sheet(mp_impl->sheet_count++, name.data(), name.size());
}

void orcus_json::read_stream(const char* p, size_t n)
{
    json_content_handler hdl(mp_impl->map_tree);
    json_parser<json_content_handler> parser(p, n, hdl);
    parser.parse();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
