/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "factory_sheet.hpp"
#include "orcus/spreadsheet/sheet.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/global.hpp"

#include <ixion/formula_name_resolver.hpp>
#include <ixion/model_context.hpp>
#include <ixion/formula.hpp>

namespace orcus { namespace spreadsheet {

import_sheet_named_exp::import_sheet_named_exp(document& doc, sheet_t sheet_index) :
    m_doc(doc), m_sheet_index(sheet_index) {}

import_sheet_named_exp::~import_sheet_named_exp() {}

void import_sheet_named_exp::define_name(
    const char* p_name, size_t n_name, const char* p_exp, size_t n_exp)
{
    const ixion::formula_name_resolver* resolver = m_doc.get_formula_name_resolver();
    assert(resolver);

    ixion::model_context& cxt = m_doc.get_model_context();

    ixion::formula_tokens_t tokens =
        ixion::parse_formula_string(
            cxt, ixion::abs_address_t(0,0,0), *resolver, p_exp, n_exp);

    std::unique_ptr<ixion::formula_tokens_t> tokens_p =
        orcus::make_unique<ixion::formula_tokens_t>(std::move(tokens));

    cxt.set_named_expression(m_sheet_index, p_name, n_name, std::move(tokens_p));
}

import_sheet::import_sheet(document& doc, sheet& sh, sheet_view* view) :
    m_sheet(sh),
    m_named_exp(doc, sh.get_index())
{
    if (view)
        m_sheet_view = orcus::make_unique<import_sheet_view>(*view);
}

import_sheet::~import_sheet() {}

iface::import_sheet_view* import_sheet::get_sheet_view()
{
    return m_sheet_view.get();
}

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
    return &m_named_exp;
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

import_sheet_view::import_sheet_view(sheet_view& view) : m_view(view) {}

import_sheet_view::~import_sheet_view() {}

void import_sheet_view::set_active_pane(sheet_pane_t pane)
{
}

void import_sheet_view::set_selected_range(sheet_pane_t pane, range_t range)
{
}

void import_sheet_view::set_sheet_active()
{
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
