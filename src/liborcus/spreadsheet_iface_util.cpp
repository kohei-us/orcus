/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "spreadsheet_iface_util.hpp"
#include "formula_result.hpp"

#include "pstring.hpp"
#include "orcus/spreadsheet/import_interface.hpp"

namespace orcus {

void push_array_formula(
    spreadsheet::iface::import_array_formula* xformula,
    const spreadsheet::range_t& range, std::string_view formula,
    spreadsheet::formula_grammar_t grammar, const range_formula_results& results)
{
    xformula->set_range(range);
    xformula->set_formula(grammar, formula);

    for (size_t row = 0; row < results.row_size(); ++row)
    {
        for (size_t col = 0; col < results.col_size(); ++col)
        {
            const formula_result& v = results.get(row, col);
            switch (v.type)
            {
                case formula_result::result_type::numeric:
                    xformula->set_result_value(row, col, v.value_numeric);
                    break;
                case formula_result::result_type::string:
                    xformula->set_result_string(row, col, {v.value_string.p, v.value_string.n});
                    break;
                case formula_result::result_type::boolean:
                    xformula->set_result_bool(row, col, v.value_boolean);
                    break;
                case formula_result::result_type::empty:
                    xformula->set_result_empty(row, col);
                    break;
                default:
                    ;
            }
        }
    }

    xformula->commit();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
