/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_FACTORY_SHEET_HPP
#define INCLUDED_ORCUS_SPREADSHEET_FACTORY_SHEET_HPP

#include "orcus/spreadsheet/import_interface.hpp"
#include "orcus/spreadsheet/import_interface_view.hpp"

#include <memory>

namespace orcus { namespace spreadsheet {

class document;
class sheet_view;
class sheet;
class import_sheet_view;

class import_sheet_named_exp : public iface::import_named_expression
{
    document& m_doc;
    sheet_t m_sheet_index;

public:
    import_sheet_named_exp(document& doc, sheet_t sheet_index);
    virtual ~import_sheet_named_exp() override;

    virtual void define_name(const char* p_name, size_t n_name, const char* p_exp, size_t n_exp) override;
};

class import_sheet : public iface::import_sheet
{
    sheet& m_sheet;
    import_sheet_named_exp m_named_exp;
    std::unique_ptr<import_sheet_view> m_sheet_view;

public:
    import_sheet(document& doc, sheet& sh, sheet_view* view);
    virtual ~import_sheet() override;

    virtual iface::import_sheet_view* get_sheet_view() override;
    virtual iface::import_auto_filter* get_auto_filter() override;
    virtual iface::import_conditional_format* get_conditional_format() override;
    virtual iface::import_data_table* get_data_table() override;
    virtual iface::import_named_expression* get_named_expression() override;
    virtual iface::import_sheet_properties* get_sheet_properties() override;
    virtual iface::import_table* get_table() override;
    virtual void set_array_formula(row_t row, col_t col, formula_grammar_t grammar, const char* p, size_t n, const char* p_range, size_t n_range) override;
    virtual void set_array_formula(row_t row, col_t col, formula_grammar_t grammar, const char* p, size_t n, row_t array_rows, col_t array_cols) override;
    virtual void set_auto(row_t row, col_t col, const char* p, size_t n) override;
    virtual void set_bool(row_t row, col_t col, bool value) override;
    virtual void set_date_time(row_t row, col_t col, int year, int month, int day, int hour, int minute, double second) override;
    virtual void set_format(row_t row, col_t col, size_t xf_index) override;
    virtual void set_format(row_t row_start, col_t col_start, row_t row_end, col_t col_end, size_t xf_index) override;
    virtual void set_formula(row_t row, col_t col, formula_grammar_t grammar, const char* p, size_t n) override;
    virtual void set_formula_result(row_t row, col_t col, const char* p, size_t n) override;
    virtual void set_formula_result(row_t row, col_t col, double value) override;
    virtual void set_shared_formula(row_t row, col_t col, formula_grammar_t grammar, size_t sindex, const char* p_formula, size_t n_formula) override;
    virtual void set_shared_formula(row_t row, col_t col, formula_grammar_t grammar, size_t sindex, const char* p_formula, size_t n_formula, const char* p_range, size_t n_range) override;
    virtual void set_shared_formula(row_t row, col_t col, size_t sindex) override;
    virtual void set_string(row_t row, col_t col, size_t sindex) override;
    virtual void set_value(row_t row, col_t col, double value) override;
    virtual range_size_t get_sheet_size() const override;
};

class import_sheet_view : public iface::import_sheet_view
{
    sheet_view& m_view;
    sheet_t m_sheet_index;
public:
    import_sheet_view(sheet_view& view, sheet_t si);
    virtual ~import_sheet_view();
    virtual void set_sheet_active() override;

    virtual void set_split_pane(
        double hor_split, double ver_split,
        const orcus::spreadsheet::address_t& top_left_cell,
        orcus::spreadsheet::sheet_pane_t active_pane) override;

    virtual void set_frozen_pane(
        orcus::spreadsheet::col_t visible_columns,
        orcus::spreadsheet::row_t visible_rows,
        const orcus::spreadsheet::address_t& top_left_cell,
        orcus::spreadsheet::sheet_pane_t active_pane) override;

    virtual void set_selected_range(sheet_pane_t pane, range_t range) override;
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
