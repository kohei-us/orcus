/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ORCUS_XLSX_TYPES_HPP__
#define __ORCUS_XLSX_TYPES_HPP__

#include "ooxml_types.hpp"
#include "orcus/spreadsheet/types.hpp"

namespace orcus {

namespace spreadsheet { namespace iface {

class import_sheet;

}}

struct xlsx_rel_sheet_info : public opc_rel_extra
{
    std::string_view name;
    size_t  id;

    xlsx_rel_sheet_info() : id(0) {}

    virtual ~xlsx_rel_sheet_info() override {}
};

struct xlsx_rel_pivot_cache_info : public opc_rel_extra
{
    spreadsheet::pivot_cache_id_t id;

    xlsx_rel_pivot_cache_info(spreadsheet::pivot_cache_id_t _id) : id(_id) {}

    virtual ~xlsx_rel_pivot_cache_info() override {}
};

/**
 * Data to pass to the pivot cache record handler.
 */
struct xlsx_rel_pivot_cache_record_info : public opc_rel_extra
{
    spreadsheet::pivot_cache_id_t id;

    xlsx_rel_pivot_cache_record_info(spreadsheet::pivot_cache_id_t _id) : id(_id) {}

    virtual ~xlsx_rel_pivot_cache_record_info() override {}
};

struct xlsx_rel_table_info : public opc_rel_extra
{
    spreadsheet::iface::import_sheet* sheet_interface;

    xlsx_rel_table_info() : sheet_interface(nullptr) {}

    virtual ~xlsx_rel_table_info() override {}
};

enum xlsx_cell_t
{
    xlsx_ct_unknown = 0,
    xlsx_ct_boolean,
    xlsx_ct_error,
    xlsx_ct_numeric,
    xlsx_ct_inline_string,
    xlsx_ct_shared_string,
    xlsx_ct_formula_string,
};

xlsx_cell_t to_xlsx_cell_type(std::string_view s);

std::string_view to_string(xlsx_cell_t type);

enum xlsx_rev_row_column_action_t
{
    xlsx_rev_rca_unknown = 0,
    xlsx_rev_rca_delete_column,
    xlsx_rev_rca_delete_row,
    xlsx_rev_rca_insert_column,
    xlsx_rev_rca_insert_row
};

xlsx_rev_row_column_action_t to_xlsx_rev_row_column_action_type(std::string_view s);

std::string_view to_string(xlsx_rev_row_column_action_t type);

enum class xlsx_dynamic_filter_t
{
    unknown,
    above_average,
    below_average,
    last_month,
    last_quarter,
    last_week,
    last_year,
    m1,
    m10,
    m11,
    m12,
    m2,
    m3,
    m4,
    m5,
    m6,
    m7,
    m8,
    m9,
    next_month,
    next_quarter,
    next_week,
    next_year,
    null,
    q1,
    q2,
    q3,
    q4,
    this_month,
    this_quarter,
    this_week,
    this_year,
    today,
    tomorrow,
    year_to_date,
    yesterday,
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
