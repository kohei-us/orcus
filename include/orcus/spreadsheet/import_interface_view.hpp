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
     * Set active pane in this sheet.  Only one pane can be active at any
     * given moment.
     *
     * @param pane active pane in this sheet.
     */
    virtual void set_active_pane(orcus::spreadsheet::sheet_pane_t pane) = 0;

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
