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

xls_xml_data_context::xls_xml_data_context(
    session_context& session_cxt, const tokens& tokens, spreadsheet::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory),
    mp_cur_sheet(nullptr),
    m_row(0), m_col(0),
    m_cell_type(ct_unknown),
    m_cell_value(std::numeric_limits<double>::quiet_NaN())
{
}

bool xls_xml_data_context::can_handle_element(xmlns_id_t ns, xml_token_t name) const
{
    return true;
}

xml_context_base* xls_xml_data_context::create_child_context(xmlns_id_t ns, xml_token_t name)
{
    return nullptr;
}

void xls_xml_data_context::end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child)
{
}

void xls_xml_data_context::start_element(xmlns_id_t ns, xml_token_t name, const::std::vector<xml_token_attr_t>& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);

    if (ns == NS_xls_xml_ss)
    {
        switch (name)
        {
            case XML_Data:
                start_element_data(parent, attrs);
                break;
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

void xls_xml_data_context::characters(const pstring& str, bool transient)
{
    if (str.empty())
        return;

    switch (m_cell_type)
    {
        case ct_unknown:
            break;
        case ct_string:
        {
            if (transient)
                m_cell_string.push_back(intern(str));
            else
                m_cell_string.push_back(str);

            break;
        }
        case ct_number:
        {
            const char* p = str.get();
            m_cell_value = to_double(p, p + str.size());
            break;
        }
        case ct_datetime:
            m_cell_datetime = to_date_time(str);
            break;
        default:
            if (get_config().debug)
            {
                cout << "warning: unknown cell type '" << m_cell_type
                    << "': characters='" << str << "'" << endl;
            }
    }
}

bool xls_xml_data_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_xls_xml_ss)
    {
        switch (name)
        {
            case XML_Data:
                end_element_data();
                break;
            default:
                ;
        }
    }

    return pop_stack(ns, name);
}

void xls_xml_data_context::reset(
    spreadsheet::iface::import_sheet* sheet,
    spreadsheet::row_t row, spreadsheet::col_t col,
    const pstring& cell_formula)
{
    mp_cur_sheet = sheet;
    m_row = row;
    m_col = col;
    m_cell_formula = cell_formula;
}

void xls_xml_data_context::start_element_data(
    const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
//  xml_element_expected(parent, NS_xls_xml_ss, XML_Cell);

    m_cell_type = ct_unknown;
    m_cell_string.clear();
    m_cell_datetime = date_time_t();

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
                        m_cell_type = ct_string;
                    else if (attr.value == "Number")
                        m_cell_type = ct_number;
                    else if (attr.value == "DateTime")
                        m_cell_type = ct_datetime;
                }
                break;
                default:
                    ;
            }
        }
    );
}

void xls_xml_data_context::end_element_data()
{
    if (!m_cell_formula.empty())
    {
        push_formula_cell();
        return;
    }

    switch (m_cell_type)
    {
        case ct_unknown:
            break;
        case ct_number:
            mp_cur_sheet->set_value(m_row, m_col, m_cell_value);
            break;
        case ct_string:
        {
            spreadsheet::iface::import_shared_strings* ss = mp_factory->get_shared_strings();
            if (!ss)
                return;

            if (m_cell_string.empty())
                return;

            if (m_cell_string.size() == 1)
            {
                const pstring& s = m_cell_string.back();
                mp_cur_sheet->set_string(m_row, m_col, ss->append(&s[0], s.size()));
            }
            else
            {
                string s;
                vector<pstring>::const_iterator it = m_cell_string.begin(), it_end = m_cell_string.end();
                for (; it != it_end; ++it)
                    s += *it;

                mp_cur_sheet->set_string(m_row, m_col, ss->append(&s[0], s.size()));
            }
            m_cell_string.clear();

            break;
        }
        case ct_datetime:
        {
            mp_cur_sheet->set_date_time(
                m_row, m_col,
                m_cell_datetime.year, m_cell_datetime.month, m_cell_datetime.day,
                m_cell_datetime.hour, m_cell_datetime.minute, m_cell_datetime.second);
            break;
        }
        default:
            if (get_config().debug)
                cout << "warning: unknown cell type '" << m_cell_type << "': value not pushed." << endl;
    }

    m_cell_type = ct_unknown;
}

void xls_xml_data_context::push_formula_cell()
{
    mp_cur_sheet->set_formula(
        m_row, m_col, spreadsheet::formula_grammar_t::xls_xml,
        m_cell_formula.get(), m_cell_formula.size());

    switch (m_cell_type)
    {
        case ct_number:
            mp_cur_sheet->set_formula_result(m_row, m_col, m_cell_value);
            break;
        default:
            ;
    }

    m_cell_formula.clear();
}

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
    m_cc_data(session_cxt, tokens, factory)
{
}

xls_xml_context::~xls_xml_context()
{
}

bool xls_xml_context::can_handle_element(xmlns_id_t ns, xml_token_t name) const
{
    if (ns == NS_xls_xml_ss)
    {
        switch (name)
        {
            case XML_Data:
                return false;
            default:
                ;
        }
    }
    return true;
}

