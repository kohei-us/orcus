/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "csv_dumper.hpp"
#include "dumper_global.hpp"
#include "orcus/spreadsheet/document.hpp"

#include <ixion/model_context.hpp>
#include <ixion/formula_name_resolver.hpp>
#include <ixion/formula_result.hpp>
#include <mdds/multi_type_vector/collection.hpp>

#include <fstream>
#include <sstream>
#include <iostream>

namespace orcus { namespace spreadsheet { namespace detail {

csv_dumper::csv_dumper(const document& doc) :
    m_doc(doc), m_sep(','), m_quote('"')
{
}

void csv_dumper::dump(const std::string& filepath, ixion::sheet_t sheet_id) const
{
    std::ofstream file(filepath.c_str());
    if (!file)
    {
        std::cerr << "failed to create file: " << filepath << std::endl;
        return;
    }

    const ixion::model_context& cxt = m_doc.get_model_context();
    ixion::abs_range_t data_range = cxt.get_data_range(sheet_id);
    const ixion::column_stores_t* p = cxt.get_columns(sheet_id);
    if (!p)
        return;

    columns_type columns(p->begin(), p->end());

    // Only iterate through the data range.
    columns.set_collection_range(0, data_range.last.column+1);
    columns.set_element_range(0, data_range.last.row+1);

    std::for_each(columns.begin(), columns.end(),
        [&](const columns_type::const_iterator::value_type& node)
        {
            size_t row = node.position;
            size_t col = node.index;

            if (col == 0 && row > 0)
                file << std::endl;

            if (col > 0)
                file << m_sep;

            dump_cell_value(file, cxt, node,
                [](std::ostream& os, const std::string& s)
                {
                    os << s;
                },
                [](std::ostream& os) {}
            );
        }
    );
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
