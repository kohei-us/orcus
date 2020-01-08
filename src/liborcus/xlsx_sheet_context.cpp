/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xlsx_sheet_context.hpp"
#include "xlsx_autofilter_context.hpp"
#include "xlsx_conditional_format_context.hpp"
#include "xlsx_session_data.hpp"
#include "xlsx_types.hpp"
#include "ooxml_global.hpp"
#include "ooxml_schemas.hpp"
#include "ooxml_token_constants.hpp"
#include "ooxml_namespace_types.hpp"
#include "xml_context_global.hpp"
#include "orcus/exception.hpp"
#include "orcus/global.hpp"
#include "orcus/spreadsheet/import_interface.hpp"
#include "orcus/spreadsheet/import_interface_view.hpp"
#include "orcus/measurement.hpp"

#include <mdds/sorted_string_map.hpp>

#include <algorithm>
#include <sstream>
#include <vector>

using namespace std;

namespace orcus {

namespace {

class col_attr_parser : public std::unary_function<xml_token_attr_t, void>
{
    long m_min;
    long m_max;
    double m_width;
    bool m_custom_width;
    bool m_contains_width;
    bool m_hidden;
public:
    col_attr_parser() : m_min(0), m_max(0), m_width(0.0), m_custom_width(false),
                        m_contains_width(false), m_hidden(false) {}

    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.value.empty())
            return;

        const char* p = attr.value.get();
        const char* p_end = p + attr.value.size();

        switch (attr.name)
        {
            case XML_min:
                m_min = to_long(p, p_end);
            break;
            case XML_max:
                m_max = to_long(p, p_end);
            break;
            case XML_width:
                m_width = to_double(p, p_end);
                m_contains_width = true;
            break;
            case XML_customWidth:
                m_custom_width = to_long(p, p_end);
            break;
            case XML_hidden:
                m_hidden = to_long(p, p_end);
            break;
            default:
                ;
        }
    }

    long get_min() const { return m_min; }
    long get_max() const { return m_max; }
    double get_width() const { return m_width; }
    bool is_custom_width() const { return m_custom_width; }
    bool contains_width() const { return m_contains_width; }
    bool is_hidden() const { return m_hidden; }
};

class row_attr_parser : public std::unary_function<xml_token_attr_t, void>
{
    spreadsheet::row_t m_row;
    length_t m_height;
    bool m_contains_address;
    bool m_hidden;
public:
    row_attr_parser() : m_row(0), m_contains_address(false), m_hidden(false) {}
    void operator() (const xml_token_attr_t& attr)
    {
        switch (attr.name)
        {
            case XML_r:
            {
                // row index
                m_row = static_cast<spreadsheet::row_t>(to_long(attr.value));
                if (!m_row)
                    throw xml_structure_error("row number can never be zero!");

                m_row -= 1; // from 1-based to 0-based.
                m_contains_address = true;
            }
            break;
            case XML_ht:
            {
                m_height.value = to_double(attr.value);
                m_height.unit = length_unit_t::point;
            }
            break;
            case XML_hidden:
                m_hidden = to_long(attr.value) != 0;
            break;
            default:
                ;
        }
    }

    spreadsheet::row_t get_row() const { return m_row; }

    length_t get_height() const { return m_height; }

    bool contains_address() const { return m_contains_address; }

    bool is_hidden() const { return m_hidden; }
};

class cell_attr_parser : public std::unary_function<xml_token_attr_t, void>
{
    struct address
    {
        spreadsheet::row_t row;
        spreadsheet::col_t col;
        address(spreadsheet::row_t _row, spreadsheet::col_t _col) : row(_row), col(_col) {}
    };

    xlsx_cell_t m_type;
    address m_address;
    size_t m_xf;
    bool m_contains_address;

public:
    cell_attr_parser() :
        m_type(xlsx_ct_numeric),
        m_address(0,0),
        m_xf(0),
        m_contains_address(false) {}

