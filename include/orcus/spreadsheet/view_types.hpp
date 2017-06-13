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
 * Type of sheet pane position in a split sheet view.
 */
enum class sheet_pane_t : uint8_t
{
    unspecified = 0,
    top_left,
    top_right,
    bottom_left,
    bottom_right
};

/**
 * Type of pane state - whether it's frozen, split, or both.
 */
enum class pane_state_t : uint8_t
{
    unspecified = 0,
    frozen,
    split,
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

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
