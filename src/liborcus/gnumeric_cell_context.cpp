/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gnumeric_token_constants.hpp"
#include "gnumeric_namespace_types.hpp"
#include "gnumeric_cell_context.hpp"
#include "orcus/spreadsheet/import_interface.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>

namespace ss = orcus::spreadsheet;

namespace orcus {

enum gnumeric_cell_type
{
    cell_type_bool,
    cell_type_value,
    cell_type_string,
    cell_type_formula,
    cell_type_shared_formula,
    cell_type_array,
    cell_type_unknown
};


struct gnumeric_cell_data
{
    gnumeric_cell_data() : row(0), col(0), cell_type(cell_type_unknown), shared_formula_id(-1),
                            array_rows(0), array_cols(0) {}
    ss::row_t row;
    ss::col_t col;
    gnumeric_cell_type cell_type;
    std::size_t shared_formula_id;
    ss::row_t array_rows;
    ss::col_t array_cols;
};

gnumeric_cell_context::gnumeric_cell_context(session_context& session_cxt, const tokens& tokens, ss::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory),
    mp_sheet(nullptr)
{
}

gnumeric_cell_context::~gnumeric_cell_context() = default;

void gnumeric_cell_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
{
    push_stack(ns, name);

    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_Cell:
                start_cell(attrs);
                break;
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool gnumeric_cell_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_Cell:
                end_cell();
                break;
            default:
                ;
        }
    }
    return pop_stack(ns, name);
}

void gnumeric_cell_context::characters(std::string_view str, bool transient)
{
    if (transient)
        m_chars = m_pool.intern(str).first;
    else
        m_chars = str;
}

void gnumeric_cell_context::reset(ss::iface::import_sheet* sheet)
{
    mp_cell_data.reset();
    m_pool.clear();
    m_chars = std::string_view{};
    mp_sheet = sheet;
}

void gnumeric_cell_context::start_cell(const xml_token_attrs_t& attrs)
{
    mp_cell_data.reset(new gnumeric_cell_data);
    mp_cell_data->cell_type = cell_type_formula;

    for (const auto& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_Row:
                mp_cell_data->row = atoi(attr.value.data());
                break;
            case XML_Col:
                mp_cell_data->col = atoi(attr.value.data());
                break;
            case XML_ValueType:
            {
                int value_type = atoi(attr.value.data());
                switch (value_type)
                {
                    case 20:
                        mp_cell_data->cell_type = cell_type_bool;
                        break;
                    case 30:
                    case 40:
                        mp_cell_data->cell_type = cell_type_value;
                        break;
                    case 60:
                        mp_cell_data->cell_type = cell_type_string;
                        break;
                }
                break;
            }
            case XML_ExprID:
                mp_cell_data->shared_formula_id = atoi(attr.value.data());
                mp_cell_data->cell_type = cell_type_shared_formula;
                break;
            case XML_Rows:
                mp_cell_data->array_rows = atoi(attr.value.data());
                mp_cell_data->cell_type = cell_type_array;
                break;
            case XML_Cols:
                mp_cell_data->array_cols = atoi(attr.value.data());
                mp_cell_data->cell_type = cell_type_array;
                break;
        }
    }
}

void gnumeric_cell_context::end_cell()
{
    if (!mp_cell_data)
        return;

    ss::col_t col = mp_cell_data->col;
    ss::row_t row = mp_cell_data->row;
    gnumeric_cell_type cell_type = mp_cell_data->cell_type;
    switch (cell_type)
    {
        case cell_type_value:
        {
            double val = atof(m_chars.data());
            mp_sheet->set_value(row, col, val);
            break;
        }
        case cell_type_string:
        {
            ss::iface::import_shared_strings* shared_strings = mp_factory->get_shared_strings();
            if (!shared_strings)
                break;

            size_t id = shared_strings->add(m_chars);
            mp_sheet->set_string(row, col, id);
            break;
        }
        case cell_type_formula:
        {
            ss::iface::import_formula* xformula = mp_sheet->get_formula();
            if (!xformula)
                break;

            if (m_chars.empty() || m_chars[0] != '=')
                // formula string should start with a '='
                break;

            xformula->set_position(row, col);
            xformula->set_formula(ss::formula_grammar_t::gnumeric, m_chars.substr(1));
            xformula->commit();
            break;
        }
        case cell_type_shared_formula:
        {
            ss::iface::import_formula* xformula = mp_sheet->get_formula();
            if (!xformula)
                break;

            xformula->set_position(row, col);

            if (m_chars.empty() || m_chars[0] != '=')
                // formula string should start with a '='
                break;

            xformula->set_formula(ss::formula_grammar_t::gnumeric, m_chars.substr(1));
            xformula->set_shared_formula_index(mp_cell_data->shared_formula_id);
            xformula->commit();
            break;
        }
        case cell_type_array:
        {
            ss::range_t range;
            range.first.column = col;
            range.first.row = row;
            range.last.column = col + mp_cell_data->array_cols - 1;
            range.last.row = row + mp_cell_data->array_rows - 1;

            ss::iface::import_array_formula* af = mp_sheet->get_array_formula();
            if (!af)
                break;

            if (m_chars.empty() || m_chars[0] != '=')
                // formula string should start with a '='
                break;

            af->set_range(range);
            af->set_formula(ss::formula_grammar_t::gnumeric, m_chars.substr(1));
            af->commit();
            break;
        }
        case cell_type_bool:
        {
            bool val = m_chars == "TRUE";
            mp_sheet->set_bool(row, col, val);
            break;
        }
        default:
            ;
    }

    mp_cell_data.reset();
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
