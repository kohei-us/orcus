/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_json.hpp"
#include "json_map_tree.hpp"

namespace orcus {

struct orcus_json::impl
{
    json_map_tree m_map_tree;
};

orcus_json::orcus_json(spreadsheet::iface::import_factory* im_fact) {}
orcus_json::~orcus_json() {}

void orcus_json::set_cell_link(const pstring& path, const pstring& sheet, spreadsheet::row_t row, spreadsheet::col_t col) {}

void orcus_json::start_range(const pstring& sheet, spreadsheet::row_t row, spreadsheet::col_t col) {}
void orcus_json::append_field_link(const pstring& path) {}
void orcus_json::set_range_row_group(const pstring& path) {}
void orcus_json::commit_range() {}

void orcus_json::append_sheet(const pstring& name) {}

void orcus_json::read_stream(const char* p, size_t n) {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
