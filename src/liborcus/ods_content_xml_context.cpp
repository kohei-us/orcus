/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ods_content_xml_context.hpp"
#include "odf_token_constants.hpp"
#include "odf_namespace_types.hpp"
#include "odf_styles_context.hpp"
#include "session_context.hpp"
#include "ods_session_data.hpp"

#include "orcus/global.hpp"
#include "orcus/spreadsheet/import_interface.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstring>

#include <mdds/sorted_string_map.hpp>

using namespace std;
namespace ss = orcus::spreadsheet;

namespace orcus {

namespace {

typedef mdds::sorted_string_map<ods_content_xml_context::cell_value_type> cell_value_map_type;

// Keys must be sorted.
cell_value_map_type::entry cell_value_entries[] = {
    { ORCUS_ASCII("date"),   ods_content_xml_context::vt_date },
    { ORCUS_ASCII("float"),  ods_content_xml_context::vt_float },
    { ORCUS_ASCII("string"), ods_content_xml_context::vt_string }
};

const cell_value_map_type& get_cell_value_map()
{
    static cell_value_map_type cv_map(
        cell_value_entries,
        sizeof(cell_value_entries)/sizeof(cell_value_entries[0]),
        ods_content_xml_context::vt_unknown);

    return cv_map;
}

class null_date_attr_parser : public unary_function<xml_token_attr_t, void>
{
public:
    null_date_attr_parser() {}
    null_date_attr_parser(const null_date_attr_parser& r) :
        m_date_value(r.m_date_value)
    {
    }

    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.ns == NS_odf_table && attr.name == XML_date_value)
            m_date_value = attr.value;
    }

    const pstring& get_date_value() const { return m_date_value; }
private:
    pstring m_date_value;
};

class table_attr_parser : public unary_function<xml_token_attr_t, void>
{
public:
    table_attr_parser() {}
    table_attr_parser(const table_attr_parser& r) :
        m_name(r.m_name)
    {
    }

    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.ns == NS_odf_table && attr.name == XML_name)
            m_name = attr.value;
    }

    const pstring& get_name() const { return m_name; }
private:
    pstring m_name;
};

class cell_attr_parser
{
public:
    cell_attr_parser(session_context& cxt, ods_content_xml_context::cell_attr& attr) :
        m_cxt(cxt), m_attr(attr) {}
    cell_attr_parser(const cell_attr_parser& r) :
        m_cxt(r.m_cxt), m_attr(r.m_attr) {}

    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.value.empty())
            return;

        if (attr.ns == NS_odf_table)
            process_ns_table(attr);

        if (attr.ns == NS_odf_office)
            process_ns_office(attr);
    }

