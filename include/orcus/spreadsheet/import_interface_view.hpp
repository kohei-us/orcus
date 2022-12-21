/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IMPORT_ORCUS_SPREADSHEET_IMPORT_INTERFACE_VIEW_HPP
#define IMPORT_ORCUS_SPREADSHEET_IMPORT_INTERFACE_VIEW_HPP

#include <cstdlib>

#include "view_types.hpp"
#include "../types.hpp"
#include "../env.hpp"

namespace orcus { namespace spreadsheet { namespace iface {

/**
 * Interface for importing view properties.  This interface may be obtained
 * from the import_sheet interface.
 */
class ORCUS_DLLPUBLIC import_sheet_view
{
public:
    virtual ~import_sheet_view();

    /**
     * Set the current sheet as the active sheet.
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
        double hor_split, double ver_split, const address_t& top_left_cell,
        sheet_pane_t active_pane) = 0;

    /**
     * Set the state of frozen view in the current sheet.
     *
     * @param visible_columns number of visible columns in the left pane.
     * @param visible_rows number of visible rows in the top pane.
     * @param top_left_cell the top left visible cell in the bottom right
     *                      pane.
     * @param active_pane active pane in this sheet.
     */
    virtual void set_frozen_pane(
        col_t visible_columns, row_t visible_rows, const address_t& top_left_cell,
        sheet_pane_t active_pane) = 0;

    /**
     * Set the selected cursor range in a specified sheet pane.
     *
     * @param pane sheet pane associated with the selection.  The top-left
     *             pane is used for a non-split sheet view.
     * @param range selected cursor range.  The range will be 1 column by 1
     *              row when the cursor is on a single cell only.
     */
    virtual void set_selected_range(sheet_pane_t pane, range_t range) = 0;
};

}}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
