/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "flat_dumper.hpp"
#include "orcus/spreadsheet/document.hpp"

#include <ixion/formula.hpp>
#include <ixion/model_context.hpp>
#include <ixion/formula_name_resolver.hpp>
#include <ixion/formula_result.hpp>
#include <mdds/multi_type_matrix.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using std::cout;
using std::endl;

namespace orcus { namespace spreadsheet { namespace detail {

flat_dumper::flat_dumper(const document& doc) : m_doc(doc) {}

void flat_dumper::dump(std::ostream& os, ixion::sheet_t sheet_id) const
{
    const ixion::model_context& cxt = m_doc.get_model_context();
    ixion::abs_range_t range = cxt.get_data_range(sheet_id);
    if (!range.valid())
        // Sheet is empty.  Nothing to print.
        return;

    const ixion::formula_name_resolver* resolver = m_doc.get_formula_name_resolver();
    if (!resolver)
        return;

    size_t row_count = range.last.row + 1;
    size_t col_count = range.last.column + 1;
    os << "rows: " << row_count << "  cols: " << col_count << endl;

    typedef mdds::multi_type_matrix<mdds::mtm::std_string_trait> mx_type;
    mx_type mx(row_count, col_count);

    // Put all cell values into matrix as string elements first.
    for (size_t row = 0; row < row_count; ++row)
    {
        for (size_t col = 0; col < col_count; ++col)
        {
            ixion::abs_address_t pos(sheet_id, row, col);
            switch (cxt.get_celltype(pos))
            {
                case ixion::celltype_t::string:
                {
                    size_t sindex = cxt.get_string_identifier(pos);
                    const std::string* p = cxt.get_string(sindex);
                    assert(p);
                    mx.set(row, col, *p);
                    break;
                }
                case ixion::celltype_t::numeric:
                {
                    std::ostringstream os2;
                    os2 << cxt.get_numeric_value(pos) << " [v]";
                    mx.set(row, col, os2.str());
                    break;
                }
                case ixion::celltype_t::boolean:
                {
                    std::ostringstream os2;
                    os2 << (cxt.get_boolean_value(pos) ? "true" : "false") << " [b]";
                    mx.set(row, col, os2.str());
                    break;
                }
                case ixion::celltype_t::formula:
                {
                    // print the formula and the formula result.
                    const ixion::formula_cell* cell = cxt.get_formula_cell(pos);
                    assert(cell);
                    const ixion::formula_tokens_store_ptr_t& ts = cell->get_tokens();
                    if (ts)
                    {
                        const ixion::formula_tokens_t& tokens = ts->get();

                        std::ostringstream os2;
                        std::string formula;
                        if (resolver)
                        {
                            pos = cell->get_parent_position(pos);
                            formula = ixion::print_formula_tokens(
                               cxt, pos, *resolver, tokens);
                        }
                        else
                            formula = "???";

                        ixion::formula_group_t fg = cell->get_group_properties();

                        if (fg.grouped)
                            os2 << '{' << formula << '}';
                        else
                            os2 << formula;

                        ixion::formula_result res = cell->get_result_cache();
                        os2 << " (" << res.str(cxt) << ")";

                        mx.set(row, col, os2.str());
                    }
                    break;
                }
                default:
                    ;
            }
        }
    }

    // Calculate column widths first.
    mx_type::size_pair_type sp = mx.size();
    std::vector<size_t> col_widths(sp.column, 0);

    for (size_t r = 0; r < sp.row; ++r)
    {
        for (size_t c = 0; c < sp.column; ++c)
        {
            if (mx.get_type(r, c) == mdds::mtm::element_empty)
                continue;

            const std::string s = mx.get_string(r, c);
            if (col_widths[c] < s.size())
                col_widths[c] = s.size();
        }
    }

    // Create a row separator string;
    std::ostringstream os2;
    os2 << '+';
    for (size_t i = 0; i < col_widths.size(); ++i)
    {
        os2 << '-';
        size_t cw = col_widths[i];
        for (size_t j = 0; j < cw; ++j)
            os2 << '-';
        os2 << "-+";
    }

    std::string sep = os2.str();

    // Now print to stdout.
    os << sep << endl;
    for (size_t r = 0; r < row_count; ++r)
    {
        os << "|";
        for (size_t c = 0; c < col_count; ++c)
        {
            size_t cw = col_widths[c]; // column width
            if (mx.get_type(r, c) == mdds::mtm::element_empty)
            {
                for (size_t i = 0; i < cw; ++i)
                    os << ' ';
                os << "  |";
            }
            else
            {
                const std::string s = mx.get_string(r, c);
                os << ' ' << s;
                cw -= s.size();
                for (size_t i = 0; i < cw; ++i)
                    os << ' ';
                os << " |";
            }
        }
        os << endl;
        os << sep << endl;
    }
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
