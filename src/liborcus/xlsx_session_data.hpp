/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_XLSX_SESSION_DATA_HPP
#define INCLUDED_ORCUS_XLSX_SESSION_DATA_HPP

#include "session_context.hpp"
#include "formula_result.hpp"

#include "orcus/spreadsheet/types.hpp"

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace orcus {

/**
 * Collection of global data that need to be persistent across different
 * parts during a single import session.
 */
struct xlsx_session_data : public session_context::custom_data
{
    struct formula
    {
        spreadsheet::sheet_t sheet;
        spreadsheet::address_t ref;
        std::string exp;

        formula_result result;

        formula(
            spreadsheet::sheet_t _sheet, spreadsheet::row_t _row, spreadsheet::col_t _column,
            const std::string& _exp);
    };

    struct array_formula
    {
        spreadsheet::sheet_t sheet;
        spreadsheet::range_t ref;
        std::string exp;

        std::shared_ptr<range_formula_results> results;

        array_formula(
            spreadsheet::sheet_t sheet, const spreadsheet::range_t& ref,
            const std::string& exp);
    };

    struct shared_formula
    {
        spreadsheet::sheet_t sheet;
        spreadsheet::row_t row;
        spreadsheet::col_t column;
        size_t identifier;
        std::string formula;
        bool master;

        formula_result result;

        shared_formula(
            spreadsheet::sheet_t _sheet, spreadsheet::row_t _row, spreadsheet::col_t _column,
            size_t _identifier);

        shared_formula(
            spreadsheet::sheet_t _sheet, spreadsheet::row_t _row, spreadsheet::col_t _column,
            size_t _identifier, const std::string& _formula);
    };

    typedef std::vector<std::unique_ptr<formula>> formulas_type;
    typedef std::vector<std::unique_ptr<array_formula>> array_formulas_type;
    typedef std::vector<std::unique_ptr<shared_formula>> shared_formulas_type;
    typedef std::unordered_map<pstring, spreadsheet::sheet_t, pstring::hash> sheet_name_map_type;

    formulas_type m_formulas;
    array_formulas_type m_array_formulas;
    shared_formulas_type m_shared_formulas;
    string_pool m_formula_result_strings;

    virtual ~xlsx_session_data();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
