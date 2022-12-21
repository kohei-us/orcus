/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_SPREADSHEET_VIEW_TYPES_HPP
#define ORCUS_SPREADSHEET_VIEW_TYPES_HPP

#include "orcus/spreadsheet/types.hpp"

namespace orcus { namespace spreadsheet {

/**
 * Sheet pane position in a split sheet view.  When the sheet is split, it is
 * split into four panes.
 */
enum class sheet_pane_t : uint8_t
{
    unspecified = 0,
    /** Top-left pane. */
    top_left,
    /** Top-right pane. */
    top_right,
    /** Bottom-left pane. */
    bottom_left,
    /** Bottom-right pane. */
    bottom_right
};

/**
 * State of a split pane - whether it's frozen, split, or both.
 */
enum class pane_state_t : uint8_t
{
    /** The state of the pane is not specified. */
    unspecified = 0,
    /** The pane is frozen. */
    frozen,
    /** The pane is split. */
    split,
    /** The pane is both frozen and split. */
    frozen_split
};

/**
 * Store information about the state of a split sheet view.
 */
struct split_pane_t
{
    /**
     * Horizontal distance to the vertical split bar in 1/20th of a point, or
     * 0 if not horizontally split.
     */
    double hor_split;

    /**
     * Vertical distance to the horizontal split bar in 1/20th of a point, or
     * 0 if not vertically split.
     */
    double ver_split;

    /**
     * Top-left visible cell of the bottom-right pane.  This value is valid
     * only when either the horizontal distance or the vertical distance is
     * non-zero.
     */
    address_t top_left_cell;
};

/**
 * Store the state of a frozen sheet view.
 */
struct frozen_pane_t
{
    /**
     * The number of visible columns in the top-left pane.
     */
    col_t visible_columns;
    /**
     * The number of visible rows in the top-left pane.
     */
    row_t visible_rows;
    /**
     * The position of the top-left cell in the bottom-right pane.
     */
    address_t top_left_cell;
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
