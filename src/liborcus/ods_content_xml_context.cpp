/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ods_content_xml_context.hpp"
#include "odf_token_constants.hpp"
#include "odf_namespace_types.hpp"
#include "session_context.hpp"
#include "ods_session_data.hpp"
#include "impl_utils.hpp"

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/spreadsheet/import_interface_styles.hpp>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstring>

#include <mdds/sorted_string_map.hpp>

using namespace std;
namespace ss = orcus::spreadsheet;

namespace orcus {

namespace {

namespace cell_value {

using map_type = mdds::sorted_string_map<ods_content_xml_context::cell_value_type, mdds::string_view_map_entry>;

// Keys must be sorted.
map_type::entry entries[] = {
    { "date",   ods_content_xml_context::vt_date },
    { "float",  ods_content_xml_context::vt_float },
    { "string", ods_content_xml_context::vt_string }
};

const map_type& get()
{
    static map_type cv_map(
        entries, std::size(entries), ods_content_xml_context::vt_unknown);

    return cv_map;
}

} // namespace cell_value

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

    auto& ods_data = cxt.get_data<ods_session_data>();

    if (!name.empty() && !expression.empty() && !base.empty())
        ods_data.named_exps.emplace_back(name, expression, base, name_type, scope);
}

} // anonymous namespace

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
    m_child_styles(session_cxt, tokens, mp_factory->get_styles()),
    m_child_para(session_cxt, tokens, factory->get_shared_strings(), m_styles),
    m_child_dde_links(session_cxt, tokens)
{
    register_child(&m_child_styles);
    register_child(&m_child_para);
    register_child(&m_child_dde_links);

    spreadsheet::iface::import_global_settings* gs = mp_factory->get_global_settings();
    if (gs)
    {
        // Set the default null date to 1899-12-30 per specification (19.614).
        gs->set_origin_date(1899, 12, 30);
    }
}

ods_content_xml_context::~ods_content_xml_context() = default;

xml_context_base* ods_content_xml_context::create_child_context(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_odf_text && name == XML_p)
    {
        m_child_para.reset();
        return &m_child_para;
    }

    if (ns == NS_odf_office && name == XML_automatic_styles)
    {
        m_child_styles.reset();
        return &m_child_styles;
    }

    if (ns == NS_odf_table && name == XML_dde_links)
    {
        m_child_dde_links.reset();
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
        auto new_styles = m_child_styles.pop_styles();
        merge(m_styles, new_styles);
        assert(new_styles.empty());

        if (get_config().debug)
            dump_state(m_styles, std::cout);

        spreadsheet::iface::import_styles* xstyles = mp_factory->get_styles();
        if (xstyles)
        {
            for (const auto& [style_name, style_value] : m_styles)
            {
                if (style_value->family == style_family_table_cell)
                {
                    // TODO: Actually we need a boolean flag to see if it is an automatic style or a real style
                    //  currently we have no way to set a real style to a cell anyway
                    const auto& cell = std::get<odf_style::cell>(style_value->data);
                    m_cell_format_map.insert(name2id_type::value_type(style_name, cell.xf));
                }
            }
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

void ods_content_xml_context::start_null_date(const xml_attrs_t& attrs)
{
    spreadsheet::iface::import_global_settings* gs = mp_factory->get_global_settings();
    if (!gs)
        // Global settings not available. No point going further.
        return;

    std::string_view null_date;

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns == NS_odf_table && attr.name == XML_date_value)
            null_date = attr.value;
    }

    date_time_t val = date_time_t::from_chars(null_date);

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
        std::string_view name;

        for (const xml_token_attr_t& attr : attrs)
        {
            if (attr.ns == NS_odf_table && attr.name == XML_name)
                name = attr.value;
        }

        m_tables.push_back(mp_factory->append_sheet(m_tables.size(), name));
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

    std::string_view style_name;

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns == NS_odf_table)
        {
            if (attr.name == XML_style_name)
                style_name = attr.value;
        }
    }

    auto it = m_styles.find(style_name);
    if (it == m_styles.end())
        // Style by this name not found.
        return;

    const odf_style& style = *it->second;

    sheet_props->set_column_width(
        m_col,
        std::get<odf_style::column>(style.data).width.value,
        std::get<odf_style::column>(style.data).width.unit);
}

void ods_content_xml_context::end_column()
{
    ++m_col;
}

