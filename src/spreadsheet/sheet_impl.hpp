/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_SHEET_IMPL_HPP
#define INCLUDED_ORCUS_SPREADSHEET_SHEET_IMPL_HPP

#include "impl_types.hpp"
#include "orcus/spreadsheet/auto_filter.hpp"

namespace orcus { namespace spreadsheet {

class document;
class sheet;

typedef mdds::flat_segment_tree<row_t, size_t>  segment_row_index_type;
typedef std::unordered_map<col_t, std::unique_ptr<segment_row_index_type>> cell_format_type;

// Widths and heights are stored in twips.
typedef mdds::flat_segment_tree<col_t, col_width_t> col_widths_store_type;
typedef mdds::flat_segment_tree<row_t, row_height_t> row_heights_store_type;

// hidden information
typedef mdds::flat_segment_tree<col_t, bool> col_hidden_store_type;
typedef mdds::flat_segment_tree<row_t, bool> row_hidden_store_type;

struct sheet_impl
{
    document& doc;

    mutable col_widths_store_type col_widths;
    mutable row_heights_store_type row_heights;
    col_widths_store_type::const_iterator col_width_pos;
    row_heights_store_type::const_iterator row_height_pos;

    mutable col_hidden_store_type col_hidden;
    mutable row_hidden_store_type row_hidden;
    col_hidden_store_type::const_iterator col_hidden_pos;
    row_hidden_store_type::const_iterator row_hidden_pos;

    detail::col_merge_size_type merge_ranges; /// 2-dimensional merged cell ranges.

    std::unique_ptr<auto_filter_t> auto_filter_data;

    cell_format_type cell_formats;
    const sheet_t sheet_id;

    sheet_impl() = delete;
    sheet_impl(const sheet_impl&) = delete;
    sheet_impl& operator=(const sheet_impl&) = delete;

    sheet_impl(document& _doc, sheet& sh, sheet_t sheet_index);
    ~sheet_impl();

    const detail::merge_size* get_merge_size(row_t row, col_t col) const;

    ixion::abs_range_t get_data_range() const;
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
