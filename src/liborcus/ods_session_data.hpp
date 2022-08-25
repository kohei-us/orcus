/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_ODS_SESSION_DATA_HPP
#define INCLUDED_ORCUS_ODS_SESSION_DATA_HPP

#include "session_context.hpp"
#include "odf_styles.hpp"

#include <orcus/spreadsheet/types.hpp>

#include <deque>

namespace orcus {

struct ods_session_data : public session_context::custom_data
{
    enum formula_result_type { rt_none, rt_numeric, rt_string, rt_error };
    enum named_exp_type { ne_unknown, ne_range, ne_expression };

    struct formula_result
    {
        formula_result_type type;
        double numeric_value;
        pstring string_value;

        formula_result();
    };

    struct formula
    {
        spreadsheet::sheet_t sheet;
        spreadsheet::row_t   row;
        spreadsheet::col_t   column;

        spreadsheet::formula_grammar_t grammar;
        pstring exp;

        formula_result result;

        formula(
            spreadsheet::sheet_t _sheet, spreadsheet::row_t _row, spreadsheet::col_t _col,
            spreadsheet::formula_grammar_t _grammar, const pstring& _exp);
    };

    struct named_exp
    {
        pstring name;
        pstring expression;
        pstring base;

        named_exp_type type;
        spreadsheet::sheet_t scope; // >= 0 for sheet scope, or < 0 for global scope.

        named_exp(const pstring& _name, const pstring& _expression, const pstring& _base, named_exp_type _type, spreadsheet::sheet_t _scope);
    };

    std::deque<formula> m_formulas;
    std::deque<named_exp> m_named_exps;

    odf_styles_map_type styles_map;

    virtual ~ods_session_data();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