private:

    void process_formula(const pstring& str, bool transient)
    {
        if (str.empty())
            return;

        // Formula is prefixed with formula type, followed by a ':'
        // then the actual formula content.

        // First, detect prefix if any.  Only try up to the first 5 characters.
        const char* p0 = str.get();
        const char* end = p0 + std::min<size_t>(str.size(), 5);
        size_t prefix_size = 0;
        for (const char* p = p0; p != end; ++p)
        {
            if (*p == ':')
            {
                // Prefix separator found.
                prefix_size = p - p0;
                break;
            }

            if (!is_alpha(*p))
                // Only alphabets are allowed in the prefix space.
                break;
        }

        pstring prefix, formula;
        if (prefix_size)
        {
            prefix = pstring(p0, prefix_size);
            const char* p = p0;
            p += prefix_size + 1;
            end = p0 + str.size();
            formula = pstring(p, end - p);
        }
        else
        {
            // TODO : Handle cases where a formula doesn't have a prefix.
        }

        // Formula needs to begin with '='.
        if (formula.empty() || formula[0] != '=')
            return;

        // Remove the '='.
        formula = pstring(formula.get()+1, formula.size()-1);

        if (prefix == "of")
        {
            // ODF formula.  No action needed.
        }

        m_attr.formula = formula;
        if (transient)
            m_attr.formula = m_cxt.m_string_pool.intern(m_attr.formula).first;
    }

    void process_ns_table(const xml_token_attr_t &attr)
    {
        switch (attr.name)
        {
            case XML_style_name:
            {
                m_attr.style_name = attr.value;
                if (attr.transient)
                    m_attr.style_name = m_cxt.m_string_pool.intern(m_attr.style_name).first;
                break;
            }
            case XML_number_columns_repeated:
                m_attr.number_columns_repeated = to_long(attr.value);
                break;
            case XML_formula:
                process_formula(attr.value, attr.transient);
                break;
            default:
                ;
        }
    }

    void process_ns_office(const xml_token_attr_t &attr)
    {
        switch (attr.name)
        {
            case XML_value:
            {
                const char* end = attr.value.get() + attr.value.size();
                char* endptr;
                double val = strtod(attr.value.get(), &endptr);
                if (endptr == end)
                    m_attr.value = val;
            }
            break;
            case XML_value_type:
            {
                const cell_value_map_type& cv_map = get_cell_value_map();
                m_attr.type = cv_map.find(attr.value.get(), attr.value.size());
            }
            break;
            case XML_date_value:
                m_attr.date_value = attr.value;
            break;
            default:
                ;
        }
    }

    session_context& m_cxt;
    ods_content_xml_context::cell_attr& m_attr;
};

void pick_up_named_range_or_expression(
    session_context& cxt, const xml_attrs_t& attrs, xmlns_id_t exp_attr_ns, xml_token_t exp_attr_name,
    ods_session_data::named_exp_type name_type, ss::sheet_t scope)
{
    pstring name;
    pstring expression;
    pstring base;

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns == exp_attr_ns && attr.name == exp_attr_name)
        {
            expression = cxt.intern(attr);
            continue;
        }

        switch (attr.name)
        {
            case XML_name:
                name = cxt.intern(attr);
                break;
            case XML_base_cell_address:
                base = cxt.intern(attr);
                break;
        }
    }

    ods_session_data& ods_data = static_cast<ods_session_data&>(*cxt.mp_data);

    if (!name.empty() && !expression.empty() && !base.empty())
        ods_data.m_named_exps.emplace_back(name, expression, base, name_type, scope);
}

}

// ============================================================================

ods_content_xml_context::sheet_data::sheet_data() :
    sheet(nullptr), index(-1) {}

void ods_content_xml_context::sheet_data::reset()
{
    sheet = nullptr;
    index = -1;
}

ods_content_xml_context::row_attr::row_attr() :
    number_rows_repeated(1)
{
}

ods_content_xml_context::cell_attr::cell_attr() :
    number_columns_repeated(1),
    type(vt_unknown),
    value(0.0),
    formula_grammar(spreadsheet::formula_grammar_t::ods)
{
}

// ============================================================================

ods_content_xml_context::ods_content_xml_context(session_context& session_cxt, const tokens& tokens, spreadsheet::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory),
    m_row(0), m_col(0),
    m_para_index(0),
    m_has_content(false),
    m_styles(),
    m_child_para(session_cxt, tokens, factory->get_shared_strings(), m_styles),
    m_child_dde_links(session_cxt, tokens)
{
    spreadsheet::iface::import_global_settings* gs = mp_factory->get_global_settings();
    if (gs)
    {
        // Set the default null date to 1899-12-30 per specification (19.614).
        gs->set_origin_date(1899, 12, 30);
    }
}

ods_content_xml_context::~ods_content_xml_context()
{
}

bool ods_content_xml_context::can_handle_element(xmlns_id_t ns, xml_token_t name) const
{
    if (ns == NS_odf_text && name == XML_p)
        return false;

    if (ns == NS_odf_office && name == XML_automatic_styles)
        return false;

    if (ns == NS_odf_table && name == XML_dde_links)
        return false;

    return true;
}

