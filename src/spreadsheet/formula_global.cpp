/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "formula_global.hpp"

#include <ixion/address.hpp>
#include <ixion/formula_name_resolver.hpp>

namespace orcus { namespace spreadsheet {

ixion::abs_range_t to_abs_range(
    const ixion::formula_name_resolver& resolver, const char* p_ref, size_t n_ref)
{
    ixion::abs_range_t range(ixion::abs_range_t::invalid);
    ixion::abs_address_t pos(0,0,0);

    ixion::formula_name_t res = resolver.resolve({p_ref, n_ref}, pos);
    switch (res.type)
    {
        case ixion::formula_name_t::cell_reference:
            // Single cell reference.
            range.first = std::get<ixion::address_t>(res.value).to_abs(pos);
            range.last = range.first;
            break;
        case ixion::formula_name_t::range_reference:
            // Range reference.
            range = std::get<ixion::range_t>(res.value).to_abs(pos);
            break;
        default:
            ; // Unsupported range.  Leave it invalid.
    }

    return range;
}

ixion::abs_range_t to_abs_range(const range_t& range, sheet_t sheet_pos)
{
    ixion::abs_range_t ret;
    ret.first.column = range.first.column;
    ret.first.row    = range.first.row;
    ret.first.sheet  = sheet_pos;

    ret.last.column = range.last.column;
    ret.last.row    = range.last.row;
    ret.last.sheet  = sheet_pos;

    return ret;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
