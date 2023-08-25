/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "spreadsheet_impl_types.hpp"

#include <ostream>

namespace orcus { namespace spreadsheet { namespace detail {

cell_position_t::cell_position_t() : row(-1), col(-1) {}

cell_position_t::cell_position_t(std::string_view _sheet, spreadsheet::row_t _row, spreadsheet::col_t _col) :
    sheet(_sheet), row(_row), col(_col) {}

cell_position_t::cell_position_t(const cell_position_t& r) : sheet(r.sheet), row(r.row), col(r.col) {}

bool cell_position_t::operator== (const cell_position_t& other) const
{
    return sheet == other.sheet && row == other.row && col == other.col;
}

bool cell_position_t::operator!= (const cell_position_t& other) const
{
    return !operator==(other);
}

std::ostream& operator<< (std::ostream& os, const cell_position_t& ref)
{
    os << "[sheet='" << ref.sheet << "' row=" << ref.row << " column=" << ref.col << "]";
    return os;
}

bool operator< (const cell_position_t& left, const cell_position_t& right)
{
    if (left.sheet != right.sheet)
        return left.sheet < right.sheet;

    if (left.row != right.row)
        return left.row < right.row;

    return left.col < right.col;
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
