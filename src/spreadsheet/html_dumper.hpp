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
    overlapped_cells_type m_overlapped_ranges;
    const col_merge_size_type& m_merge_ranges;
    sheet_t m_sheet_id;

    const overlapped_col_index_type* get_overlapped_ranges(row_t row) const;
    const merge_size* get_merge_size(row_t row, col_t col) const;

    void build_overlapped_ranges();

public:
    html_dumper(
        const document& doc,
        const col_merge_size_type& merge_ranges,
        sheet_t sheet_id);

    void dump(std::ostream& os) const;
};

}}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

