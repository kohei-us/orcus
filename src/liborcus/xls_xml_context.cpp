/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xls_xml_context.hpp"
#include "xls_xml_namespace_types.hpp"
#include "xls_xml_token_constants.hpp"
#include "orcus/spreadsheet/import_interface.hpp"
#include "orcus/measurement.hpp"

#include <iostream>

using namespace std;

namespace orcus {

namespace {

class sheet_attr_parser : public unary_function<xml_token_attr_t, void>
{
    pstring m_name;
public:
    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.ns == NS_xls_xml_ss)
        {
            switch (attr.name)
            {
                case XML_Name:
                    m_name = attr.value;
                break;
                default:
                    ;
            }
        }
    }

    pstring get_name() const { return m_name; }
};

class row_attr_parser : public unary_function<xml_token_attr_t, void>
{
    long m_row_index;
public:
    row_attr_parser() : m_row_index(-1) {}

    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.value.empty())
            return;

        if (attr.ns == NS_xls_xml_ss)
        {
            switch (attr.name)
            {
                case XML_Index:
                {
                    const char* p = attr.value.get();
                    const char* p_end = p + attr.value.size();
                    m_row_index = to_long(p, p_end);
                }
                break;
                default:
                    ;
            }
        }
    }

    long get_row_index() const { return m_row_index; }
};

}

xls_xml_context::named_exp::named_exp(const pstring& _name, const pstring& _expression, spreadsheet::sheet_t _scope) :
    name(_name), expression(_expression), scope(_scope) {}

xls_xml_context::xls_xml_context(session_context& session_cxt, const tokens& tokens, spreadsheet::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory),
    mp_cur_sheet(nullptr),
    mp_sheet_props(nullptr),
    m_cur_sheet(-1),
    m_cur_row(0), m_cur_col(0),
    m_cur_merge_down(0), m_cur_merge_across(0),
    m_cur_cell_type(ct_unknown),
    m_cur_cell_value(std::numeric_limits<double>::quiet_NaN())
{
}

xls_xml_context::~xls_xml_context()
{
}

bool xls_xml_context::can_handle_element(xmlns_id_t ns, xml_token_t name) const
{
    return true;
}

xml_context_base* xls_xml_context::create_child_context(xmlns_id_t ns, xml_token_t name)
{
    return nullptr;
}

void xls_xml_context::end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child)
{
}

