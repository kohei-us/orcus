/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "factory_sheet.hpp"
#include "orcus/spreadsheet/sheet.hpp"

namespace orcus { namespace spreadsheet {

import_sheet::import_sheet(sheet& sh) : m_sheet(sh) {}

import_sheet::~import_sheet() {}

iface::import_auto_filter* import_sheet::get_auto_filter()
{
    return m_sheet.get_auto_filter();
}

iface::import_conditional_format* import_sheet::get_conditional_format()
{
    return nullptr;
}

iface::import_data_table* import_sheet::get_data_table()
{
    return m_sheet.get_data_table();
}

iface::import_named_expression* import_sheet::get_named_expression()
{
    return nullptr;
}

iface::import_sheet_properties* import_sheet::get_sheet_properties()
{
    return m_sheet.get_sheet_properties();
}

iface::import_table* import_sheet::get_table()
{
    return m_sheet.get_table();
}

void import_sheet::set_array_formula(row_t row, col_t col, formula_grammar_t grammar, const char* p, size_t n, const char* p_range, size_t n_range)
{
    m_sheet.set_array_formula(row, col, grammar, p, n, p_range, n_range);
}

void import_sheet::set_array_formula(row_t row, col_t col, formula_grammar_t grammar, const char* p, size_t n, row_t array_rows, col_t array_cols)
{
    m_sheet.set_array_formula(row, col, grammar, p, n, array_rows, array_cols);
}

void import_sheet::set_auto(row_t row, col_t col, const char* p, size_t n)
{
    m_sheet.set_auto(row, col, p, n);
}

void import_sheet::set_bool(row_t row, col_t col, bool value)
{
    m_sheet.set_bool(row, col, value);
}

void import_sheet::set_date_time(row_t row, col_t col, int year, int month, int day, int hour, int minute, double second)
{
    m_sheet.set_date_time(row, col, year, month, day, hour, minute, second);
}

void import_sheet::set_format(row_t row, col_t col, size_t xf_index)
{
    m_sheet.set_format(row, col, xf_index);
}

void import_sheet::set_format(
    row_t row_start, col_t col_start, row_t row_end, col_t col_end, size_t xf_index)
{
    m_sheet.set_format(row_start, col_start, row_end, col_end, xf_index);
}

void import_sheet::set_formula(row_t row, col_t col, formula_grammar_t grammar, const char* p, size_t n)
{
    m_sheet.set_formula(row, col, grammar, p, n);
}

void import_sheet::set_formula_result(row_t row, col_t col, const char* p, size_t n)
{
    m_sheet.set_formula_result(row, col, p, n);
}

void import_sheet::set_formula_result(row_t row, col_t col, double value)
{
    m_sheet.set_formula_result(row, col, value);
}

void import_sheet::set_shared_formula(
    row_t row, col_t col, formula_grammar_t grammar, size_t sindex,
    const char* p_formula, size_t n_formula)
{
    m_sheet.set_shared_formula(row, col, grammar, sindex, p_formula, n_formula);
}

void import_sheet::set_shared_formula(
    row_t row, col_t col, formula_grammar_t grammar, size_t sindex,
    const char* p_formula, size_t n_formula, const char* p_range, size_t n_range)
{
    m_sheet.set_shared_formula(row, col, grammar, sindex, p_formula, n_formula, p_range, n_range);
}

void import_sheet::set_shared_formula(row_t row, col_t col, size_t sindex)
{
    m_sheet.set_shared_formula(row, col, sindex);
}

void import_sheet::set_string(row_t row, col_t col, size_t sindex)
{
    m_sheet.set_string(row, col, sindex);
}

void import_sheet::set_value(row_t row, col_t col, double value)
{
    m_sheet.set_value(row, col, value);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
