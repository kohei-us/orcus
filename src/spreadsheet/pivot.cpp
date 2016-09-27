/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/spreadsheet/pivot.hpp"

namespace orcus { namespace spreadsheet {

pivot_cache::pivot_cache() {}

pivot_cache::~pivot_cache() {}

void pivot_cache::set_worksheet_source(
    const char* ref, size_t n_ref, const char* sheet_name, size_t n_sheet_name)
{
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
