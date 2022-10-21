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
        std::string_view string_value;

        formula_result();
    };

    struct formula
    {
        spreadsheet::sheet_t sheet;
        spreadsheet::row_t   row;
        spreadsheet::col_t   column;

        spreadsheet::formula_grammar_t grammar;
        std::string_view exp;

        formula_result result;

        formula(
            spreadsheet::sheet_t _sheet, spreadsheet::row_t _row, spreadsheet::col_t _col,
            spreadsheet::formula_grammar_t _grammar, std::string_view _exp);
    };

    struct named_exp
    {
        std::string_view name;
        std::string_view expression;
        std::string_view base;

        named_exp_type type;
        spreadsheet::sheet_t scope; // >= 0 for sheet scope, or < 0 for global scope.

        named_exp(std::string_view _name, std::string_view _expression, std::string_view _base, named_exp_type _type, spreadsheet::sheet_t _scope);
    };

    std::deque<formula> formulas;
    std::deque<named_exp> named_exps;

    odf_styles_map_type styles_map;

    struct number_formats_store
    {
        std::map<std::string_view, std::size_t> name2id_map;
        std::map<std::size_t, std::string> id2code_map;

        std::string_view get_code(std::string_view name) const;
    };

    number_formats_store number_formats;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