xml_context_base* ods_content_xml_context::create_child_context(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_odf_text && name == XML_p)
    {
        m_child_para.reset();
        m_child_para.transfer_common(*this);
        return &m_child_para;
    }

    if (ns == NS_odf_office && name == XML_automatic_styles)
    {
        mp_child.reset(new styles_context(get_session_context(), get_tokens(), m_styles, mp_factory->get_styles()));
        mp_child->transfer_common(*this);
        return mp_child.get();
    }

    if (ns == NS_odf_table && name == XML_dde_links)
    {
        m_child_dde_links.reset();
        m_child_dde_links.transfer_common(*this);
        return &m_child_dde_links;
    }

    return nullptr;
}

void ods_content_xml_context::end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child)
{
    if (ns == NS_odf_text && name == XML_p)
    {
        text_para_context* para_context = static_cast<text_para_context*>(child);
        m_has_content = !para_context->empty();
        m_para_index = para_context->get_string_index();
    }
    else if (ns == NS_odf_office && name == XML_automatic_styles)
    {
        if (get_config().debug)
            cout << "styles picked up:" << endl;

        odf_styles_map_type::const_iterator it = m_styles.begin(), it_end = m_styles.end();
        for (; it != it_end; ++it)
        {
            if (get_config().debug)
                cout << "  style: " << it->first << " [ ";

            switch (it->second->family)
            {
                case style_family_table_column:
                {
                    if (get_config().debug)
                        cout << "column width: " << it->second->column_data->width.to_string();
                    break;
                }
                case style_family_table_row:
                {
                    if (get_config().debug)
                        cout << "row height: " << it->second->row_data->height.to_string();
                    break;
                }
                case style_family_table_cell:
                {
                    const odf_style::cell& cell = *it->second->cell_data;
                    if (get_config().debug)
                        cout << "xf ID: " << cell.xf;

                    spreadsheet::iface::import_styles* styles = mp_factory->get_styles();
                    if (styles)
                    {
                        // TODO: Actually we need a boolean flag to see if it is an automatic style or a real style
                        //  currently we have no way to set a real style to a cell anyway
                        m_cell_format_map.insert(name2id_type::value_type(it->first, cell.xf));
                    }
                    break;
                }
                case style_family_text:
                {
                    if (get_config().debug)
                    {
                        const odf_style::text& data = *it->second->text_data;
                        cout << "font ID: " << data.font;
                    }
                    break;
                }
                default:
                    ;
            }

            if (get_config().debug)
                cout << " ]" << endl;
        }
    }
}