xml_context_base* xls_xml_context::create_child_context(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_xls_xml_ss)
    {
        switch (name)
        {
            case XML_Data:
            {
                // Move the cell formula string to the Data element context.
                m_cc_data.reset(mp_cur_sheet, m_cur_row, m_cur_col, m_cur_cell_formula);
                m_cur_cell_formula.clear();
                return &m_cc_data;
            }
            default:
                ;
        }
    }
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
                spreadsheet::iface::import_named_expression* sheet_named_exp = nullptr;
                if (mp_cur_sheet)
                {
                    mp_sheet_props = mp_cur_sheet->get_sheet_properties();
                    sheet_named_exp = mp_cur_sheet->get_named_expression();
                }

                m_sheet_named_exps.push_back(sheet_named_exp);

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
                            name = intern(attr);
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
            case XML_Styles:
            {
                xml_element_expected(parent, NS_xls_xml_ss, XML_Workbook);
                break;
            }
            case XML_Style:
            {
                xml_element_expected(parent, NS_xls_xml_ss, XML_Styles);

                pstring style_id, style_name;

                for (const xml_token_attr_t& attr : attrs)
                {
                    if (attr.ns != NS_xls_xml_ss)
                        continue;

                    switch (attr.name)
                    {
                        case XML_ID:
                            style_id = intern(attr);
                            break;
                        case XML_Name:
                            style_name = intern(attr);
                            break;
                        default:
                            ;
                    }
                }

                m_current_style = orcus::make_unique<style_type>();
                m_current_style->id = style_id;
                m_current_style->name = style_name;

                break;
            }
            case XML_Font:
            {
                xml_element_expected(parent, NS_xls_xml_ss, XML_Style);

                for (const xml_token_attr_t& attr : attrs)
                {
                    if (attr.ns != NS_xls_xml_ss)
                        continue;

                    switch (attr.name)
                    {
                        case XML_Bold:
                        {
                            m_current_style->font.bold = to_bool(attr.value);
                            break;
                        }
                        case XML_Italic:
                        {
                            m_current_style->font.italic = to_bool(attr.value);
                            break;
                        }
                        default:
                            ;
                    }
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
            case XML_Workbook:
                end_element_workbook();
                break;
            case XML_Worksheet:
                mp_cur_sheet = nullptr;
                break;
            case XML_Style:
            {
                if (m_current_style)
                {
                    if (m_current_style->id == "Default")
                        m_default_style = std::move(m_current_style);
                    else
                        m_styles.push_back(std::move(m_current_style));
                }
                break;
            }
            case XML_Styles:
            {
                end_element_styles();
                break;
            }
            default:
                ;
        }
    }
    return pop_stack(ns, name);
}

void xls_xml_context::characters(const pstring& str, bool transient)
{
}

void xls_xml_context::start_element_cell(const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    xml_element_expected(parent, NS_xls_xml_ss, XML_Row);

    long col_index = 0;
    pstring formula;
    m_cur_cell_style_id.clear();

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
                case XML_StyleID:
                    m_cur_cell_style_id = intern(attr);
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

    if (!m_cur_cell_style_id.empty())
    {
        auto it = m_style_map.find(m_cur_cell_style_id);
        if (it != m_style_map.end())
        {
            size_t xf_id = it->second;
            mp_cur_sheet->set_format(m_cur_row, m_cur_col, xf_id);
        }
    }

    ++m_cur_col;
    if (m_cur_merge_across > 0)
        m_cur_col += m_cur_merge_across;
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

    // sheet-local named expressions follow.

    for (const named_exp& ne : m_named_exps_sheet)
    {
        spreadsheet::iface::import_named_expression* p = nullptr;
        if (ne.scope >= 0 && size_t(ne.scope) < m_sheet_named_exps.size())
            p = m_sheet_named_exps[ne.scope]; // it may be nullptr.

        if (p)
            p->define_name(
                ne.name.data(), ne.name.size(), ne.expression.data(), ne.expression.size());
    }
}

void xls_xml_context::end_element_styles()
{
    commit_default_style(); // Commit the default style first.
    commit_styles();
}

void xls_xml_context::commit_default_style()
{
    spreadsheet::iface::import_styles* styles = mp_factory->get_styles();
    if (!styles)
        return;

    if (m_default_style)
    {
        styles->set_font_bold(m_default_style->font.bold);
        styles->set_font_italic(m_default_style->font.italic);
    }

    styles->commit_font();

    styles->commit_fill();
    styles->commit_border();
    styles->commit_cell_protection();
    styles->commit_number_format();

    styles->commit_cell_style_xf();
    styles->commit_cell_xf();

    if (m_default_style)
    {
        const pstring& name = m_default_style->name;
        if (!name.empty())
            styles->set_cell_style_name(name.data(), name.size());
    }

    styles->commit_cell_style();
}

void xls_xml_context::commit_styles()
{
    if (m_styles.empty())
        return;

    spreadsheet::iface::import_styles* styles = mp_factory->get_styles();
    if (!styles)
        return;

    // Build a map of cell style textural ID's to cell format (xf) numeric ID's.

    for (const std::unique_ptr<style_type>& style : m_styles)
    {
        styles->set_font_bold(style->font.bold);
        styles->set_font_italic(style->font.italic);

        size_t font_id = styles->commit_font();

        styles->set_xf_font(font_id);

        size_t xf_id = styles->commit_cell_xf();

        m_style_map.insert({style->id, xf_id});
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
