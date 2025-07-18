/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_OOXML_SCHEMAS_HPP
#define INCLUDED_ORCUS_OOXML_SCHEMAS_HPP

#include "ooxml_types.hpp"

namespace orcus {

const extern schema_t SCH_mc;
const extern schema_t SCH_opc_content_types;
const extern schema_t SCH_opc_rels;
const extern schema_t SCH_opc_rels_metadata_core_props;
const extern schema_t SCH_od_rels_calc_chain;
const extern schema_t SCH_od_rels_connections;
const extern schema_t SCH_od_rels_printer_settings;
const extern schema_t SCH_od_rels_rev_headers;
const extern schema_t SCH_od_rels_rev_log;
const extern schema_t SCH_od_rels_shared_strings;
const extern schema_t SCH_od_rels_styles;
const extern schema_t SCH_od_rels_theme;
const extern schema_t SCH_od_rels_usernames;
const extern schema_t SCH_od_rels_worksheet;
const extern schema_t SCH_od_rels_extended_props;
const extern schema_t SCH_od_rels_office_doc;
const extern schema_t SCH_od_rels_table;
const extern schema_t SCH_od_rels_pivot_cache_def;
const extern schema_t SCH_od_rels_pivot_cache_rec;
const extern schema_t SCH_od_rels_pivot_table;
const extern schema_t SCH_od_rels_drawing;
const extern schema_t SCH_xlsx_main;
const extern schema_t SCH_mso_x14ac;

/**
 * Null-terminated array of all schema types.
 */
const extern schema_t* const SCH_all;

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
