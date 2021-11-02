/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sheet_impl.hpp"
#include "orcus/spreadsheet/document.hpp"

#include <ixion/model_context.hpp>

namespace orcus { namespace spreadsheet {

sheet_impl::sheet_impl(document& doc, sheet& /*sh*/, sheet_t sheet_index) :
    m_doc(doc),
    m_col_widths(0, m_doc.get_sheet_size().columns, get_default_column_width()),
    m_row_heights(0, m_doc.get_sheet_size().rows, get_default_row_height()),
    m_col_width_pos(m_col_widths.begin()),
    m_row_height_pos(m_row_heights.begin()),
    m_col_hidden(0, m_doc.get_sheet_size().columns, false),
    m_row_hidden(0, m_doc.get_sheet_size().rows, false),
    m_col_hidden_pos(m_col_hidden.begin()),
    m_row_hidden_pos(m_row_hidden.begin()),
    m_sheet(sheet_index) {}

sheet_impl::~sheet_impl() {}

const detail::merge_size* sheet_impl::get_merge_size(row_t row, col_t col) const
{
    detail::col_merge_size_type::const_iterator it_col = m_merge_ranges.find(col);
    if (it_col == m_merge_ranges.end())
        return nullptr;

    detail::merge_size_type& col_merge_sizes = *it_col->second;
    detail::merge_size_type::const_iterator it_row = col_merge_sizes.find(row);
    if (it_row == col_merge_sizes.end())
        return nullptr;

    return &it_row->second;
}

ixion::abs_range_t sheet_impl::get_data_range() const
{
    const ixion::model_context& cxt = m_doc.get_model_context();
    return cxt.get_data_range(m_sheet);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
