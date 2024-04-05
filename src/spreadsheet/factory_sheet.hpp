/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_FACTORY_SHEET_HPP
#define INCLUDED_ORCUS_SPREADSHEET_FACTORY_SHEET_HPP

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/spreadsheet/import_interface_view.hpp>
#include <orcus/spreadsheet/auto_filter.hpp>

#include <orcus/spreadsheet/export_interface.hpp>

#include "factory_table.hpp"
#include "shared_formula.hpp"

#include <memory>
#include <optional>
#include <ixion/formula_name_resolver.hpp>
#include <ixion/formula_result.hpp>
#include <ixion/matrix.hpp>

namespace orcus {

class string_pool;

namespace spreadsheet {

class document;
class sheet_view;
class sheet;
class import_sheet_view;

class import_sheet_named_exp : public iface::import_named_expression
{
    document& m_doc;
    sheet_t m_sheet_index;
    std::string_view m_name;
    ixion::abs_address_t m_base;
    ixion::formula_tokens_t m_tokens;

    void define(std::string_view name, std::string_view expression, formula_ref_context_t ref_cxt);

public:
    import_sheet_named_exp(document& doc, sheet_t sheet_index);
    virtual ~import_sheet_named_exp() override;

    virtual void set_base_position(const src_address_t& pos) override;
    virtual void set_named_expression(std::string_view name, std::string_view expression) override;
    virtual void set_named_range(std::string_view name, std::string_view range) override;
    virtual void commit();
};

/**
 * Implement the sheet properties import interface, but the actual
 * properties are stored in sheet.
 */
class import_sheet_properties : public iface::import_sheet_properties
{
    document& m_doc;
    sheet& m_sheet;
public:
    import_sheet_properties(document& doc, sheet& sh);
    ~import_sheet_properties();

    virtual void set_column_width(col_t col, col_t col_span, double width, orcus::length_unit_t unit);
    virtual void set_column_hidden(col_t col, col_t col_span, bool hidden);
    virtual void set_row_height(row_t row, row_t row_span, double height, orcus::length_unit_t unit);
    virtual void set_row_hidden(row_t row, bool hidden);
    virtual void set_merge_cell_range(const range_t& range);
};

class import_data_table : public iface::import_data_table
{
    sheet& m_sheet;
public:
    import_data_table(sheet& sh);
    ~import_data_table();

    void reset();

    virtual void set_type(data_table_type_t type) override;

    virtual void set_range(const range_t& range) override;

    virtual void set_first_reference(std::string_view ref, bool deleted) override;

    virtual void set_second_reference(std::string_view ref, bool deleted) override;

    virtual void commit() override;
};

class import_auto_filter : public orcus::spreadsheet::iface::import_auto_filter
{
    sheet& m_sheet;
    string_pool& m_string_pool;
    std::unique_ptr<auto_filter_t> mp_data;
    col_t m_cur_col;
    auto_filter_column_t m_cur_col_data;

public:
    import_auto_filter(sheet& sh, string_pool& sp);

    void reset();

    virtual void set_range(const range_t& range) override;

    virtual void set_column(col_t col) override;

    virtual void append_column_match_value(std::string_view value) override;

    virtual void commit_column() override;

    virtual void commit() override;
};

class import_array_formula : public iface::import_array_formula
{
    document& m_doc;
    sheet& m_sheet;

    range_t m_range;
    ixion::formula_tokens_t m_tokens;
    ixion::formula_result m_missing_formula_result;
    ixion::matrix m_result_mtx;
    formula_error_policy_t m_error_policy;

public:
    import_array_formula(document& doc, sheet& sheet);
    virtual ~import_array_formula() override;

    virtual void set_range(const range_t& range) override;

    virtual void set_formula(formula_grammar_t grammar, std::string_view formula) override;

    virtual void set_result_value(row_t row, col_t col, double value) override;

    virtual void set_result_string(row_t row, col_t col, std::string_view value) override;

    virtual void set_result_empty(row_t row, col_t col) override;

    virtual void set_result_bool(row_t row, col_t col, bool value) override;

    virtual void commit() override;

    void set_missing_formula_result(ixion::formula_result result);

    void set_formula_error_policy(formula_error_policy_t policy);

    void reset();
};

class import_formula : public iface::import_formula
{
    document& m_doc;
    sheet& m_sheet;
    shared_formula_pool& m_shared_formula_pool;

    row_t m_row;
    col_t m_col;
    size_t m_shared_index;
    bool m_shared;

    ixion::formula_tokens_store_ptr_t m_tokens_store;
    std::optional<ixion::formula_result> m_result;
    formula_error_policy_t m_error_policy;

public:
    import_formula(document& doc, sheet& sheet, shared_formula_pool& pool);
    virtual ~import_formula() override;

    virtual void set_position(row_t row, col_t col) override;
    virtual void set_formula(formula_grammar_t grammar, std::string_view formula) override;
    virtual void set_shared_formula_index(size_t index) override;
    virtual void set_result_value(double value) override;
    virtual void set_result_string(std::string_view value) override;
    virtual void set_result_empty() override;
    virtual void set_result_bool(bool value) override;
    virtual void commit() override;

    void set_missing_formula_result(ixion::formula_result result);
    void set_formula_error_policy(formula_error_policy_t policy);

    void reset();
};

class import_sheet : public iface::import_sheet
{
    document& m_doc;
    sheet& m_sheet;
    shared_formula_pool m_shared_formula_pool;
    import_formula m_formula;
    import_array_formula m_array_formula;
    import_sheet_named_exp m_named_exp;
    import_sheet_properties m_sheet_properties;
    import_data_table m_data_table;
    import_auto_filter m_auto_filter;
    import_table m_table;
    character_set_t m_charset;

    std::unique_ptr<import_sheet_view> m_sheet_view;

    bool m_fill_missing_formula_results;

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
    virtual iface::import_formula* get_formula() override;
    virtual iface::import_array_formula* get_array_formula() override;
    virtual void set_auto(row_t row, col_t col, std::string_view s) override;
    virtual void set_bool(row_t row, col_t col, bool value) override;
    virtual void set_date_time(row_t row, col_t col, int year, int month, int day, int hour, int minute, double second) override;
    virtual void set_format(row_t row, col_t col, size_t xf_index) override;
    virtual void set_format(row_t row_start, col_t col_start, row_t row_end, col_t col_end, size_t xf_index) override;
    virtual void set_column_format(col_t col, col_t col_span, std::size_t xf_index) override;
    virtual void set_row_format(row_t row, std::size_t xf_index) override;
    virtual void set_string(row_t row, col_t col, string_id_t sindex) override;
    virtual void set_value(row_t row, col_t col, double value) override;
    virtual void fill_down_cells(row_t src_row, col_t src_col, row_t range_size) override;
    virtual range_size_t get_sheet_size() const override;

    void set_character_set(character_set_t charset);
    void set_fill_missing_formula_results(bool b);
    void set_formula_error_policy(formula_error_policy_t policy);
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
        const address_t& top_left_cell,
        sheet_pane_t active_pane) override;

    virtual void set_frozen_pane(
        col_t visible_columns, row_t visible_rows,
        const address_t& top_left_cell,
        sheet_pane_t active_pane) override;

    virtual void set_selected_range(sheet_pane_t pane, range_t range) override;
};

class export_sheet : public iface::export_sheet
{
    const document& m_doc;
    const sheet& m_sheet;
public:
    export_sheet(const document& doc, const sheet& sh);
    ~export_sheet();

    virtual void write_string(std::ostream& os, row_t row, col_t col) const override;
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