void ods_content_xml_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_attrs_t& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);

    if (ns == NS_odf_office)
    {
        switch (name)
        {
            case XML_body:
                break;
            case XML_spreadsheet:
                break;
            default:
                warn_unhandled();
        }
    }
    else if (ns == NS_odf_table)
    {
        switch (name)
        {
            case XML_calculation_settings:
                break;
            case XML_null_date:
                xml_element_expected(parent, NS_odf_table, XML_calculation_settings);
                start_null_date(attrs);
                break;
            case XML_table:
                start_table(parent, attrs);
                break;
            case XML_table_column:
            {
                static const xml_elem_set_t expected = {
                    { NS_odf_table, XML_table },
                    { NS_odf_table, XML_table_column_group },
                    { NS_odf_table, XML_table_columns },
                    { NS_odf_table, XML_table_header_columns }
                };
                xml_element_expected(parent, expected);
                start_column(attrs);
                break;
            }
            case XML_table_row:
            {
                static const xml_elem_set_t expected = {
                    { NS_odf_table, XML_table },
                    { NS_odf_table, XML_table_header_rows },
                    { NS_odf_table, XML_table_row_group },
                };
                xml_element_expected(parent, expected);
                start_row(attrs);
                break;
            }
            case XML_table_cell:
                xml_element_expected(parent, NS_odf_table, XML_table_row);
                start_cell(attrs);
                break;
            case XML_dde_links:
                xml_element_expected(parent, NS_odf_office, XML_spreadsheet);
                break;
            case XML_dde_link:
                xml_element_expected(parent, NS_odf_table, XML_dde_links);
                break;
            case XML_named_expressions:
            {
                static const xml_elem_set_t expected = {
                    { NS_odf_office, XML_spreadsheet },
                    { NS_odf_table, XML_table },
                };
                xml_element_expected(parent, expected);
                break;
            }
            case XML_named_range:
                start_named_range(parent, attrs);
                break;
            case XML_named_expression:
                start_named_expression(parent, attrs);
                break;
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool ods_content_xml_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_odf_office)
    {
        switch (name)
        {
            case XML_body:
                break;
            case XML_spreadsheet:
                end_spreadsheet();
                break;
            default:
                ;
        }
    }
    else if (ns == NS_odf_table)
    {
        switch (name)
        {
            case XML_calculation_settings:
                break;
            case XML_null_date:
                break;
            case XML_table:
                end_table();
                break;
            case XML_table_column:
                end_column();
                break;
            case XML_table_row:
                end_row();
                break;
            case XML_table_cell:
                end_cell();
                break;
            case XML_named_range:
                end_named_range();
                break;
            case XML_named_expression:
                end_named_expression();
                break;
            default:
                ;
        }
    }
    return pop_stack(ns, name);
}

void ods_content_xml_context::characters(const pstring& /*str*/, bool /*transient*/)
{
}

void ods_content_xml_context::start_null_date(const xml_attrs_t& attrs)
{
    spreadsheet::iface::import_global_settings* gs = mp_factory->get_global_settings();
    if (!gs)
        // Global settings not available. No point going further.
        return;

    pstring null_date = for_each(attrs.begin(), attrs.end(), null_date_attr_parser()).get_date_value();
    date_time_t val = to_date_time(null_date);

    gs->set_origin_date(val.year, val.month, val.day);
}

void ods_content_xml_context::start_table(const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    static const xml_elem_set_t expected = {
        { NS_odf_office, XML_spreadsheet },
        { NS_odf_table, XML_dde_link },
    };
    xml_element_expected(parent, expected);

    if (parent == xml_token_pair_t(NS_odf_office, XML_spreadsheet))
    {
        table_attr_parser parser = for_each(attrs.begin(), attrs.end(), table_attr_parser());
        const pstring& name = parser.get_name();
        m_tables.push_back(mp_factory->append_sheet(m_tables.size(), name.get(), name.size()));
        m_cur_sheet.sheet = m_tables.back();
        m_cur_sheet.index = m_tables.size() - 1;

        if (get_config().debug)
            cout << "start table " << name << endl;

        m_row = m_col = 0;
    }
    else if (parent == xml_token_pair_t(NS_odf_table, XML_dde_link))
    {
        if (get_config().debug)
            cout << "start table (DDE link)" << endl;
    }
}

void ods_content_xml_context::end_table()
{
    if (m_cur_sheet.sheet)
    {
        if (get_config().debug)
            cout << "end table" << endl;

        m_cur_sheet.reset();
    }
}

void ods_content_xml_context::start_named_range(const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    xml_element_expected(parent, NS_odf_table, XML_named_expressions);

    pick_up_named_range_or_expression(
        get_session_context(), attrs, NS_odf_table, XML_cell_range_address,
        ods_session_data::ne_range, m_cur_sheet.index);
}

void ods_content_xml_context::end_named_range()
{
}

void ods_content_xml_context::start_named_expression(const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    xml_element_expected(parent, NS_odf_table, XML_named_expressions);

    pick_up_named_range_or_expression(
        get_session_context(), attrs, NS_odf_table, XML_expression,
        ods_session_data::ne_expression, m_cur_sheet.index);
}

void ods_content_xml_context::end_named_expression()
{
}