    void operator() (const xml_token_attr_t& attr)
    {
        switch (attr.name)
        {
            case XML_r:
                // cell address in A1 notation.
                m_address = to_cell_address(attr.value);
                m_contains_address = true;
            break;
            case XML_t:
                // cell type
                m_type = to_xlsx_cell_type(attr.value);
            break;
            case XML_s:
                // cell style
                m_xf = to_long(attr.value);
            break;
        }
    }

    xlsx_cell_t get_cell_type() const { return m_type; }

    spreadsheet::row_t get_row() const { return m_address.row; }
    spreadsheet::col_t get_col() const { return m_address.col; }
    size_t get_xf() const { return m_xf; }
    bool contains_address() const { return m_contains_address; }

private:

    address to_cell_address(const pstring& s) const
    {
        spreadsheet::row_t row = 0;
        spreadsheet::col_t col = 0;
        const char* p = s.get();
        size_t n = s.size();
        for (size_t i = 0; i < n; ++i, ++p)
        {
            char c = *p;
            if ('A' <= c && c <= 'Z')
            {
                col *= 26;
                col += static_cast<spreadsheet::col_t>(c - 'A' + 1);
            }
            else if ('0' <= c && c <= '9')
            {
                row *= 10;
                row += static_cast<spreadsheet::row_t>(c - '0');
            }
            else
            {
                std::ostringstream os;
                os << "invalid cell address: " << s;
                throw xml_structure_error(os.str());
            }
        }

        if (!row || !col)
        {
            std::ostringstream os;
            os << "invalid cell address: " << s;
            throw xml_structure_error(os.str());
        }

        return address(row-1, col-1); // switch from 1-based to 0-based.
    }
};

namespace sheet_pane {

typedef mdds::sorted_string_map<spreadsheet::sheet_pane_t> map_type;

// Keys must be sorted.
const std::vector<map_type::entry> entries = {
    { ORCUS_ASCII("bottomLeft"),  spreadsheet::sheet_pane_t::bottom_left  },
    { ORCUS_ASCII("bottomRight"), spreadsheet::sheet_pane_t::bottom_right },
    { ORCUS_ASCII("topLeft"),     spreadsheet::sheet_pane_t::top_left     },
    { ORCUS_ASCII("topRight"),    spreadsheet::sheet_pane_t::top_right    },
};

const map_type& get()
{
    static map_type mt(entries.data(), entries.size(), spreadsheet::sheet_pane_t::unspecified);
    return mt;
}

}

namespace pane_state {

typedef mdds::sorted_string_map<spreadsheet::pane_state_t> map_type;

// Keys must be sorted.
const std::vector<map_type::entry> entries = {
    { ORCUS_ASCII("frozen"),      spreadsheet::pane_state_t::frozen       },
    { ORCUS_ASCII("frozenSplit"), spreadsheet::pane_state_t::frozen_split },
    { ORCUS_ASCII("split"),       spreadsheet::pane_state_t::split        },
};

const map_type& get()
{
    static map_type mt(entries.data(), entries.size(), spreadsheet::pane_state_t::unspecified);
    return mt;
}

}

namespace formula_type {

typedef mdds::sorted_string_map<spreadsheet::formula_t> map_type;

// Keys must be sorted.
const std::vector<map_type::entry> entries = {
    { ORCUS_ASCII("array"),     spreadsheet::formula_t::array      },
    { ORCUS_ASCII("dataTable"), spreadsheet::formula_t::data_table },
    { ORCUS_ASCII("normal"),    spreadsheet::formula_t::normal     },
    { ORCUS_ASCII("shared"),    spreadsheet::formula_t::shared     },
};

const map_type& get()
{
    static map_type mt(entries.data(), entries.size(), spreadsheet::formula_t::unknown);
    return mt;
}

}

}

xlsx_sheet_context::formula::formula() :
    type(spreadsheet::formula_t::unknown),
    str(),
    data_table_ref1(),
    data_table_ref2(),
    shared_id(-1),
    data_table_2d(false),
    data_table_row_based(false),
    data_table_ref1_deleted(false),
    data_table_ref2_deleted(false)
{
    ref.first.column = -1;
    ref.first.row = -1;
    ref.last = ref.first;
}

