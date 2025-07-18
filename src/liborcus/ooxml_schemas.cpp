/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ooxml_schemas.hpp"

namespace orcus {

const schema_t SCH_mc                           = "http://schemas.openxmlformats.org/markup-compatibility/2006";
const schema_t SCH_opc_content_types            = "http://schemas.openxmlformats.org/package/2006/content-types";
const schema_t SCH_opc_rels                     = "http://schemas.openxmlformats.org/package/2006/relationships";
const schema_t SCH_opc_rels_metadata_core_props = "http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties";
const schema_t SCH_od_rels_calc_chain           = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/calcChain";
const schema_t SCH_od_rels_connections          = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/connections";
const schema_t SCH_od_rels_printer_settings     = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/printerSettings";
const schema_t SCH_od_rels_rev_headers          = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/revisionHeaders";
const schema_t SCH_od_rels_rev_log              = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/revisionLog";
const schema_t SCH_od_rels_shared_strings       = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings";
const schema_t SCH_od_rels_styles               = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles";
const schema_t SCH_od_rels_theme                = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme";
const schema_t SCH_od_rels_usernames            = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/usernames";
const schema_t SCH_od_rels_worksheet            = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet";
const schema_t SCH_od_rels_extended_props       = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties";
const schema_t SCH_od_rels_office_doc           = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument";
const schema_t SCH_od_rels_table                = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/table";
const schema_t SCH_od_rels_pivot_cache_def      = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotCacheDefinition";
const schema_t SCH_od_rels_pivot_cache_rec      = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotCacheRecords";
const schema_t SCH_od_rels_pivot_table          = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotTable";
const schema_t SCH_od_rels_drawing              = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/drawing";
const schema_t SCH_xlsx_main                    = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
const schema_t SCH_mso_x14ac                    = "http://schemas.microsoft.com/office/spreadsheetml/2009/9/ac";

namespace {

const schema_t schs[] = {
    SCH_mc,
    SCH_opc_content_types,
    SCH_opc_rels,
    SCH_opc_rels_metadata_core_props,
    SCH_od_rels_calc_chain,
    SCH_od_rels_connections,
    SCH_od_rels_printer_settings,
    SCH_od_rels_rev_headers,
    SCH_od_rels_rev_log,
    SCH_od_rels_shared_strings,
    SCH_od_rels_styles,
    SCH_od_rels_theme,
    SCH_od_rels_usernames,
    SCH_od_rels_worksheet,
    SCH_od_rels_extended_props,
    SCH_od_rels_office_doc,
    SCH_od_rels_table,
    SCH_od_rels_pivot_cache_def,
    SCH_od_rels_pivot_cache_rec,
    SCH_od_rels_pivot_table,
    SCH_od_rels_drawing,
    SCH_xlsx_main,
    SCH_mso_x14ac,
    nullptr
};

}

const schema_t* const SCH_all = schs;

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
