/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xlsx_handler.hpp"
#include "xlsx_sheet_context.hpp"
#include "xlsx_table_context.hpp"
#include "xlsx_pivot_context.hpp"
#include "xlsx_drawing_context.hpp"

#include <iostream>

using namespace std;

namespace orcus {

xlsx_sheet_xml_handler::xlsx_sheet_xml_handler(
    session_context& session_cxt, const tokens& t,
    spreadsheet::sheet_t sheet_id,
    spreadsheet::iface::import_reference_resolver& resolver,
    spreadsheet::iface::import_sheet& sheet) :
    xml_stream_handler(session_cxt, t, std::make_unique<xlsx_sheet_context>(session_cxt, t, sheet_id, resolver, sheet))
{
}

xlsx_sheet_xml_handler::~xlsx_sheet_xml_handler()
{
}

void xlsx_sheet_xml_handler::pop_rel_extras(opc_rel_extras_t& other)
{
    xlsx_sheet_context& cxt = static_cast<xlsx_sheet_context&>(get_root_context());
    cxt.pop_rel_extras(other);
}

xlsx_table_xml_handler::xlsx_table_xml_handler(
    session_context& session_cxt, const tokens& t,
    spreadsheet::iface::import_table& table,
    spreadsheet::iface::import_reference_resolver& resolver) :
    xml_stream_handler(session_cxt, t, std::make_unique<xlsx_table_context>(session_cxt, t, table, resolver))
{
}

xlsx_pivot_cache_def_xml_handler::xlsx_pivot_cache_def_xml_handler(
    session_context& cxt, const tokens& t,
    spreadsheet::iface::import_pivot_cache_definition& pcache,
    spreadsheet::pivot_cache_id_t pcache_id) :
    xml_stream_handler(cxt, t, std::make_unique<xlsx_pivot_cache_def_context>(cxt, t, pcache, pcache_id)) {}

opc_rel_extras_t xlsx_pivot_cache_def_xml_handler::pop_rel_extras()
{
    xlsx_pivot_cache_def_context& cxt =
        static_cast<xlsx_pivot_cache_def_context&>(get_root_context());

    return cxt.pop_rel_extras();
}

xlsx_pivot_cache_rec_xml_handler::xlsx_pivot_cache_rec_xml_handler(
    session_context& cxt, const tokens& t,
    spreadsheet::iface::import_pivot_cache_records& pc_records) :
    xml_stream_handler(cxt, t, std::make_unique<xlsx_pivot_cache_rec_context>(cxt, t, pc_records)) {}

xlsx_pivot_table_xml_handler::xlsx_pivot_table_xml_handler(
    session_context& cxt, const tokens& t) :
    xml_stream_handler(cxt, t, std::make_unique<xlsx_pivot_table_context>(cxt, t)) {}

xlsx_drawing_xml_handler::xlsx_drawing_xml_handler(
    session_context& cxt, const tokens& t) :
    xml_stream_handler(cxt, t, std::make_unique<xlsx_drawing_context>(cxt, t)) {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