void xlsx_sheet_context::formula::reset()
{
    *this = formula();
}

xlsx_sheet_context::xlsx_sheet_context(
    session_context& session_cxt, const tokens& tokens, spreadsheet::sheet_t sheet_id,
    spreadsheet::iface::import_reference_resolver& resolver,
    spreadsheet::iface::import_sheet& sheet) :
    xml_context_base(session_cxt, tokens),
    m_resolver(resolver),
    m_sheet(sheet),
    m_sheet_id(sheet_id),
    m_cur_row(-1),
    m_cur_col(-1),
    m_cur_cell_type(xlsx_ct_numeric),
    m_cur_cell_xf(0)
{
}

xlsx_sheet_context::~xlsx_sheet_context()
{
}

bool xlsx_sheet_context::can_handle_element(xmlns_id_t ns, xml_token_t name) const
{
    if (ns == NS_ooxml_xlsx && name == XML_autoFilter)
        return false;
    else if (ns == NS_ooxml_xlsx && name == XML_conditionalFormatting && m_sheet.get_conditional_format())
        return false;

    return true;
}

xml_context_base* xlsx_sheet_context::create_child_context(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_ooxml_xlsx && name == XML_autoFilter)
    {
        mp_child.reset(new xlsx_autofilter_context(get_session_context(), get_tokens(), m_resolver));
        mp_child->transfer_common(*this);
        return mp_child.get();
    }
    else if (ns == NS_ooxml_xlsx && name == XML_conditionalFormatting && m_sheet.get_conditional_format())
    {
        mp_child.reset(new xlsx_conditional_format_context(get_session_context(), get_tokens(),
                    *m_sheet.get_conditional_format()));
        mp_child->transfer_common(*this);
        return mp_child.get();
    }
    return nullptr;
}

void xlsx_sheet_context::end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child)
{
    if (!child)
        return;

    if (ns == NS_ooxml_xlsx && name == XML_autoFilter)
    {
        spreadsheet::iface::import_auto_filter* af = m_sheet.get_auto_filter();
        if (!af)
            return;

        const xlsx_autofilter_context& cxt = static_cast<const xlsx_autofilter_context&>(*child);
        cxt.push_to_model(*af);
    }
}

