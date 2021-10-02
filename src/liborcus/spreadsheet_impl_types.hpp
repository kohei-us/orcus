/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_IMPL_TYPES_HPP
#define INCLUDED_ORCUS_SPREADSHEET_IMPL_TYPES_HPP

#include "pstring.hpp"
#include "orcus/spreadsheet/types.hpp"

#include <iosfwd>

namespace orcus { namespace spreadsheet { namespace detail {

struct cell_position_t
{
    std::string_view sheet;
    spreadsheet::row_t row;
    spreadsheet::col_t col;

    cell_position_t();
    cell_position_t(const pstring& _sheet, spreadsheet::row_t _row, spreadsheet::col_t _col);
    cell_position_t(const cell_position_t& r);

    bool operator== (const cell_position_t& other) const;
    bool operator!= (const cell_position_t& other) const;
};

std::ostream& operator<< (std::ostream& os, const cell_position_t& ref);

bool operator< (const cell_position_t& left, const cell_position_t& right);

}}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
