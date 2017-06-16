/*************************************************************************
 *
 * Copyright (c) 2017 Kohei Yoshida
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************/

#ifndef IMPORT_ORCUS_SPREADSHEET_IMPORT_INTERFACE_VIEW_HPP
#define IMPORT_ORCUS_SPREADSHEET_IMPORT_INTERFACE_VIEW_HPP

#include <cstdlib>

#include "orcus/spreadsheet/view_types.hpp"
#include "orcus/types.hpp"
#include "orcus/env.hpp"

namespace orcus { namespace spreadsheet { namespace iface {

class ORCUS_DLLPUBLIC import_sheet_view
{
public:
    virtual ~import_sheet_view();

    /**
     * Set this sheet as the active sheet.
     */
    virtual void set_sheet_active() = 0;

    /**
     * Set the information about split view in the current sheet.
     *
     * @param hor_split horizontal position of the split in 1/20th of a point,
     *                  or 0 if none.  "Horizontal" in this case indicates the
     *                  column direction.
     * @param ver_split vertical position of the split in 1/20th of a point,
     *                  or 0 if none.  "Vertical" in this case indicates the
     *                  row direction.
     * @param top_left_cell the top left visible cell in the bottom right
     *                      pane.
     * @param active_pane active pane in this sheet.
     */
    virtual void set_split_pane(
        double hor_split, double ver_split,
        const orcus::spreadsheet::address_t& top_left_cell,
        orcus::spreadsheet::sheet_pane_t active_pane) = 0;

    /**
     * Set the information about frozen view in the current sheet.
     *
     * @param visible_columns number of visible columns in the left pane.
     * @param visible_rows number of visible rows in the top pane.
     * @param top_left_cell the top left visible cell in the bottom right
     *                      pane.
     * @param active_pane active pane in this sheet.
     */
    virtual void set_frozen_pane(
        orcus::spreadsheet::col_t visible_columns,
        orcus::spreadsheet::row_t visible_rows,
        const orcus::spreadsheet::address_t& top_left_cell,
        orcus::spreadsheet::sheet_pane_t active_pane) = 0;

    /**
     * Set the selected cursor range in a specified sheet pane.
     *
     * @param pane sheet pane associated with the selection.  The top-left
     *             pane is used for a non-split sheet view.
     * @param range selected cursor range.  The range will be 1 column by 1
     *              row when the cursor is on a single cell only.
     */
    virtual void set_selected_range(
        orcus::spreadsheet::sheet_pane_t pane,
        orcus::spreadsheet::range_t range) = 0;
};

}}}

#endif