void xlsx_sheet_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_attrs_t& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);

    switch (name)
    {
        case XML_worksheet:
        {
            if (get_config().debug)
                print_attrs(get_tokens(), attrs);
        }
        break;
        case XML_cols:
            xml_element_expected(parent, NS_ooxml_xlsx, XML_worksheet);
        break;
        case XML_col:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_cols);
            col_attr_parser func;
            func = for_each(attrs.begin(), attrs.end(), func);

            spreadsheet::iface::import_sheet_properties* sheet_props = m_sheet.get_sheet_properties();
            if (sheet_props)
            {
                double width = func.get_width();
                bool contains_width = func.contains_width();
                bool hidden = func.is_hidden();
                for (spreadsheet::col_t col = func.get_min(); col <= func.get_max(); ++col)
                {
                    if (contains_width)
                        sheet_props->set_column_width(col-1, width, length_unit_t::xlsx_column_digit);
                    sheet_props->set_column_hidden(col-1, hidden);
                }
            }
        }
        break;
        case XML_dimension:
            xml_element_expected(parent, NS_ooxml_xlsx, XML_worksheet);
        break;
        case XML_mergeCells:
            xml_element_expected(parent, NS_ooxml_xlsx, XML_worksheet);
        break;
        case XML_mergeCell:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_mergeCells);

            spreadsheet::iface::import_sheet_properties* sheet_props = m_sheet.get_sheet_properties();
            if (sheet_props)
            {
                // ref contains merged range in A1 reference style.
                pstring ref = for_each(
                    attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_ref)).get_value();

                spreadsheet::range_t range = m_resolver.resolve_range(ref.get(), ref.size());
                sheet_props->set_merge_cell_range(range);
            }
        }
        break;
        case XML_pageMargins:
        {
            xml_elem_stack_t elems;
            elems.push_back(xml_token_pair_t(NS_ooxml_xlsx, XML_worksheet));
            elems.push_back(xml_token_pair_t(NS_ooxml_xlsx, XML_customSheetView));
            xml_element_expected(parent, elems);
        }
        break;
        case XML_sheetViews:
            xml_element_expected(parent, NS_ooxml_xlsx, XML_worksheet);
        break;
        case XML_sheetView:
            start_element_sheet_view(parent, attrs);
            break;
        case XML_selection:
            start_element_selection(parent, attrs);
            break;
        case XML_pane:
            start_element_pane(parent, attrs);
            break;
        case XML_sheetData:
            xml_element_expected(parent, NS_ooxml_xlsx, XML_worksheet);
        break;
        case XML_sheetFormatPr:
            xml_element_expected(parent, NS_ooxml_xlsx, XML_worksheet);
        break;
        case XML_row:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_sheetData);
            row_attr_parser func;
            func = for_each(attrs.begin(), attrs.end(), func);
            if (func.contains_address())
                m_cur_row = func.get_row();
            else
                ++m_cur_row;

            m_cur_col = -1;

            spreadsheet::iface::import_sheet_properties* sheet_props = m_sheet.get_sheet_properties();
            if (sheet_props)
            {
                length_t ht = func.get_height();
                if (ht.unit != length_unit_t::unknown)
                    sheet_props->set_row_height(m_cur_row, ht.value, ht.unit);

                bool hidden = func.is_hidden();
                sheet_props->set_row_hidden(m_cur_row, hidden);
            }
        }
        break;
        case XML_c:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_row);
            cell_attr_parser func;
            func = for_each(attrs.begin(), attrs.end(), func);

            if (func.contains_address())
            {
                if (m_cur_row != func.get_row())
                    throw xml_structure_error("row numbers differ!");

                m_cur_col = func.get_col();
            }
            else
            {
                ++m_cur_col;
            }

            m_cur_cell_type = func.get_cell_type();
            m_cur_cell_xf = func.get_xf();
        }
        break;
        case XML_f:
            start_element_formula(parent, attrs);
            break;
        case XML_v:
            xml_element_expected(parent, NS_ooxml_xlsx, XML_c);
        break;
        case XML_tableParts:
            xml_element_expected(parent, NS_ooxml_xlsx, XML_worksheet);
        break;
        case XML_tablePart:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_tableParts);

            // The rid string must be pooled to the session context's string
            // pool as it is used long after thet sheet context is deleted.
            single_attr_getter func(get_session_context().m_string_pool, NS_ooxml_r, XML_id);
            pstring rid = for_each(attrs.begin(), attrs.end(), func).get_value();

            unique_ptr<xlsx_rel_table_info> p(new xlsx_rel_table_info);
            p->sheet_interface = &m_sheet;
            m_rel_extras.data.insert(
                opc_rel_extras_t::map_type::value_type(rid, std::move(p)));
        }
        break;
        default:
            warn_unhandled();
    }

}

bool xlsx_sheet_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    switch (name)
    {
        case XML_c:
            end_element_cell();
        break;
        case XML_f:
            m_cur_formula.str = m_cur_str;
        break;
        case XML_v:
            m_cur_value = m_cur_str;
        break;
        default:
            ;
    }

    m_cur_str.clear();
    return pop_stack(ns, name);
}

void xlsx_sheet_context::characters(const pstring& str, bool transient)
{
    m_cur_str = intern_in_context(str, transient);
}

