/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_XLSX_SESSION_DATA_HPP
#define INCLUDED_ORCUS_XLSX_SESSION_DATA_HPP

#include "session_context.hpp"

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
        spreadsheet::row_t row;
        spreadsheet::col_t column;
        std::string exp;
        std::string range;
        bool array;

        formula(
            spreadsheet::sheet_t _sheet, spreadsheet::row_t _row, spreadsheet::col_t _column,
            const std::string& _exp);

        formula(
            spreadsheet::sheet_t _sheet, spreadsheet::row_t _row, spreadsheet::col_t _column,
            const std::string& _exp, const std::string& _range);
    };

    struct shared_formula
    {
        spreadsheet::sheet_t sheet;
        spreadsheet::row_t row;
        spreadsheet::col_t column;
        size_t identifier;
        std::string formula;
        std::string range;
        bool master;

        shared_formula(
            spreadsheet::sheet_t _sheet, spreadsheet::row_t _row, spreadsheet::col_t _column,
            size_t _identifier);

        shared_formula(
            spreadsheet::sheet_t _sheet, spreadsheet::row_t _row, spreadsheet::col_t _column,
            size_t _identifier, const std::string& _formula, const std::string& _range);
    };

    typedef std::vector<std::unique_ptr<formula>> formulas_type;
    typedef std::vector<std::unique_ptr<shared_formula>> shared_formulas_type;
    typedef std::unordered_map<pstring, spreadsheet::sheet_t, pstring::hash> sheet_name_map_type;

    formulas_type m_formulas;
    shared_formulas_type m_shared_formulas;
    sheet_name_map_type m_sheet_name_map;

    virtual ~xlsx_session_data();

    /**
     * @param name sheet name. Note that this must be already interned with
     *             the string pool of the session context.
     * @param id sheet index.
     */
    void set_sheet_name_map(pstring name, spreadsheet::sheet_t id);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
