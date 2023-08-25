/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xlsx_session_data.hpp"

namespace orcus {

xlsx_session_data::formula::formula(
    spreadsheet::sheet_t _sheet, spreadsheet::row_t _row, spreadsheet::col_t _column,
    std::string_view _exp) :
    sheet(_sheet), exp(_exp)
{
    ref.column = _column;
    ref.row = _row;
}

xlsx_session_data::array_formula::array_formula(
    spreadsheet::sheet_t _sheet, const spreadsheet::range_t& _ref, std::string_view _exp) :
    sheet(_sheet),
    ref(_ref),
    exp(_exp),
    results(
        std::make_shared<range_formula_results>(
            ref.last.row-ref.first.row+1,
            ref.last.column-ref.first.column+1))
{
}

xlsx_session_data::shared_formula::shared_formula(
    spreadsheet::sheet_t _sheet, spreadsheet::row_t _row, spreadsheet::col_t _column, size_t _identifier) :
    sheet(_sheet), row(_row), column(_column), identifier(_identifier), master(false) {}

xlsx_session_data::shared_formula::shared_formula(
    spreadsheet::sheet_t _sheet, spreadsheet::row_t _row, spreadsheet::col_t _column,
    size_t _identifier, std::string_view _formula) :
    sheet(_sheet), row(_row), column(_column),
    identifier(_identifier), formula(_formula), master(true) {}

xlsx_session_data::~xlsx_session_data() = default;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
