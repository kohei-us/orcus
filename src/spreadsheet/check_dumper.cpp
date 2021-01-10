/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "check_dumper.hpp"
#include "sheet_impl.hpp"
#include "number_format.hpp"
#include "orcus/spreadsheet/document.hpp"

#include <ixion/model_context.hpp>
#include <ixion/cell.hpp>
#include <ixion/formula.hpp>
#include <ixion/formula_name_resolver.hpp>
#include <ixion/formula_result.hpp>

#include <string>

namespace orcus { namespace spreadsheet { namespace detail {

namespace {

void write_cell_position(std::ostream& os, const pstring& sheet_name, row_t row, col_t col)
{
    os << sheet_name << '/' << row << '/' << col << ':';
}

std::string escape_chars(const std::string& str)
{
    if (str.empty())
        return str;

    std::string ret;
    const char* p = &str[0];
    const char* p_end = p + str.size();
    for (; p != p_end; ++p)
    {
        if (*p == '"')
            ret.push_back('\\');
        ret.push_back(*p);
    }
    return ret;
}

}

check_dumper::check_dumper(const sheet_impl& sheet, const pstring& sheet_name) :
    m_sheet(sheet), m_sheet_name(sheet_name)
{
}

void check_dumper::dump(std::ostream& os) const
{
    ixion::abs_range_t range = m_sheet.get_data_range();
    if (!range.valid())
        // Sheet is empty.  Nothing to print.
        return;

    const ixion::model_context& cxt = m_sheet.m_doc.get_model_context();
    const ixion::formula_name_resolver* resolver =
        m_sheet.m_doc.get_formula_name_resolver(spreadsheet::formula_ref_context_t::global);

    size_t row_count = range.last.row + 1;
    size_t col_count = range.last.column + 1;

    for (size_t row = 0; row < row_count; ++row)
    {
        for (size_t col = 0; col < col_count; ++col)
        {
            ixion::abs_address_t pos(m_sheet.m_sheet, row, col);
            switch (cxt.get_celltype(pos))
            {
                case ixion::celltype_t::string:
                {
                    write_cell_position(os, m_sheet_name, row, col);
                    size_t sindex = cxt.get_string_identifier(pos);
                    const std::string* p = cxt.get_string(sindex);
                    assert(p);
                    os << "string:\"" << escape_chars(*p) << '"' << std::endl;
                    break;
                }
                case ixion::celltype_t::numeric:
                {
                    write_cell_position(os, m_sheet_name, row, col);
                    os << "numeric:";
                    detail::format_to_file_output(os, cxt.get_numeric_value(pos));
                    os << std::endl;
                    break;
                }
                case ixion::celltype_t::boolean:
                {
                    write_cell_position(os, m_sheet_name, row, col);
                    os << "boolean:" << (cxt.get_boolean_value(pos) ? "true" : "false") << std::endl;
                    break;
                }
                case ixion::celltype_t::formula:
                {
                    write_cell_position(os, m_sheet_name, row, col);
                    os << "formula";

                    // print the formula and the formula result.
                    const ixion::formula_cell* cell = cxt.get_formula_cell(pos);
                    assert(cell);
                    const ixion::formula_tokens_store_ptr_t& ts = cell->get_tokens();
                    if (ts)
                    {
                        const ixion::formula_tokens_t& tokens = ts->get();
                        std::string formula;
                        if (resolver)
                        {
                            pos = cell->get_parent_position(pos);
                            formula = ixion::print_formula_tokens(
                                m_sheet.m_doc.get_model_context(), pos, *resolver, tokens);
                        }
                        else
                            formula = "???";

                        os << ':';

                        ixion::formula_group_t fg = cell->get_group_properties();

                        if (fg.grouped)
                            os << '{' << formula << '}';
                        else
                            os << formula;

                        try
                        {
                            ixion::formula_result res = cell->get_result_cache(
                                ixion::formula_result_wait_policy_t::throw_exception);
                            os << ':' << res.str(m_sheet.m_doc.get_model_context());
                        }
                        catch (const std::exception&)
                        {
                            os << ":#RES!";
                        }
                    }

                    os << std::endl;
                    break;
                }
                default:
                    ;
            }
        }
    }
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