void ods_content_xml_context::start_row(const xml_attrs_t& attrs)
{
    m_col = 0;
    m_row_attr = row_attr();

    std::string_view style_name;

    for (const xml_token_attr_t& attr : attrs)
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

    if (get_config().debug)
        std::cout << "row: (style='" << style_name << "')" << std::endl;

    if (!m_cur_sheet.sheet)
        return;

    // Pass row properties to the interface.
    spreadsheet::iface::import_sheet_properties* sheet_props =
        m_cur_sheet.sheet->get_sheet_properties();

    if (sheet_props)
    {
        if (auto it = m_styles.find(style_name); it != m_styles.end())
        {
            const odf_style& style = *it->second;
            if (style.family == style_family_table_row)
            {
                const auto& data = std::get<odf_style::row>(style.data);
                if (data.height_set)
                    sheet_props->set_row_height(m_row, data.height.value, data.height.unit);
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

    /**
     * Process the prefixed raw formula string stored as attribute value.
     */
    auto process_formula = [](std::string_view str) -> std::string_view
    {
        if (str.empty())
            return {};

        // Formula is prefixed with formula type, followed by a ':'
        // then the actual formula content.

        // First, detect prefix if any.  Only try up to the first 5 characters.
        const char* p0 = str.data();
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

        std::string_view prefix, formula;
        if (prefix_size)
        {
            prefix = std::string_view(p0, prefix_size);
            const char* p = p0;
            p += prefix_size + 1;
            end = p0 + str.size();
            formula = std::string_view(p, end - p);
        }
        else
        {
            // TODO : Handle cases where a formula doesn't have a prefix.
        }

        // Formula needs to begin with '='.
        if (formula.empty() || formula[0] != '=')
            return {};

        // Remove the '='.
        formula = std::string_view(formula.data()+1, formula.size()-1);

        if (prefix == "of")
        {
            // ODF formula.  No action needed.
        }

        return formula;
    };

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.value.empty())
            continue;

        if (attr.ns == NS_odf_table)
        {
            switch (attr.name)
            {
                case XML_style_name:
                {
                    m_cell_attr.style_name = intern(attr);
                    break;
                }
                case XML_number_columns_repeated:
                    m_cell_attr.number_columns_repeated = to_long(attr.value);
                    break;
                case XML_formula:
                    m_cell_attr.formula = process_formula(attr.value);
                    m_cell_attr.formula = intern(m_cell_attr.formula);
                    break;
                default:
                    ;
            }
        }

        if (attr.ns == NS_odf_office)
        {
            switch (attr.name)
            {
                case XML_value:
                {
                    const char* end = attr.value.data() + attr.value.size();
                    char* endptr;
                    double val = strtod(attr.value.data(), &endptr);
                    if (endptr == end)
                        m_cell_attr.value = val;
                    break;
                }
                case XML_value_type:
                    m_cell_attr.type = cell_value::get().find(attr.value.data(), attr.value.size());
                    break;
                case XML_date_value:
                    m_cell_attr.date_value = attr.value;
                    break;
                default:
                    ;
            }
        }
    }
}

void ods_content_xml_context::end_cell()
{
    push_cell_format();
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

void ods_content_xml_context::push_cell_format()
{
    if (!m_cur_sheet.sheet)
        return;

    ss::iface::import_styles* xstyles = mp_factory->get_styles();
    if (!xstyles)
        return;

    {
        auto it = m_cell_format_map.find(m_cell_attr.style_name);
        if (it != m_cell_format_map.end())
        {
            m_cur_sheet.sheet->set_format(m_row, m_col, it->second);
            // style key found and direct cell format set.
            return;
        }
    }

    const ods_session_data& ods_data = get_session_context().get_data<ods_session_data>();

    {
        auto it = ods_data.styles_map.find(m_cell_attr.style_name);
        if (it == ods_data.styles_map.end())
            // style key not found and it's not a named style.
            return;

        const odf_style& style = *it->second;
        if (style.family != style_family_table_cell)
            // it's a named style but not a cell style
            return;

        // It references a named style. Create a direct cell style that
        // references this named style, and set that as the cell format since we
        // can't reference a named style directly from a cell.
        const auto& celldata = std::get<odf_style::cell>(style.data);

        ss::iface::import_xf* xf = xstyles->start_xf(ss::xf_category_t::cell);
        ENSURE_INTERFACE(xf, import_xf);
        xf->set_style_xf(celldata.xf);
        std::size_t xfid = xf->commit();
        m_cell_format_map.insert({m_cell_attr.style_name, xfid});
        m_cur_sheet.sheet->set_format(m_row, m_col, xfid);
    }
}

void ods_content_xml_context::push_cell_value()
{
    assert(m_cur_sheet.index >= 0); // this is expected to be called only within a valid sheet scope.

    bool has_formula = !m_cell_attr.formula.empty();
    if (has_formula)
    {
        // Store formula cell data for later processing.
        auto& ods_data = get_session_context().get_data<ods_session_data>();
        ods_data.formulas.emplace_back(
            m_cur_sheet.index, m_row, m_col, m_cell_attr.formula_grammar, m_cell_attr.formula);

        ods_session_data::formula& formula_data = ods_data.formulas.back();

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
                date_time_t val = date_time_t::from_chars(m_cell_attr.date_value);
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
    auto& ods_data = get_session_context().get_data<ods_session_data>();

    ss::iface::import_reference_resolver* resolver =
        mp_factory->get_reference_resolver(ss::formula_ref_context_t::named_expression_base);

    if (resolver)
    {
        for (const ods_session_data::named_exp& data : ods_data.named_exps)
        {
            if (get_config().debug)
            {
                cout << "named expression: name='" << data.name
                     << "'; base='" << data.base
                     << "'; expression='" << data.expression
                     << "'; sheet-scope=" << data.scope
                     << endl;
            }

            ss::src_address_t base = resolver->resolve_address(data.base);

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
                        named_exp->set_named_range(data.name, data.expression);
                        break;
                    case ods_session_data::ne_expression:
                        named_exp->set_named_expression(data.name, data.expression);
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
    for (ods_session_data::formula& data : ods_data.formulas)
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
                formula->set_formula(data.grammar, data.exp);

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
    ods_data.formulas.clear();
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
