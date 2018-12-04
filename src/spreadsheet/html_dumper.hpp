/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_HTML_DUMPER_HPP
#define INCLUDED_ORCUS_SPREADSHEET_HTML_DUMPER_HPP

#include <string>
#include <ostream>
#include <ixion/types.hpp>

#include "impl_types.hpp"

namespace orcus { namespace spreadsheet {

class document;

namespace detail {

class html_dumper
{
    const document& m_doc;
    const overlapped_cells_type& m_overlapped_ranges;
    const col_merge_size_type& m_merge_ranges;

    const overlapped_col_index_type* get_overlapped_ranges(row_t row) const;
    const merge_size* get_merge_size(row_t row, col_t col) const;

public:
    html_dumper(
        const document& doc,
        const overlapped_cells_type& overlapped_ranges,
        const col_merge_size_type& merge_ranges);

    void dump(std::ostream& os, ixion::sheet_t sheet_id) const;
};

}}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