void xls_xml_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_attrs_t& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);
    if (ns == NS_xls_xml_ss)
    {
        switch (name)
        {
            case XML_Workbook:
                // Do nothing.
                break;
            case XML_Worksheet:
            {
                xml_element_expected(parent, NS_xls_xml_ss, XML_Workbook);
                pstring sheet_name = for_each(attrs.begin(), attrs.end(), sheet_attr_parser()).get_name();
                mp_cur_sheet = mp_factory->append_sheet(sheet_name.get(), sheet_name.size());
                if (mp_cur_sheet)
                    mp_sheet_props = mp_cur_sheet->get_sheet_properties();
                m_cur_row = 0;
                m_cur_col = 0;
                ++m_cur_sheet;
                break;
            }
            case XML_Table:
                xml_element_expected(parent, NS_xls_xml_ss, XML_Worksheet);
                break;
            case XML_Row:
            {
                xml_element_expected(parent, NS_xls_xml_ss, XML_Table);
                m_cur_col = 0;
                long row_index = for_each(attrs.begin(), attrs.end(), row_attr_parser()).get_row_index();
                if (row_index > 0)
                {
                    // 1-based row index. Convert it to a 0-based one.
                    m_cur_row = row_index - 1;
                }
                break;
            }
            case XML_Cell:
                start_element_cell(parent, attrs);
                break;
            case XML_Data:
                start_element_data(parent, attrs);
                break;
            case XML_Names:
            {
                xml_elem_stack_t expected_parents;
                expected_parents.emplace_back(NS_xls_xml_ss, XML_Workbook);
                expected_parents.emplace_back(NS_xls_xml_ss, XML_Worksheet);

                xml_element_expected(parent, expected_parents);
                break;
            }
            case XML_NamedRange:
            {
                xml_element_expected(parent, NS_xls_xml_ss, XML_Names);

                pstring name, exp;

                for (const xml_token_attr_t& attr : attrs)
                {
                    if (attr.ns != NS_xls_xml_ss)
                        continue;

                    switch (attr.name)
                    {
                        case XML_Name:
                            name = attr.value;
                            if (attr.transient)
                                name = intern(name);
                            break;
                        case XML_RefersTo:
                        {
                            exp = attr.value;
                            if (exp.size() > 1 && exp[0] == '=')
                                exp = pstring(exp.data()+1, exp.size()-1);
                            if (!exp.empty() && attr.transient)
                                exp = intern(exp);
                            break;
                        }
                        default:
                            ;
                    }
                }

                if (!name.empty() && !exp.empty())
                {
                    if (m_cur_sheet >= 0)
                        m_named_exps_sheet.emplace_back(name, exp, m_cur_sheet);
                    else
                        m_named_exps_global.emplace_back(name, exp, -1);
                }

                break;
            }
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool xls_xml_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_xls_xml_ss)
    {
        switch (name)
        {
            case XML_Row:
                ++m_cur_row;
                break;
            case XML_Cell:
                end_element_cell();
                break;
            case XML_Data:
                end_element_data();
                break;
            case XML_Workbook:
                end_element_workbook();
                break;
            case XML_Worksheet:
                mp_cur_sheet = nullptr;
                break;
            default:
                ;
        }
    }
    return pop_stack(ns, name);
}

void xls_xml_context::characters(const pstring& str, bool transient)
{
    if (str.empty())
        return;

    const xml_token_pair_t& elem = get_current_element();

    if (elem.first == NS_xls_xml_ss && elem.second == XML_Data)
    {
        switch (m_cur_cell_type)
        {
            case ct_string:
            {
                if (transient)
                    m_cur_cell_string.push_back(m_pool.intern(str).first);
                else
                    m_cur_cell_string.push_back(str);

                break;
            }
            case ct_number:
            {
                const char* p = str.get();
                m_cur_cell_value = to_double(p, p + str.size());
                break;
            }
            case ct_datetime:
                m_cur_cell_datetime = to_date_time(str);
                break;
            default:
                if (get_config().debug)
                {
                    cout << "warning: unknown cell type '" << m_cur_cell_type
                        << "': characters='" << str << "'" << endl;
                }
        }
    }
}

void xls_xml_context::start_element_cell(const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    xml_element_expected(parent, NS_xls_xml_ss, XML_Row);

    long col_index = 0;
    pstring formula;

    m_cur_merge_across = 0; // extra column(s) that are part of the merged cell.
    m_cur_merge_down = 0; // extra row(s) that are part of the merged cell.

    std::for_each(attrs.begin(), attrs.end(),
        [&](const xml_token_attr_t& attr)
        {
            if (attr.value.empty())
                return;

            if (attr.ns != NS_xls_xml_ss)
                return;

            switch (attr.name)
            {
                case XML_Index:
                    col_index = to_long(attr.value);
                    break;
                case XML_Formula:
                    if (attr.value[0] == '=' && attr.value.size() > 1)
                    {
                        pstring s(attr.value.get()+1, attr.value.size()-1);
                        formula = s;
                        if (attr.transient)
                            formula = intern(s);
                    }
                    break;
                case XML_MergeAcross:
                    m_cur_merge_across = to_long(attr.value);
                    break;
                case XML_MergeDown:
                    m_cur_merge_down = to_long(attr.value);
                    break;
                default:
                    ;
            }
        }
    );

    if (!formula.empty())
        m_cur_cell_formula = formula;

    if (col_index > 0)
    {
        // 1-based column index. Convert it to a 0-based one.
        m_cur_col = col_index - 1;
    }
}