void xlsx_sheet_context::start_element_formula(const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    xml_element_expected(parent, {
        { NS_ooxml_xlsx, XML_c },
        { NS_mso_x14, XML_cfRule },
    });

    m_cur_formula.reset();

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns != NS_ooxml_xlsx)
            continue;

        switch (attr.name)
        {
            case XML_t:
                m_cur_formula.type = formula_type::get().find(attr.value.data(), attr.value.size());
                break;
            case XML_ref:
                m_cur_formula.ref = m_resolver.resolve_range(attr.value.data(), attr.value.size());
                break;
            case XML_si:
                m_cur_formula.shared_id = to_long(attr.value);
                break;
            case XML_dt2D:
                m_cur_formula.data_table_2d = to_long(attr.value) != 0;
                break;
            case XML_dtr:
                m_cur_formula.data_table_row_based = to_long(attr.value) != 0;
                break;
            case XML_del1:
                m_cur_formula.data_table_ref1_deleted = to_long(attr.value) != 0;
                break;
            case XML_del2:
                m_cur_formula.data_table_ref2_deleted = to_long(attr.value) != 0;
                break;
            case XML_r1:
                m_cur_formula.data_table_ref1 = intern_in_context(attr);
                break;
            case XML_r2:
                m_cur_formula.data_table_ref2 = intern_in_context(attr);
                break;
            default:
                ;
        }
    }
}

void xlsx_sheet_context::start_element_sheet_view(
    const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    xml_element_expected(parent, NS_ooxml_xlsx, XML_sheetViews);

    spreadsheet::iface::import_sheet_view* view = m_sheet.get_sheet_view();
    if (!view)
        return;

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns == NS_ooxml_xlsx)
        {
            switch (attr.name)
            {
                case XML_tabSelected:
                {
                    bool v = to_bool(attr.value);
                    if (v)
                        // This sheet is active.
                        view->set_sheet_active();
                    break;
                }
                default:
                    ;
            }
        }
    }
}

void xlsx_sheet_context::start_element_selection(
    const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    xml_elem_stack_t elems;
    elems.emplace_back(NS_ooxml_xlsx, XML_sheetView);
    elems.emplace_back(NS_ooxml_xlsx, XML_customSheetView);
    xml_element_expected(parent, elems);

    spreadsheet::iface::import_sheet_view* view = m_sheet.get_sheet_view();
    if (!view)
        return;

    // example: <selection pane="topRight" activeCell="H2" sqref="H2:L2"/>

    spreadsheet::sheet_pane_t pane = spreadsheet::sheet_pane_t::unspecified;
    spreadsheet::range_t range;

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns == NS_ooxml_xlsx)
        {
            switch (attr.name)
            {
                case XML_pane:
                {
                    pane = sheet_pane::get().find(
                        attr.value.data(), attr.value.size());
                    break;
                }
                case XML_activeCell:
                    // Single cell where the cursor is. Ignore this for now.
                    break;
                case XML_sqref:
                {
                    // Single cell address for a non-range cursor, or range
                    // address if a range selection is present.
                    range = m_resolver.resolve_range(attr.value.data(), attr.value.size());
                    break;
                }
                default:
                    ;
            }
        }
    }

    if (pane == spreadsheet::sheet_pane_t::unspecified)
        pane = spreadsheet::sheet_pane_t::top_left;

    view->set_selected_range(pane, range);
}

void xlsx_sheet_context::start_element_pane(
    const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    xml_elem_stack_t elems;
    elems.emplace_back(NS_ooxml_xlsx, XML_sheetView);
    elems.emplace_back(NS_ooxml_xlsx, XML_customSheetView);
    xml_element_expected(parent, elems);

    spreadsheet::iface::import_sheet_view* view = m_sheet.get_sheet_view();
    if (!view)
        return;

    // <pane xSplit="4" ySplit="8" topLeftCell="E9" activePane="bottomRight" state="frozen"/>

    double xsplit = 0.0, ysplit = 0.0;
    spreadsheet::address_t top_left_cell;
    spreadsheet::sheet_pane_t active_pane = spreadsheet::sheet_pane_t::unspecified;
    spreadsheet::pane_state_t pane_state = spreadsheet::pane_state_t::unspecified;

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns != NS_ooxml_xlsx)
            continue;

        switch (attr.name)
        {
            case XML_xSplit:
                xsplit = to_double(attr.value);
                break;
            case XML_ySplit:
                ysplit = to_double(attr.value);
                break;
            case XML_topLeftCell:
                top_left_cell = m_resolver.resolve_address(attr.value.data(), attr.value.size());
                break;
            case XML_activePane:
                active_pane = sheet_pane::get().find(attr.value.data(), attr.value.size());
                break;
            case XML_state:
                pane_state = pane_state::get().find(attr.value.data(), attr.value.size());
                break;
            default:
                ;
        }
    }

    if (active_pane == spreadsheet::sheet_pane_t::unspecified)
        active_pane = spreadsheet::sheet_pane_t::top_left;

    if (pane_state == spreadsheet::pane_state_t::unspecified)
        pane_state = spreadsheet::pane_state_t::split;

    switch (pane_state)
    {
        case spreadsheet::pane_state_t::frozen:
            view->set_frozen_pane(xsplit, ysplit, top_left_cell, active_pane);
            break;
        case spreadsheet::pane_state_t::split:
            view->set_split_pane(xsplit, ysplit, top_left_cell, active_pane);
            break;
        case spreadsheet::pane_state_t::frozen_split:
            if (get_config().debug)
                cout << "FIXME: frozen-split state not yet handled." << endl;
            break;
        default:
            ;
    }
}

