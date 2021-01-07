/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "factory_sheet.hpp"
#include "orcus/spreadsheet/sheet.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/view.hpp"
#include "orcus/global.hpp"
#include "orcus/measurement.hpp"
#include "orcus/string_pool.hpp"

#include "formula_global.hpp"

#include <ixion/formula_name_resolver.hpp>
#include <ixion/model_context.hpp>
#include <ixion/formula.hpp>

namespace orcus { namespace spreadsheet {

import_sheet_named_exp::import_sheet_named_exp(document& doc, sheet_t sheet_index) :
    m_doc(doc), m_sheet_index(sheet_index), m_base(sheet_index, 0, 0) {}

import_sheet_named_exp::~import_sheet_named_exp() {}

void import_sheet_named_exp::define(const char* p_name, size_t n_name, const char* p_exp, size_t n_exp, formula_ref_context_t ref_cxt)
{
    string_pool& sp = m_doc.get_string_pool();
    m_name = sp.intern(p_name, n_name).first;

    const ixion::formula_name_resolver* resolver = m_doc.get_formula_name_resolver(ref_cxt);
    assert(resolver);

    ixion::model_context& cxt = m_doc.get_model_context();
    m_tokens = ixion::parse_formula_string(cxt, m_base, *resolver, p_exp, n_exp);
}

void import_sheet_named_exp::set_base_position(const src_address_t& pos)
{
    m_base.sheet = pos.sheet;
    m_base.row = pos.row;
    m_base.column = pos.column;
}

void import_sheet_named_exp::set_named_expression(
    const char* p_name, size_t n_name, const char* p_exp, size_t n_exp)
{
    define(p_name, n_name, p_exp, n_exp, formula_ref_context_t::global);
}

void import_sheet_named_exp::set_named_range(
    const char* p_name, size_t n_name, const char* p_range, size_t n_range)
{
    define(p_name, n_name, p_range, n_range, formula_ref_context_t::named_range);
}

void import_sheet_named_exp::commit()
{
    ixion::model_context& cxt = m_doc.get_model_context();
    cxt.set_named_expression(m_sheet_index, m_name.data(), m_name.size(), m_base, std::move(m_tokens));

    m_name.clear();
    m_base.sheet = 0;
    m_base.row = 0;
    m_base.column = 0;
}

import_data_table::import_data_table(sheet& sh) : m_sheet(sh) {}
import_data_table::~import_data_table() {}

void import_data_table::reset()
{
}

void import_data_table::set_type(data_table_type_t type)
{
}

void import_data_table::set_range(const range_t& range)
{
}

void import_data_table::set_first_reference(const char* p_ref, size_t n_ref, bool deleted)
{
}

void import_data_table::set_second_reference(const char* p_ref, size_t n_ref, bool deleted)
{
}

void import_data_table::commit()
{
}

import_auto_filter::import_auto_filter(sheet& sh, string_pool& sp) :
    m_sheet(sh),
    m_string_pool(sp),
    m_cur_col(-1) {}

void import_auto_filter::reset()
{
    mp_data.reset(new auto_filter_t);
    m_cur_col = -1;
    m_cur_col_data.reset();
}

void import_auto_filter::set_range(const range_t& range)
{
    mp_data->range = to_abs_range(range, m_sheet.get_index());
}

void import_auto_filter::set_column(col_t col)
{
    m_cur_col = col;
}

void import_auto_filter::append_column_match_value(const char* p, size_t n)
{
    // The string pool belongs to the document.
    pstring s = m_string_pool.intern(p, n).first;
    m_cur_col_data.match_values.insert(s);
}

void import_auto_filter::commit_column()
{
    if (!mp_data)
        return;

    mp_data->commit_column(m_cur_col, m_cur_col_data);
    m_cur_col_data.reset();
}

void import_auto_filter::commit()
{
    m_sheet.set_auto_filter_data(mp_data.release());
}

import_array_formula::import_array_formula(document& doc, sheet& sheet) :
    m_doc(doc), m_sheet(sheet), m_missing_formula_result(), m_error_policy(formula_error_policy_t::fail)
{
    m_range.first.column = -1;
    m_range.first.row = -1;
    m_range.last = m_range.first;
}

import_array_formula::~import_array_formula()
{
}

void import_array_formula::set_range(const range_t& range)
{
    m_range = range;

    // Initialize the result matrix with the missing result value.
    switch (m_missing_formula_result.get_type())
    {
        case ixion::formula_result::result_type::value:
        {
            ixion::matrix _mtx(
                m_range.last.row - m_range.first.row + 1,
                m_range.last.column - m_range.first.column + 1,
                m_missing_formula_result.get_value());
            m_result_mtx.swap(_mtx);
            break;
        }
        case ixion::formula_result::result_type::error:
        {
            ixion::matrix _mtx(
                m_range.last.row - m_range.first.row + 1,
                m_range.last.column - m_range.first.column + 1,
                m_missing_formula_result.get_error());
            m_result_mtx.swap(_mtx);
            break;
        }
        case ixion::formula_result::result_type::string:
        {
            ixion::matrix _mtx(
                m_range.last.row - m_range.first.row + 1,
                m_range.last.column - m_range.first.column + 1,
                m_missing_formula_result.get_string());
            m_result_mtx.swap(_mtx);
            break;
        }
        default:
        {
            ixion::matrix _mtx(
                m_range.last.row - m_range.first.row + 1,
                m_range.last.column - m_range.first.column + 1);
            m_result_mtx.swap(_mtx);
        }
    }
}

void import_array_formula::set_formula(formula_grammar_t grammar, const char* p, size_t n)
{
    const ixion::formula_name_resolver* resolver =
        m_doc.get_formula_name_resolver(spreadsheet::formula_ref_context_t::global);
    if (!resolver)
        return;

    // Tokenize the formula string and store it.
    ixion::model_context& cxt = m_doc.get_model_context();
    ixion::abs_address_t pos(m_sheet.get_index(), m_range.first.row, m_range.first.column);

    try
    {
        m_tokens = ixion::parse_formula_string(cxt, pos, *resolver, p, n);
    }
    catch (const std::exception& e)
    {
        if (m_error_policy == formula_error_policy_t::fail)
            throw;

        const char* p_error = e.what();
        size_t n_error = strlen(p_error);
        m_tokens = ixion::create_formula_error_tokens(cxt, p, n, p_error, n_error);
    }
}

void import_array_formula::set_result_value(row_t row, col_t col, double value)
{
    m_result_mtx.set(row, col, value);
}

void import_array_formula::set_result_string(row_t row, col_t col, size_t sindex)
{
    // TODO : handle this
}

void import_array_formula::set_result_empty(row_t row, col_t col)
{
    // TODO : handle this
}

void import_array_formula::set_result_bool(row_t row, col_t col, bool value)
{
    m_result_mtx.set(row, col, value);
}

void import_array_formula::commit()
{
    ixion::formula_result cached_results(std::move(m_result_mtx));
    m_sheet.set_grouped_formula(m_range, std::move(m_tokens), std::move(cached_results));
}

void import_array_formula::set_missing_formula_result(ixion::formula_result result)
{
    m_missing_formula_result = std::move(result);
}

void import_array_formula::set_formula_error_policy(formula_error_policy_t policy)
{
    m_error_policy = policy;
}

void import_array_formula::reset()
{
    m_tokens.clear();
    m_result_mtx = ixion::matrix();
    m_range.first.row = -1;
    m_range.first.column = -1;
    m_range.last.row = -1;
    m_range.last.column = -1;
}

import_formula::import_formula(document& doc, sheet& sheet, shared_formula_pool& pool) :
    m_doc(doc),
    m_sheet(sheet),
    m_shared_formula_pool(pool),
    m_row(-1),
    m_col(-1),
    m_shared_index(0),
    m_shared(false),
    m_error_policy(formula_error_policy_t::fail) {}

import_formula::~import_formula() {}

void import_formula::set_position(row_t row, col_t col)
{
    m_row = row;
    m_col = col;
}

void import_formula::set_formula(formula_grammar_t grammar, const char* p, size_t n)
{
    if (m_row < 0 || m_col < 0)
        return;

    const ixion::formula_name_resolver* resolver =
        m_doc.get_formula_name_resolver(spreadsheet::formula_ref_context_t::global);
    if (!resolver)
        return;

    // Tokenize the formula string and store it.
    ixion::model_context& cxt = m_doc.get_model_context();
    ixion::abs_address_t pos(m_sheet.get_index(), m_row, m_col);

    ixion::formula_tokens_t tokens;
    try
    {
        tokens = ixion::parse_formula_string(cxt, pos, *resolver, p, n);
    }
    catch (const std::exception& e)
    {
        if (m_error_policy == formula_error_policy_t::fail)
            throw;

        const char* p_error = e.what();
        size_t n_error = strlen(p_error);
        tokens = ixion::create_formula_error_tokens(cxt, p, n, p_error, n_error);
    }

    m_tokens_store = ixion::formula_tokens_store::create();
    m_tokens_store->get() = std::move(tokens);
}

void import_formula::set_shared_formula_index(size_t index)
{
    m_shared = true;
    m_shared_index = index;
}

void import_formula::set_result_value(double value)
{
    m_result.reset(ixion::formula_result(value));
}

void import_formula::set_result_string(size_t sindex) {}
void import_formula::set_result_empty() {}
void import_formula::set_result_bool(bool value) {}

void import_formula::commit()
{
    if (m_row < 0 || m_col < 0)
        return;

    if (m_shared)
    {
        if (m_tokens_store)
        {
            if (m_result)
                m_sheet.set_formula(m_row, m_col, m_tokens_store, *m_result);
            else
                m_sheet.set_formula(m_row, m_col, m_tokens_store);

            m_shared_formula_pool.add(m_shared_index, m_tokens_store);
        }
        else
        {
            ixion::formula_tokens_store_ptr_t ts = m_shared_formula_pool.get(m_shared_index);
            if (!ts)
                return;

            if (m_result)
                m_sheet.set_formula(m_row, m_col, ts, *m_result);
            else
                m_sheet.set_formula(m_row, m_col, ts);
        }
        return;
    }

    if (m_result)
        m_sheet.set_formula(m_row, m_col, m_tokens_store, *m_result);
    else
        m_sheet.set_formula(m_row, m_col, m_tokens_store);
}

void import_formula::set_missing_formula_result(ixion::formula_result result)
{
    m_result.reset(std::move(result));
}

void import_formula::set_formula_error_policy(formula_error_policy_t policy)
{
    m_error_policy = policy;
}

void import_formula::reset()
{
    m_tokens_store.reset();
    m_result.reset();
    m_row = -1;
    m_col = -1;
    m_shared_index = 0;
    m_shared = false;
}

import_sheet::import_sheet(document& doc, sheet& sh, sheet_view* view) :
    m_doc(doc),
    m_sheet(sh),
    m_formula(doc, sh, m_shared_formula_pool),
    m_array_formula(doc, sh),
    m_named_exp(doc, sh.get_index()),
    m_sheet_properties(doc, sh),
    m_data_table(sh),
    m_auto_filter(sh, doc.get_string_pool()),
    m_table(doc, sh),
    m_charset(character_set_t::unspecified),
    m_fill_missing_formula_results(false)
{
    if (view)
        m_sheet_view = std::make_unique<import_sheet_view>(*view, sh.get_index());
}

import_sheet::~import_sheet() {}

iface::import_sheet_view* import_sheet::get_sheet_view()
{
    return m_sheet_view.get();
}

iface::import_auto_filter* import_sheet::get_auto_filter()
{
    m_auto_filter.reset();
    return &m_auto_filter;
}

iface::import_conditional_format* import_sheet::get_conditional_format()
{
    return nullptr;
}

iface::import_data_table* import_sheet::get_data_table()
{
    return &m_data_table;
}

iface::import_named_expression* import_sheet::get_named_expression()
{
    return &m_named_exp;
}

iface::import_sheet_properties* import_sheet::get_sheet_properties()
{
    return &m_sheet_properties;
}

iface::import_table* import_sheet::get_table()
{
    m_table.reset();
    return &m_table;
}

iface::import_formula* import_sheet::get_formula()
{
    m_formula.reset();

    if (m_fill_missing_formula_results)
    {
        m_formula.set_missing_formula_result(
            ixion::formula_result(ixion::formula_error_t::no_result_error));
    }

    return &m_formula;
}

iface::import_array_formula* import_sheet::get_array_formula()
{
    m_array_formula.reset();

    if (m_fill_missing_formula_results)
    {
        m_array_formula.set_missing_formula_result(
            ixion::formula_result(ixion::formula_error_t::no_result_error));
    }

    return &m_array_formula;
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

void import_sheet::set_string(row_t row, col_t col, size_t sindex)
{
    m_sheet.set_string(row, col, sindex);
}

void import_sheet::set_value(row_t row, col_t col, double value)
{
    m_sheet.set_value(row, col, value);
}

void import_sheet::fill_down_cells(row_t src_row, col_t src_col, row_t range_size)
{
    m_sheet.fill_down_cells(src_row, src_col, range_size);
}

range_size_t import_sheet::get_sheet_size() const
{
    return m_doc.get_sheet_size();
}

void import_sheet::set_character_set(character_set_t charset)
{
    m_charset = charset;
}

void import_sheet::set_fill_missing_formula_results(bool b)
{
    m_fill_missing_formula_results = b;
}

void import_sheet::set_formula_error_policy(formula_error_policy_t policy)
{
    m_formula.set_formula_error_policy(policy);
    m_array_formula.set_formula_error_policy(policy);
}

import_sheet_view::import_sheet_view(sheet_view& view, sheet_t si) :
    m_view(view), m_sheet_index(si) {}

import_sheet_view::~import_sheet_view() {}

void import_sheet_view::set_split_pane(
        double hor_split, double ver_split,
        const orcus::spreadsheet::address_t& top_left_cell,
        orcus::spreadsheet::sheet_pane_t active_pane)
{
    m_view.set_split_pane(hor_split, ver_split, top_left_cell);
    m_view.set_active_pane(active_pane);
}

void import_sheet_view::set_frozen_pane(
        orcus::spreadsheet::col_t visible_columns,
        orcus::spreadsheet::row_t visible_rows,
        const orcus::spreadsheet::address_t& top_left_cell,
        orcus::spreadsheet::sheet_pane_t active_pane)
{
    m_view.set_frozen_pane(visible_columns, visible_rows, top_left_cell);
    m_view.set_active_pane(active_pane);
}

void import_sheet_view::set_selected_range(sheet_pane_t pane, range_t range)
{
    m_view.set_selection(pane, range);
}

void import_sheet_view::set_sheet_active()
{
    m_view.get_document_view().set_active_sheet(m_sheet_index);
}

import_sheet_properties::import_sheet_properties(document& doc, sheet& sh) :
    m_doc(doc), m_sheet(sh) {}

import_sheet_properties::~import_sheet_properties() {}

void import_sheet_properties::set_column_width(col_t col, double width, orcus::length_unit_t unit)
{
    col_width_t w = orcus::convert(width, unit, length_unit_t::twip);
    m_sheet.set_col_width(col, w);
}

void import_sheet_properties::set_column_hidden(col_t col, bool hidden)
{
    m_sheet.set_col_hidden(col, hidden);
}

void import_sheet_properties::set_row_height(row_t row, double height, orcus::length_unit_t unit)
{
    row_height_t h = orcus::convert(height, unit, length_unit_t::twip);
    m_sheet.set_row_height(row, h);
}

void import_sheet_properties::set_row_hidden(row_t row, bool hidden)
{
    m_sheet.set_row_hidden(row, hidden);
}

void import_sheet_properties::set_merge_cell_range(const range_t& range)
{
    m_sheet.set_merge_cell_range(range);
}

export_sheet::export_sheet(const document& doc, const sheet& sh) : m_doc(doc), m_sheet(sh) {}
export_sheet::~export_sheet() {}

void export_sheet::write_string(std::ostream& os, row_t row, col_t col) const
{
    const ixion::model_context& cxt = m_doc.get_model_context();
    ixion::abs_address_t pos(m_sheet.get_index(), row, col);

    switch (cxt.get_celltype(pos))
    {
        case ixion::celltype_t::string:
        {
            size_t str_id = cxt.get_string_identifier(pos);
            const std::string* p = cxt.get_string(str_id);
            if (p)
                os << *p;
        }
        break;
        case ixion::celltype_t::numeric:
            os << cxt.get_numeric_value(pos);
        break;
        default:
            ;
    }
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

