/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "json_dumper.hpp"
#include "dumper_global.hpp"

#include "orcus/json_global.hpp"
#include "orcus/spreadsheet/document.hpp"

#include <ixion/model_context.hpp>
#include <ixion/formula_name_resolver.hpp>

#include <fstream>
#include <sstream>
#include <iostream>

namespace orcus { namespace spreadsheet { namespace detail {

json_dumper::json_dumper(const document& doc) : m_doc(doc) {}

void json_dumper::dump(std::ostream& os, ixion::sheet_t sheet_id) const
{
    const ixion::model_context& cxt = m_doc.get_model_context();
    ixion::abs_range_t data_range = cxt.get_data_range(sheet_id);
    if (!data_range.valid())
        return;

    ixion::abs_rc_range_t iter_range;
    iter_range.first.column = 0;
    iter_range.first.row = 0;
    iter_range.last.column = data_range.last.column;
    iter_range.last.row = data_range.last.row;

    auto iter = cxt.get_model_iterator(
        sheet_id, ixion::rc_direction_t::horizontal, iter_range);

    std::vector<std::string> column_labels;
    column_labels.reserve(data_range.last.column+1);

    // Get the column labels.
    auto resolver = ixion::formula_name_resolver::get(ixion::formula_name_resolver_t::excel_a1, &cxt);
    for (ixion::col_t i = 0; i <= data_range.last.column; ++i)
        column_labels.emplace_back(resolver->get_column_name(i));

    os << "[" << std::endl;

    ixion::row_t row = iter.get().row;
    ixion::col_t col = iter.get().col;
    assert(row == 0);
    assert(col == 0);

    os << "    {";
    os << "\"" << column_labels[col] << "\": ";

    func_str_handler str_handler = [](std::ostream& _os, const std::string& s)
    {
        _os << '"' << json::escape_string(s) << '"';
    };

    func_empty_handler empty_handler = [](std::ostream& _os) { _os << "null"; };

    dump_cell_value(os, cxt, iter.get(), str_handler, empty_handler);

    ixion::row_t last_row = row;

    for (iter.next(); iter.has(); iter.next())
    {
        const auto& cell = iter.get();
        ixion::row_t this_row = cell.row;
        ixion::col_t this_col = cell.col;

        if (this_row > last_row)
            os << "}," << std::endl;

        if (this_col == 0)
            os << "    {";
        else
            os << ", ";

        os << "\"" << column_labels.at(this_col) << "\": ";

        dump_cell_value(os, cxt, cell, str_handler, empty_handler);
        last_row = this_row;
    }

    os << "}" << std::endl << "]" << std::endl;
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
