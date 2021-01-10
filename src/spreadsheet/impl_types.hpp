/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_DETAIL_IMPL_TYPES_HPP
#define INCLUDED_ORCUS_SPREADSHEET_DETAIL_IMPL_TYPES_HPP

#include "orcus/spreadsheet/types.hpp"

#include <mdds/flat_segment_tree.hpp>
#include <unordered_map>
#include <memory>

namespace orcus { namespace spreadsheet { namespace detail {

struct merge_size
{
    col_t width;
    row_t height;

    merge_size(col_t _width, row_t _height) :
        width(_width), height(_height) {}
};

// Merged cell data stored in sheet.
typedef std::unordered_map<row_t, detail::merge_size> merge_size_type;
typedef std::unordered_map<col_t, std::unique_ptr<merge_size_type>> col_merge_size_type;

// Overlapped cells per row, used when rendering sheet content.
typedef mdds::flat_segment_tree<col_t, col_t> overlapped_col_index_type;
typedef std::unordered_map<row_t, std::unique_ptr<overlapped_col_index_type>> overlapped_cells_type;

}}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