void xlsx_sheet_context::end_element_cell()
{
    session_context& cxt = get_session_context();
    xlsx_session_data& session_data = static_cast<xlsx_session_data&>(*cxt.mp_data);

    bool array_formula_result = handle_array_formula_result();

    if (array_formula_result)
    {
        // Do nothing.
    }
    else if (!m_cur_formula.str.empty())
    {
        if (m_cur_formula.type == spreadsheet::formula_t::shared && m_cur_formula.shared_id >= 0)
        {
            // shared formula expression
            session_data.m_shared_formulas.push_back(
                orcus::make_unique<xlsx_session_data::shared_formula>(
                    m_sheet_id, m_cur_row, m_cur_col, m_cur_formula.shared_id,
                    m_cur_formula.str.str()));
        }
        else if (m_cur_formula.type == spreadsheet::formula_t::array)
        {
            // array formula expression
            session_data.m_array_formulas.push_back(
                orcus::make_unique<xlsx_session_data::array_formula>(
                    m_sheet_id, m_cur_formula.ref, m_cur_formula.str.str()));

            xlsx_session_data::array_formula& af = *session_data.m_array_formulas.back();
            push_raw_cell_result(*af.results, 0, 0);
            m_array_formula_results.push_back(std::make_pair(m_cur_formula.ref, af.results));
        }
        else
        {
            // normal (non-shared) formula expression
            session_data.m_formulas.push_back(
                orcus::make_unique<xlsx_session_data::formula>(
                    m_sheet_id, m_cur_row, m_cur_col, m_cur_formula.str.str()));
        }
    }
    else if (m_cur_formula.type == spreadsheet::formula_t::shared && m_cur_formula.shared_id >= 0)
    {
        // shared formula without formula expression
        session_data.m_shared_formulas.push_back(
            orcus::make_unique<xlsx_session_data::shared_formula>(
                m_sheet_id, m_cur_row, m_cur_col, m_cur_formula.shared_id));
    }
    else if (m_cur_formula.type == spreadsheet::formula_t::data_table)
    {
        // Import data table.
        spreadsheet::iface::import_data_table* dt = m_sheet.get_data_table();
        if (dt)
        {
            if (m_cur_formula.data_table_2d)
            {
                dt->set_type(spreadsheet::data_table_type_t::both);
                dt->set_range(m_cur_formula.ref);
                dt->set_first_reference(
                    m_cur_formula.data_table_ref1.get(), m_cur_formula.data_table_ref1.size(),
                    m_cur_formula.data_table_ref1_deleted);
                dt->set_second_reference(
                    m_cur_formula.data_table_ref2.get(), m_cur_formula.data_table_ref2.size(),
                    m_cur_formula.data_table_ref2_deleted);
            }
            else if (m_cur_formula.data_table_row_based)
            {
                dt->set_type(spreadsheet::data_table_type_t::row);
                dt->set_range(m_cur_formula.ref);
                dt->set_first_reference(
                    m_cur_formula.data_table_ref1.get(), m_cur_formula.data_table_ref1.size(),
                    m_cur_formula.data_table_ref1_deleted);
            }
            else
            {
                dt->set_type(spreadsheet::data_table_type_t::column);
                dt->set_range(m_cur_formula.ref);
                dt->set_first_reference(
                    m_cur_formula.data_table_ref1.get(), m_cur_formula.data_table_ref1.size(),
                    m_cur_formula.data_table_ref1_deleted);
            }
            dt->commit();
        }

        push_raw_cell_value();
    }
    else if (!m_cur_value.empty())
    {
        push_raw_cell_value();
    }

    if (m_cur_cell_xf)
        m_sheet.set_format(m_cur_row, m_cur_col, m_cur_cell_xf);

    // reset cell related parameters.
    m_cur_value.clear();
    m_cur_formula.reset();
    m_cur_cell_xf = 0;
    m_cur_cell_type = xlsx_ct_numeric;
}

