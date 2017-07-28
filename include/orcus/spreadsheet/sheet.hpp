/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_ODSTABLE_HPP
#define INCLUDED_ORCUS_SPREADSHEET_ODSTABLE_HPP

#include "orcus/spreadsheet/import_interface.hpp"
#include "orcus/spreadsheet/export_interface.hpp"
#include "orcus/env.hpp"

#include <ostream>
#include <ixion/address.hpp>

namespace orcus {

class pstring;
struct date_time_t;

namespace spreadsheet {

class document;
class sheet_range;
struct sheet_impl;
struct auto_filter_t;

/**
 * This class represents a single sheet instance in the internal document
 * model.
 */
class ORCUS_SPM_DLLPUBLIC sheet : public iface::export_sheet
{
    friend struct sheet_impl;

    static const row_t max_row_limit;
    static const col_t max_col_limit;

public:
    sheet(document& doc, sheet_t sheet_index, row_t row_size, col_t col_size);
    virtual ~sheet();

    // Import methods

    iface::import_sheet_properties* get_sheet_properties();
    iface::import_data_table* get_data_table();
    iface::import_table* get_table();
    iface::import_auto_filter* get_auto_filter();

    void set_auto(row_t row, col_t col, const char* p, size_t n);
    void set_string(row_t row, col_t col, size_t sindex);
    void set_value(row_t row, col_t col, double value);
    void set_bool(row_t row, col_t col, bool value);
    void set_date_time(row_t row, col_t col, int year, int month, int day, int hour, int minute, double second);
    void set_format(row_t row, col_t col, size_t index);
    void set_format(row_t row_start, col_t col_start, row_t row_end, col_t col_end, size_t index);
    void set_formula(row_t row, col_t col, formula_grammar_t grammar, const char* p, size_t n);
    void set_shared_formula(
        row_t row, col_t col, formula_grammar_t grammar, size_t sindex,
        const char* p_formula, size_t n_formula, const char* p_range, size_t n_range);
    void set_shared_formula(
        row_t row, col_t col, formula_grammar_t grammar, size_t sindex,
        const char* p_formula, size_t n_formula);
    void set_shared_formula(row_t row, col_t col, size_t sindex);
    void set_formula_result(row_t row, col_t col, double value);
    void set_formula_result(row_t row, col_t col, const char* p, size_t n);
    void set_array_formula(row_t, col_t, formula_grammar_t,
            const char*, size_t, row_t, col_t);
    void set_array_formula(row_t, col_t, formula_grammar_t,
            const char*, size_t, const char*, size_t);

    // Export methods

    virtual void write_string(std::ostream& os, row_t row, col_t col) const;

    void set_col_width(col_t col, col_width_t width);
    col_width_t get_col_width(col_t col, col_t* col_start, col_t* col_end) const;

    void set_col_hidden(col_t col, bool hidden);

    void set_row_height(row_t row, row_height_t height);
    row_height_t get_row_height(row_t row, row_t* row_start, row_t* row_end) const;

    void set_row_hidden(row_t row, bool hidden);

    void set_merge_cell_range(const range_t& range);

    /**
     * Return the size of a merged cell range.
     *
     * @param row row position of the upper-left cell.
     * @param col column position of the upper-left cell.
     *
     * @return merged cell range.
     */
    range_t get_merge_cell_range(row_t row, col_t col) const;

    size_t get_string_identifier(row_t row, col_t col) const;

    auto_filter_t* get_auto_filter_data();
    const auto_filter_t* get_auto_filter_data() const;
    void set_auto_filter_data(auto_filter_t* p);

    // Sheet dimension methods

    /**
     * Return the smallest range that contains all non-empty cells in this
     * sheet. The top-left corner of the returned range is always column 0 and
     * row 0.
     *
     * @return smallest range that contains all non-empty cells.
     */
    ixion::abs_range_t get_data_range() const;

    /**
     * Return a sheet range object that represents a sub-range within the
     * sheet.
     *
     * @param row_start start row position (0-based).
     * @param col_start start column position (0-based).
     * @param row_end end row position (0-based).
     * @param col_end end column position (0-based).
     *
     * @return sheet range object.
     */
    sheet_range get_sheet_range(
        row_t row_start, col_t col_start, row_t row_end, col_t col_end) const;

    row_t row_size() const;
    col_t col_size() const;
    sheet_t get_index() const;

    date_time_t get_date_time(row_t row, col_t col) const;

    void finalize();

    void dump_flat(std::ostream& os) const;
    void dump_check(std::ostream& os, const pstring& sheet_name) const;
    void dump_html(std::ostream& os) const;
    void dump_json(std::ostream& os) const;
    void dump_csv(std::ostream& os) const;

    /**
     * Get the cell format ID of specified cell.
     */
    size_t get_cell_format(row_t row, col_t col) const;

private:
    sheet_impl* mp_impl;
};

}}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