void ods_content_xml_context::start_column(const xml_attrs_t& attrs)
{
    if (!m_cur_sheet.sheet)
        return;

    spreadsheet::iface::import_sheet_properties* sheet_props =
        m_cur_sheet.sheet->get_sheet_properties();

    if (!sheet_props)
        return;

    pstring style_name;

    std::for_each(attrs.begin(), attrs.end(),
        [&style_name](const xml_token_attr_t& attr)
        {
            if (attr.ns == NS_odf_table)
            {
                if (attr.name == XML_style_name)
                    style_name = attr.value;
            }
        }
    );

    odf_styles_map_type::const_iterator it = m_styles.find(style_name);
    if (it == m_styles.end())
        // Style by this name not found.
        return;

    const odf_style& style = *it->second;
    sheet_props->set_column_width(m_col, style.column_data->width.value, style.column_data->width.unit);
}

void ods_content_xml_context::end_column()
{
    ++m_col;
}

void ods_content_xml_context::start_row(const xml_attrs_t& attrs)
{
    m_col = 0;
    m_row_attr = row_attr();

    pstring style_name;

    std::for_each(attrs.begin(), attrs.end(),
        [&](const xml_token_attr_t& attr)
        {
            if (attr.ns == NS_odf_table)
            {
                switch (attr.name)
                {
                    case XML_number_rows_repeated:
                        m_row_attr.number_rows_repeated = to_long(attr.value);
                        break;
                    case XML_style_name:
                        style_name = attr.value;
                        break;
                }
            }
        }
    );

    if (get_config().debug)
        cout << "row: (style='" << style_name << "')" << endl;

    if (!m_cur_sheet.sheet)
        return;

    // Pass row properties to the interface.
    spreadsheet::iface::import_sheet_properties* sheet_props =
        m_cur_sheet.sheet->get_sheet_properties();

    if (sheet_props)
    {
        odf_styles_map_type::const_iterator it = m_styles.find(style_name);
        if (it != m_styles.end())
        {
            const odf_style& style = *it->second;
            if (style.family == style_family_table_row)
            {
                const odf_style::row& row_data = *style.row_data;
                if (row_data.height_set)
                    sheet_props->set_row_height(m_row, row_data.height.value, row_data.height.unit);
            }
        }
    }
}

void ods_content_xml_context::end_row()
{
    if (m_row_attr.number_rows_repeated > 1)
    {
        // TODO: repeat this row.
        if (get_config().debug)
            cout << "TODO: repeat this row " << m_row_attr.number_rows_repeated << " times." << endl;
    }
    m_row += m_row_attr.number_rows_repeated;
}

void ods_content_xml_context::start_cell(const xml_attrs_t& attrs)
{
    m_cell_attr = cell_attr();
    for_each(attrs.begin(), attrs.end(), cell_attr_parser(get_session_context(), m_cell_attr));
}

void ods_content_xml_context::end_cell()
{
    name2id_type::const_iterator it = m_cell_format_map.find(m_cell_attr.style_name);
    if (m_cur_sheet.sheet && it != m_cell_format_map.end())
        m_cur_sheet.sheet->set_format(m_row, m_col, it->second);

    push_cell_value();

    ++m_col;
    if (m_cell_attr.number_columns_repeated > 1)
    {
        int col_upper = m_col + m_cell_attr.number_columns_repeated - 2;
        for (; m_col <= col_upper; ++m_col)
            push_cell_value();
    }
    m_has_content = false;
}

