/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_json.hpp"
#include "orcus/spreadsheet/import_interface.hpp"
#include "orcus/global.hpp"
#include "json_map_tree.hpp"

namespace orcus {

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

void orcus_json::read_stream(const char* p, size_t n) {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
