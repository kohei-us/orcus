/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gnumeric_token_constants.hpp"
#include "gnumeric_namespace_types.hpp"
#include "gnumeric_cell_context.hpp"

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/measurement.hpp>

#include <iostream>
#include <fstream>
#include <algorithm>

namespace ss = orcus::spreadsheet;

namespace orcus {

gnumeric_cell_context::gnumeric_cell_context(
    session_context& session_cxt, const tokens& tokens, ss::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory),
    mp_sheet(nullptr)
{
}

gnumeric_cell_context::~gnumeric_cell_context() = default;

void gnumeric_cell_context::start_element(
    xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
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
    m_chars = str;
    if (transient)
        m_chars = intern(m_chars);
}

void gnumeric_cell_context::reset(ss::iface::import_sheet* sheet)
{
    m_cell_data.reset();
    m_chars = std::string_view{};
    mp_sheet = sheet;
}

void gnumeric_cell_context::start_cell(const xml_token_attrs_t& attrs)
{
    m_cell_data = cell_data{};
    m_cell_data->type = cell_type_formula;

    for (const auto& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_Row:
                m_cell_data->row = to_long(attr.value);
                break;
            case XML_Col:
                m_cell_data->col = to_long(attr.value);
                break;
            case XML_ValueType:
            {
                auto v = to_long(attr.value);
                m_cell_data->type = cell_type_value;
                m_cell_data->value_type = static_cast<gnumeric_value_type>(v);
                break;
            }
            case XML_ExprID:
                m_cell_data->shared_formula_id = to_long(attr.value);
                m_cell_data->type = cell_type_shared_formula;
                break;
            case XML_Rows:
                m_cell_data->type = cell_type_value;
                m_cell_data->value_type = vt_array;
                m_cell_data->array_rows = to_long(attr.value);
                break;
            case XML_Cols:
                m_cell_data->type = cell_type_value;
                m_cell_data->value_type = vt_array;
                m_cell_data->array_cols = to_long(attr.value);
                break;
        }
    }
}

void gnumeric_cell_context::end_cell()
{
    if (!m_cell_data)
        return;

    if (!mp_sheet)
        return;

    ss::col_t col = m_cell_data->col;
    ss::row_t row = m_cell_data->row;

    switch (m_cell_data->type)
    {
        case cell_type_value:
        {
            if (!m_cell_data->value_type)
                break;

            switch (*m_cell_data->value_type)
            {
                case vt_boolean:
                {
                    bool val = to_bool(m_chars);
                    mp_sheet->set_bool(row, col, val);
                    break;
                }
                case vt_float:
                {
                    double val = to_double(m_chars);
                    mp_sheet->set_value(row, col, val);
                    break;
                }
                case vt_string:
                {
                    ss::iface::import_shared_strings* shared_strings = mp_factory->get_shared_strings();
                    if (!shared_strings)
                        break;

                    size_t id = shared_strings->add(m_chars);
                    mp_sheet->set_string(row, col, id);
                    break;
                }
                case vt_array:
                {
                    ss::range_t range;
                    range.first.column = col;
                    range.first.row = row;
                    range.last.column = col + m_cell_data->array_cols - 1;
                    range.last.row = row + m_cell_data->array_rows - 1;

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
                default:
                {
                    std::ostringstream os;
                    os << "unhandled cell value type (" << *m_cell_data->value_type << ")";
                    warn(os.str());
                }
            }
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

            if (!m_chars.empty() && m_chars[0] == '=')
                xformula->set_formula(ss::formula_grammar_t::gnumeric, m_chars.substr(1));

            xformula->set_shared_formula_index(m_cell_data->shared_formula_id);
            xformula->commit();
            break;
        }
        case cell_type_unknown:
        {
            std::ostringstream os;
            os << "cell type is unknown (row=" << row << "; col=" << col << ")";
            warn(os.str());
            break;
        }
    }

    m_cell_data.reset();
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
