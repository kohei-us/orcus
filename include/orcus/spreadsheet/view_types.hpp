/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_SPREADSHEET_VIEW_TYPES_HPP
#define ORCUS_SPREADSHEET_VIEW_TYPES_HPP

#include "orcus/types.hpp"

namespace orcus { namespace spreadsheet {

/**
 * Type of sheet pane position in a split sheet view.
 */
enum class sheet_pane_t
{
    unspecified = 0,
    top_left,
    top_right,
    bottom_left,
    bottom_right
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
