/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "flat_dumper.hpp"
#include "number_format.hpp"
#include "orcus/spreadsheet/document.hpp"

#include <ixion/formula.hpp>
#include <ixion/model_context.hpp>
#include <ixion/model_iterator.hpp>
#include <ixion/formula_name_resolver.hpp>
#include <ixion/formula_result.hpp>

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

    const ixion::formula_name_resolver* resolver =
        m_doc.get_formula_name_resolver(spreadsheet::formula_ref_context_t::global);
    if (!resolver)
        return;

    size_t row_count = range.last.row + 1;
    size_t col_count = range.last.column + 1;
    os << "rows: " << row_count << "  cols: " << col_count << endl;

    // Always start at the top-left corner.
    range.first.row = 0;
    range.first.column = 0;
    ixion::model_iterator iter = cxt.get_model_iterator(
        sheet_id, ixion::rc_direction_t::vertical, range);

    std::vector<std::string> mx(row_count*col_count);

    auto to_pos = [col_count](size_t row, size_t col) -> size_t
    {
        return col_count * row + col;
    };

    // Calculate column widths as we iterate.
    std::vector<size_t> col_widths(col_count, 0);
    auto it_colwidth = col_widths.begin();
    col_t current_col = 0;

    for (; iter.has(); iter.next())
    {
        const ixion::model_iterator::cell& c = iter.get();
        if (c.col > current_col)
        {
            ++current_col;
            ++it_colwidth;
            assert(current_col == c.col);
        }

        size_t cell_str_width = 0;

        switch (c.type)
        {
            case ixion::celltype_t::string:
            {
                size_t sindex = c.value.string;
                const std::string* p = cxt.get_string(sindex);
                assert(p);
                mx[to_pos(c.row, c.col)] = std::move(*p);
                cell_str_width = p->size();
                break;
            }
            case ixion::celltype_t::numeric:
            {
                std::ostringstream os2;
                format_to_file_output(os2, c.value.numeric);
                os2 << " [v]";
                std::string s = os2.str();
                cell_str_width = s.size();
                mx[to_pos(c.row, c.col)] = std::move(s);
                break;
            }
            case ixion::celltype_t::boolean:
            {
                std::ostringstream os2;
                os2 << (c.value.boolean ? "true" : "false") << " [b]";
                std::string s = os2.str();
                cell_str_width = s.size();
                mx[to_pos(c.row, c.col)] = std::move(s);
                break;
            }
            case ixion::celltype_t::formula:
            {
                // print the formula and the formula result.
                const ixion::formula_cell* cell = c.value.formula;
                assert(cell);
                const ixion::formula_tokens_store_ptr_t& ts = cell->get_tokens();
                if (ts)
                {
                    const ixion::formula_tokens_t& tokens = ts->get();

                    std::ostringstream os2;
                    std::string formula;
                    if (resolver)
                    {
                        ixion::abs_address_t pos(sheet_id, c.row, c.col);
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

                    std::string s = os2.str();
                    cell_str_width = s.size();
                    mx[to_pos(c.row, c.col)] = std::move(s);
                }
                break;
            }
            default:
                ;
        }

        if (*it_colwidth < cell_str_width)
            *it_colwidth = cell_str_width;
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
            const std::string& s = mx[to_pos(r, c)];
            if (s.empty())
            {
                for (size_t i = 0; i < cw; ++i)
                    os << ' ';
                os << "  |";
            }
            else
            {
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
