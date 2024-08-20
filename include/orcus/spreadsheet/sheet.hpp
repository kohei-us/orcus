/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_ODSTABLE_HPP
#define INCLUDED_ORCUS_SPREADSHEET_ODSTABLE_HPP

#include "../env.hpp"
#include "types.hpp"

#include <ostream>
#include <memory>

#include <ixion/address.hpp>
#include <ixion/formula_tokens.hpp>
#include <ixion/formula_result.hpp>

namespace orcus {

struct date_time_t;

namespace spreadsheet {

class document;

namespace old { struct auto_filter_t; }
struct auto_filter_range_t;

namespace detail {

struct sheet_impl;

}

/**
 * This class represents a single sheet instance in the internal document
 * model.
 */
class ORCUS_SPM_DLLPUBLIC sheet
{
    friend class document;
    friend struct detail::sheet_impl;

    static const row_t max_row_limit;
    static const col_t max_col_limit;

public:
    sheet(document& doc, sheet_t sheet_index);
    ~sheet() noexcept;

    void set_auto(row_t row, col_t col, std::string_view s);
    void set_string(row_t row, col_t col, string_id_t sindex);
    void set_value(row_t row, col_t col, double value);
    void set_bool(row_t row, col_t col, bool value);
    void set_date_time(row_t row, col_t col, int year, int month, int day, int hour, int minute, double second);
    void set_format(row_t row, col_t col, size_t index);
    void set_format(row_t row_start, col_t col_start, row_t row_end, col_t col_end, size_t index);
    void set_column_format(col_t col, col_t col_span, std::size_t index);
    void set_row_format(row_t row, std::size_t index);

    void set_formula(row_t row, col_t col, const ixion::formula_tokens_store_ptr_t& tokens);
    void set_formula(row_t row, col_t col, const ixion::formula_tokens_store_ptr_t& tokens, ixion::formula_result result);
    void set_grouped_formula(const range_t& range, ixion::formula_tokens_t tokens);
    void set_grouped_formula(const range_t& range, ixion::formula_tokens_t tokens, ixion::formula_result result);

    void set_col_width(col_t col, col_t col_span, col_width_t width);

    /**
     * Get column width in twips.
     *
     * @param col       column index
     * @param col_start pointer to a variable to store the index of the starting
     *                  column of the range with the same width. Pass nullptr if
     *                  the caller doesn't need this information.
     * @param col_end   pointer to a variable to store the index of the ending
     *                  column plus one, of the range with the same width. Pass
     *                  nullptr if the caller doesn't need this information.
     *
     * @return width of the specified column index (in twips).
     */
    col_width_t get_col_width(col_t col, col_t* col_start, col_t* col_end) const;

    void set_col_hidden(col_t col, col_t col_span, bool hidden);
    bool is_col_hidden(col_t col, col_t* col_start, col_t* col_end) const;

    void set_row_height(row_t row, row_t row_span, row_height_t height);
    row_height_t get_row_height(row_t row, row_t* row_start, row_t* row_end) const;

    void set_row_hidden(row_t row, row_t row_span, bool hidden);
    bool is_row_hidden(row_t row, row_t* row_start, row_t* row_end) const;

    void set_merge_cell_range(const range_t& range);

    void fill_down_cells(row_t src_row, col_t src_col, row_t range_size);

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

    old::auto_filter_t* get_auto_filter_data();
    const old::auto_filter_t* get_auto_filter_data() const;
    void set_auto_filter_data(old::auto_filter_t* p);

    void set_auto_filter(std::unique_ptr<auto_filter_range_t> filter);

    // Sheet dimension methods

    /**
     * Return the smallest range that contains all non-empty cells in this
     * sheet. The top-left corner of the returned range is always column 0 and
     * row 0.
     *
     * @return smallest range that contains all non-empty cells.
     */
    ixion::abs_range_t get_data_range() const;

    sheet_t get_index() const;

    date_time_t get_date_time(row_t row, col_t col) const;

    void dump_flat(std::ostream& os) const;
    void dump_check(std::ostream& os, std::string_view sheet_name) const;
    void dump_html(std::ostream& os) const;
    void dump_json(std::ostream& os) const;
    void dump_csv(std::ostream& os) const;

    void dump_debug_state(const std::string& output_dir, std::string_view sheet_name) const;

    /**
     * Get the cell format ID of specified cell.
     */
    size_t get_cell_format(row_t row, col_t col) const;

private:
    void finalize_import();

    std::unique_ptr<detail::sheet_impl> mp_impl;
};

}}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