void xlsx_sheet_context::push_raw_cell_value()
{
    if (m_cur_value.empty())
        return;

    switch (m_cur_cell_type)
    {
        case xlsx_ct_shared_string:
        {
            // string cell
            size_t str_id = to_long(m_cur_value);
            m_sheet.set_string(m_cur_row, m_cur_col, str_id);
        }
        break;
        case xlsx_ct_numeric:
        {
            // value cell
            double val = to_double(m_cur_value);
            m_sheet.set_value(m_cur_row, m_cur_col, val);
        }
        break;
        case xlsx_ct_boolean:
        {
            // boolean cell
            bool val = to_long(m_cur_value) != 0;
            m_sheet.set_bool(m_cur_row, m_cur_col, val);
        }
        break;
        default:
            warn("unhanlded cell content type");
    }
}

void xlsx_sheet_context::push_raw_cell_result(
    range_formula_results& res, size_t row_offset, size_t col_offset)
{
    if (m_cur_value.empty())
        return;

    switch (m_cur_cell_type)
    {
        case xlsx_ct_shared_string:
        {
            // string cell
            size_t str_id = to_long(m_cur_value);
            res.set(row_offset, col_offset, str_id);
            break;
        }
        case xlsx_ct_numeric:
        {
            // value cell
            double val = to_double(m_cur_value);
            res.set(row_offset, col_offset, val);
            break;
        }
        case xlsx_ct_boolean:
        {
            // boolean cell
            bool val = to_long(m_cur_value) != 0;
            res.set(row_offset, col_offset, val);
            break;
        }
        default:
            warn("unhanlded cell content type");
    }
}

bool xlsx_sheet_context::handle_array_formula_result()
{
    // See if the current cell is within an array formula range.
    auto it = m_array_formula_results.begin(), ite = m_array_formula_results.end();

    while (it != ite)
    {
        const spreadsheet::range_t& ref = it->first;

        if (ref.last.row < m_cur_row)
        {
            // If this result range lies above the current row, delete it as
            // we no longer have use for it.

            m_array_formula_results.erase(it++);
            continue;
        }

        if (m_cur_col < ref.first.column || ref.last.column < m_cur_col || m_cur_row < ref.first.row || ref.last.row < m_cur_row)
        {
            // This cell is not within this array formula range.  Move on to
            // the next one.
            ++it;
            continue;
        }

        size_t row_offset = m_cur_row - ref.first.row;
        size_t col_offset = m_cur_col - ref.first.column;
        range_formula_results& res = *it->second;
        push_raw_cell_result(res, row_offset, col_offset);

        return true;
    }

    return false;
}

pstring xlsx_sheet_context::intern_in_context(const xml_token_attr_t& attr)
{
    return intern_in_context(attr.value, attr.transient);
}

pstring xlsx_sheet_context::intern_in_context(const pstring& str, bool transient)
{
    if (transient)
        return m_pool.intern(str).first;

    return str;
}

void xlsx_sheet_context::pop_rel_extras(opc_rel_extras_t& other)
{
    m_rel_extras.swap(other);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