void xls_xml_context::end_element_cell()
{
    if (mp_sheet_props && (m_cur_merge_across > 0 || m_cur_merge_down > 0))
    {
        spreadsheet::range_t merge_range;
        merge_range.first.column = m_cur_col;
        merge_range.first.row = m_cur_row;
        merge_range.last.column = m_cur_col + m_cur_merge_across;
        merge_range.last.row = m_cur_row + m_cur_merge_down;

        mp_sheet_props->set_merge_cell_range(merge_range);
    }

    ++m_cur_col;
    if (m_cur_merge_across > 0)
        m_cur_col += m_cur_merge_across;
}

void xls_xml_context::start_element_data(
    const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    xml_element_expected(parent, NS_xls_xml_ss, XML_Cell);

    m_cur_cell_type = ct_unknown;
    m_cur_cell_string.clear();
    m_cur_cell_datetime = date_time_t();

    std::for_each(attrs.begin(), attrs.end(),
        [&](const xml_token_attr_t& attr)
        {
            if (attr.ns != NS_xls_xml_ss)
                return;

            switch (attr.name)
            {
                case XML_Type:
                {
                    if (attr.value == "String")
                        m_cur_cell_type = ct_string;
                    else if (attr.value == "Number")
                        m_cur_cell_type = ct_number;
                    else if (attr.value == "DateTime")
                        m_cur_cell_type = ct_datetime;
                }
                break;
                default:
                    ;
            }
        }
    );
}

void xls_xml_context::end_element_data()
{
    if (!m_cur_cell_formula.empty())
    {
        push_formula_cell();
        return;
    }

    switch (m_cur_cell_type)
    {
        case ct_number:
            mp_cur_sheet->set_value(m_cur_row, m_cur_col, m_cur_cell_value);
            break;
        case ct_string:
        {
            spreadsheet::iface::import_shared_strings* ss = mp_factory->get_shared_strings();
            if (!ss)
                return;

            if (m_cur_cell_string.empty())
                return;

            if (m_cur_cell_string.size() == 1)
            {
                const pstring& s = m_cur_cell_string.back();
                mp_cur_sheet->set_string(m_cur_row, m_cur_col, ss->append(&s[0], s.size()));
            }
            else
            {
                string s;
                vector<pstring>::const_iterator it = m_cur_cell_string.begin(), it_end = m_cur_cell_string.end();
                for (; it != it_end; ++it)
                    s += *it;

                mp_cur_sheet->set_string(m_cur_row, m_cur_col, ss->append(&s[0], s.size()));
            }
            m_cur_cell_string.clear();

            break;
        }
        case ct_datetime:
        {
            mp_cur_sheet->set_date_time(
                m_cur_row, m_cur_col,
                m_cur_cell_datetime.year, m_cur_cell_datetime.month, m_cur_cell_datetime.day,
                m_cur_cell_datetime.hour, m_cur_cell_datetime.minute, m_cur_cell_datetime.second);
            break;
        }
        default:
            if (get_config().debug)
                cout << "warning: unknown cell type '" << m_cur_cell_type << "': value not pushed." << endl;
    }
}

void xls_xml_context::end_element_workbook()
{
    spreadsheet::iface::import_named_expression* ne_global = mp_factory->get_named_expression();
    if (ne_global)
    {
        // global scope named expressions.

        for (const named_exp& ne : m_named_exps_global)
        {
            ne_global->define_name(
                ne.name.data(), ne.name.size(), ne.expression.data(), ne.expression.size());
        }
    }
}

void xls_xml_context::push_formula_cell()
{
    mp_cur_sheet->set_formula(
        m_cur_row, m_cur_col, spreadsheet::formula_grammar_t::xls_xml,
        m_cur_cell_formula.get(), m_cur_cell_formula.size());

    switch (m_cur_cell_type)
    {
        case ct_number:
            mp_cur_sheet->set_formula_result(m_cur_row, m_cur_col, m_cur_cell_value);
            break;
        default:
            ;
    }


    m_cur_cell_formula.clear();
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