void ods_content_xml_context::push_cell_value()
{
    assert(m_cur_sheet.index >= 0); // this is expected to be called only within a valid sheet scope.

    bool has_formula = !m_cell_attr.formula.empty();
    if (has_formula)
    {
        // Store formula cell data for later processing.
        ods_session_data& ods_data =
            static_cast<ods_session_data&>(*get_session_context().mp_data);
        ods_data.m_formulas.emplace_back(
            m_cur_sheet.index, m_row, m_col, m_cell_attr.formula_grammar, m_cell_attr.formula);

        ods_session_data::formula& formula_data = ods_data.m_formulas.back();

        // Store formula result.
        switch (m_cell_attr.type)
        {
            case vt_float:
            {
                formula_data.result.type = orcus::ods_session_data::rt_numeric;
                formula_data.result.numeric_value = m_cell_attr.value;
                break;
            }
            case vt_string:
                // TODO : pass string result here.  We need to decide whether
                // to pass a string ID or a raw string.
                break;
            default:
                ;
        }
        return;
    }

    if (m_cur_sheet.sheet)
    {
        switch (m_cell_attr.type)
        {
            case vt_float:
                m_cur_sheet.sheet->set_value(m_row, m_col, m_cell_attr.value);
                break;
            case vt_string:
                if (m_has_content)
                    m_cur_sheet.sheet->set_string(m_row, m_col, m_para_index);
                break;
            case vt_date:
            {
                date_time_t val = to_date_time(m_cell_attr.date_value);
                m_cur_sheet.sheet->set_date_time(
                    m_row, m_col, val.year, val.month, val.day, val.hour, val.minute, val.second);
                break;
            }
            default:
                ;
        }
    }
}

void ods_content_xml_context::end_spreadsheet()
{
    ods_session_data& ods_data =
        static_cast<ods_session_data&>(*get_session_context().mp_data);

    ss::iface::import_reference_resolver* resolver =
        mp_factory->get_reference_resolver(ss::formula_ref_context_t::named_expression_base);

    if (resolver)
    {
        for (const ods_session_data::named_exp& data : ods_data.m_named_exps)
        {
            if (get_config().debug)
            {
                cout << "named expression: name='" << data.name
                     << "'; base='" << data.base
                     << "'; expression='" << data.expression
                     << "'; sheet-scope=" << data.scope
                     << endl;
            }

            ss::src_address_t base = resolver->resolve_address(data.base.data(), data.base.size());

            ss::iface::import_named_expression* named_exp = nullptr;

            if (data.scope >= 0)
            {
                // sheet local scope
                assert(data.scope < ss::sheet_t(m_tables.size()));
                named_exp = m_tables[data.scope]->get_named_expression();
            }
            else
            {
                // global scope.
                named_exp = mp_factory->get_named_expression();
            }

            if (named_exp)
            {
                named_exp->set_base_position(base);
                switch (data.type)
                {
                    case ods_session_data::ne_range:
                        named_exp->set_named_range(
                            data.name.data(), data.name.size(), data.expression.data(), data.expression.size());
                        break;
                    case ods_session_data::ne_expression:
                        named_exp->set_named_expression(
                            data.name.data(), data.name.size(), data.expression.data(), data.expression.size());
                        break;
                    default:
                        ;
                }
                named_exp->commit();
            }
        }
    }

    // Push all formula cells.  Formula cells needs to be processed after all
    // the sheet data have been imported, else 3D reference would fail to
    // resolve.
    for (ods_session_data::formula& data : ods_data.m_formulas)
    {
        if (data.sheet < 0 || static_cast<size_t>(data.sheet) >= m_tables.size())
            // Invalid sheet index.
            continue;

        spreadsheet::iface::import_sheet* sheet = m_tables[data.sheet];
        if (sheet)
        {
            spreadsheet::iface::import_formula* formula = sheet->get_formula();
            if (formula)
            {
                formula->set_position(data.row, data.column);
                formula->set_formula(data.grammar, data.exp.data(), data.exp.size());

                switch (data.result.type)
                {
                    case ods_session_data::rt_numeric:
                        formula->set_result_value(data.result.numeric_value);
                        break;
                    case ods_session_data::rt_string:
                    case ods_session_data::rt_error:
                    case ods_session_data::rt_none:
                    default:
                        ;
                }

                formula->commit();
            }
        }
    }

    // Clear the formula buffer.
    ods_data.m_formulas.clear();
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
