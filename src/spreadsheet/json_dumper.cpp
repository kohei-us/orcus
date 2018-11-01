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

    const ixion::column_stores_t* p = cxt.get_columns(sheet_id);
    if (!p)
        return;

    columns_type columns(p->begin(), p->end());

    // Only iterate through the data range.
    columns.set_collection_range(0, data_range.last.column+1);
    columns.set_element_range(0, data_range.last.row+1);

    std::vector<std::string> column_labels;
    column_labels.reserve(data_range.last.column+1);

    // Get the column labels.
    auto resolver = ixion::formula_name_resolver::get(ixion::formula_name_resolver_t::excel_a1, &cxt);
    for (ixion::col_t i = 0; i <= data_range.last.column; ++i)
        column_labels.emplace_back(resolver->get_column_name(i));

    columns_type::const_iterator it = columns.begin();

    os << "[" << std::endl;

    size_t row = it->position;
    size_t col = it->index;

    os << "    {";
    os << "\"" << column_labels[col] << "\": ";

    func_str_handler str_handler = [](std::ostream& _os, const std::string& s)
    {
        _os << '"' << json::escape_string(s) << '"';
    };

    func_empty_handler empty_handler = [](std::ostream& _os) { _os << "null"; };

    dump_cell_value(os, cxt, *it, str_handler, empty_handler);

    size_t last_col = col;
    size_t last_row = row;

    std::for_each(++it, columns.end(),
        [&](const columns_type::const_iterator::value_type& node)
        {
            size_t this_row = node.position;
            size_t this_col = node.index;

            if (this_row > last_row)
                os << "}," << std::endl;

            if (this_col == 0)
                os << "    {";
            else
                os << ", ";

            os << "\"" << column_labels[this_col] << "\": ";

            dump_cell_value(os, cxt, node, str_handler, empty_handler);

            last_col = node.index;
            last_row = node.position;
        }
    );

    os << "}" << std::endl << "]" << std::endl;
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
