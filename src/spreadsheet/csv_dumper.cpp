/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "csv_dumper.hpp"
#include "orcus/spreadsheet/document.hpp"

#include <ixion/model_context.hpp>
#include <ixion/formula_name_resolver.hpp>
#include <ixion/formula_result.hpp>
#include <mdds/multi_type_vector/collection.hpp>

#include <fstream>
#include <sstream>
#include <iostream>

namespace orcus { namespace spreadsheet { namespace detail {

using columns_type = mdds::mtv::collection<ixion::column_store_t>;

namespace {

void dump_string(std::ostream& os, const std::string& s)
{
    os << s;
}

void dump_cell_value(
    std::ostream& os, const ixion::model_context& cxt,
    const columns_type::const_iterator::value_type& node)
{
    switch (node.type)
    {
        case ixion::element_type_empty:
            break;
        case ixion::element_type_boolean:
        {
            auto b = node.get<ixion::boolean_element_block>();
            os << (b ? "true" : "false");
            break;
        }
        case ixion::element_type_numeric:
        {
            auto v = node.get<ixion::numeric_element_block>();
            os << v;
            break;
        }
        case ixion::element_type_string:
        {
            ixion::string_id_t sindex = node.get<ixion::string_element_block>();
            const std::string* p = cxt.get_string(sindex);
            assert(p);
            dump_string(os, *p);
            break;
        }
        case ixion::element_type_formula:
        {
            const ixion::formula_cell* cell = node.get<ixion::formula_element_block>();
            assert(cell);

            const ixion::formula_result& res = cell->get_result_cache();

            switch (res.get_type())
            {
                case ixion::formula_result::result_type::value:
                    os << res.get_value();
                break;
                case ixion::formula_result::result_type::string:
                {
                    ixion::string_id_t sid = res.get_string();
                    const std::string* p = cxt.get_string(sid);
                    assert(p);
                    dump_string(os, *p);
                }
                break;
                case ixion::formula_result::result_type::error:
                    os << "\"#ERR!\"";
                break;
            }
            break;
        }
        default:
            ;
    }
}

}

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

            dump_cell_value(file, cxt, node);
        }
    );
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
