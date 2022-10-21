/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ods_session_data.hpp"

#include <limits>

namespace orcus {

ods_session_data::formula_result::formula_result() :
    type(rt_none), numeric_value(std::numeric_limits<double>::quiet_NaN()) {}

ods_session_data::formula::formula(
    spreadsheet::sheet_t _sheet, spreadsheet::row_t _row, spreadsheet::col_t _col,
    spreadsheet::formula_grammar_t _grammar, std::string_view _exp) :
    sheet(_sheet), row(_row), column(_col), grammar(_grammar), exp(_exp) {}

ods_session_data::named_exp::named_exp(
    std::string_view _name, std::string_view _expression, std::string_view _base, named_exp_type _type, spreadsheet::sheet_t _scope) :
    name(_name), expression(_expression), base(_base), type(_type), scope(_scope) {}

std::string_view ods_session_data::number_formats_store::get_code(std::string_view name) const
{
    auto it_name = name2id_map.find(name);
    if (it_name == name2id_map.end())
        return {};

    std::size_t id = it_name->second;
    auto it_code = id2code_map.find(id);
    if (it_code == id2code_map.end())
        return {};

    return it_code->second;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
