/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xlsx_sheet_context.hpp"
#include "xlsx_session_data.hpp"
#include "xlsx_types.hpp"
#include "ooxml_global.hpp"
#include "ooxml_schemas.hpp"
#include "ooxml_token_constants.hpp"
#include "ooxml_namespace_types.hpp"
#include "xml_context_global.hpp"
#include "orcus/exception.hpp"
#include "orcus/spreadsheet/import_interface.hpp"
#include "orcus/spreadsheet/import_interface_view.hpp"
#include "orcus/measurement.hpp"

#include <mdds/sorted_string_map.hpp>

#include <algorithm>
#include <sstream>
#include <vector>
#include <optional>

namespace ss = orcus::spreadsheet;

namespace orcus {

namespace {

namespace sheet_pane {

using map_type = mdds::sorted_string_map<ss::sheet_pane_t, mdds::string_view_map_entry>;

// Keys must be sorted.
constexpr map_type::entry entries[] = {
    { "bottomLeft",  ss::sheet_pane_t::bottom_left  },
    { "bottomRight", ss::sheet_pane_t::bottom_right },
    { "topLeft",     ss::sheet_pane_t::top_left     },
    { "topRight",    ss::sheet_pane_t::top_right    },
};

const map_type& get()
{
    static const map_type mt(entries, std::size(entries), ss::sheet_pane_t::unspecified);
    return mt;
}

}

namespace pane_state {

using map_type = mdds::sorted_string_map<ss::pane_state_t, mdds::string_view_map_entry>;

// Keys must be sorted.
constexpr map_type::entry entries[] = {
    { "frozen",      ss::pane_state_t::frozen       },
    { "frozenSplit", ss::pane_state_t::frozen_split },
    { "split",       ss::pane_state_t::split        },
};

const map_type& get()
{
    static const map_type mt(entries, std::size(entries), ss::pane_state_t::unspecified);
    return mt;
}

} // namespace pane_state

namespace formula_type {

using map_type = mdds::sorted_string_map<ss::formula_t, mdds::string_view_map_entry>;

// Keys must be sorted.
constexpr map_type::entry entries[] = {
    { "array",     ss::formula_t::array      },
    { "dataTable", ss::formula_t::data_table },
    { "normal",    ss::formula_t::normal     },
    { "shared",    ss::formula_t::shared     },
};

const map_type& get()
{
    static const map_type mt(entries, std::size(entries), ss::formula_t::unknown);
    return mt;
}

} // namespace formula_type

} // anonymous namespace

xlsx_sheet_context::formula::formula() :
    type(ss::formula_t::unknown),
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
    session_context& session_cxt, const tokens& tokens, ss::sheet_t sheet_id,
    ss::iface::import_reference_resolver& resolver,
    ss::iface::import_sheet& sheet) :
    xml_context_base(session_cxt, tokens),
    m_resolver(resolver),
    m_sheet(sheet),
    m_sheet_id(sheet_id),
    m_cur_row(-1),
    m_cur_col(-1),
    m_cur_cell_type(xlsx_ct_numeric),
    m_cur_cell_xf(0),
    m_cxt_autofilter(session_cxt, tokens, m_resolver),
    m_cxt_cond_format(session_cxt, tokens, m_sheet.get_conditional_format())
{
    register_child(&m_cxt_autofilter);
    register_child(&m_cxt_cond_format);

    init_ooxml_context(*this);
}

xlsx_sheet_context::~xlsx_sheet_context()
{
}

xml_context_base* xlsx_sheet_context::create_child_context(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_ooxml_xlsx && name == XML_autoFilter)
    {
        m_cxt_autofilter.reset();
        return &m_cxt_autofilter;
    }
    else if (ns == NS_ooxml_xlsx && name == XML_conditionalFormatting)
    {
        m_cxt_cond_format.reset();
        return &m_cxt_cond_format;
    }
    return nullptr;
}

void xlsx_sheet_context::end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child)
{
    if (!child)
        return;

    if (ns == NS_ooxml_xlsx && name == XML_autoFilter)
    {
        ss::iface::import_auto_filter* af = m_sheet.get_auto_filter();
        if (!af)
            return;

        const xlsx_autofilter_context& cxt = static_cast<const xlsx_autofilter_context&>(*child);
        cxt.push_to_model(*af);
    }
}

void xlsx_sheet_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);

    if (ns == NS_ooxml_xlsx)
    {
        switch (name)
        {
            case XML_worksheet:
            {
                if (get_config().debug)
                    print_attrs(get_tokens(), attrs);
                break;
            }
            case XML_cols:
                xml_element_expected(parent, NS_ooxml_xlsx, XML_worksheet);
                break;
            case XML_col:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_cols);
                start_element_col(attrs);
                break;
            }
            case XML_dimension:
                xml_element_expected(parent, NS_ooxml_xlsx, XML_worksheet);
                break;
            case XML_mergeCells:
                xml_element_expected(parent, NS_ooxml_xlsx, XML_worksheet);
                break;
            case XML_mergeCell:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_mergeCells);

                ss::iface::import_sheet_properties* sheet_props = m_sheet.get_sheet_properties();
                if (sheet_props)
                {
                    // ref contains merged range in A1 reference style.
                    std::string_view ref = for_each(
                        attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_ref)).get_value();

                    ss::src_range_t range = m_resolver.resolve_range(ref);
                    sheet_props->set_merge_cell_range(to_rc_range(range));
                }
                break;
            }
            case XML_pageMargins:
            {
                xml_elem_stack_t elems;
                elems.push_back(xml_token_pair_t(NS_ooxml_xlsx, XML_worksheet));
                elems.push_back(xml_token_pair_t(NS_ooxml_xlsx, XML_customSheetView));
                xml_element_expected(parent, elems);
                break;
            }
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
                start_element_row(attrs);
                break;
            }
            case XML_c:
            {
                start_element_cell(parent, attrs);
                break;
            }
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
                single_attr_getter func(get_session_context().spool, NS_ooxml_r, XML_id);
                std::string_view rid = for_each(attrs.begin(), attrs.end(), func).get_value();

                std::unique_ptr<xlsx_rel_table_info> p(new xlsx_rel_table_info);
                p->sheet_interface = &m_sheet;
                m_rel_extras.data.insert(
                    opc_rel_extras_t::map_type::value_type(rid, std::move(p)));
                break;
            }
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool xlsx_sheet_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_ooxml_xlsx)
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
    }

    m_cur_str = std::string_view{};
    return pop_stack(ns, name);
}

void xlsx_sheet_context::characters(std::string_view str, bool transient)
{
    m_cur_str = intern_in_context(str, transient);
}

void xlsx_sheet_context::start_element_formula(const xml_token_pair_t& parent, const xml_token_attrs_t& attrs)
{
    const xml_elem_set_t expected = {
        { NS_ooxml_xlsx, XML_c },
        { NS_mso_x14, XML_cfRule },
    };
    xml_element_expected(parent, expected);

    m_cur_formula.reset();

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns && attr.ns != NS_ooxml_xlsx)
            continue;

        switch (attr.name)
        {
            case XML_t:
                m_cur_formula.type = formula_type::get().find(attr.value);
                break;
            case XML_ref:
                m_cur_formula.ref = to_rc_range(m_resolver.resolve_range(attr.value));
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
    const xml_token_pair_t& parent, const xml_token_attrs_t& attrs)
{
    xml_element_expected(parent, NS_ooxml_xlsx, XML_sheetViews);

    ss::iface::import_sheet_view* view = m_sheet.get_sheet_view();
    if (!view)
        return;

    for (const xml_token_attr_t& attr : attrs)
    {
        if (!attr.ns || attr.ns == NS_ooxml_xlsx)
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
    const xml_token_pair_t& parent, const xml_token_attrs_t& attrs)
{
    xml_elem_stack_t elems;
    elems.emplace_back(NS_ooxml_xlsx, XML_sheetView);
    elems.emplace_back(NS_ooxml_xlsx, XML_customSheetView);
    xml_element_expected(parent, elems);

    ss::iface::import_sheet_view* view = m_sheet.get_sheet_view();
    if (!view)
        return;

    // example: <selection pane="topRight" activeCell="H2" sqref="H2:L2"/>

    ss::sheet_pane_t pane = ss::sheet_pane_t::unspecified;
    ss::range_t range;
    range.first.column = -1;
    range.first.row = -1;
    range.last = range.first;

    for (const xml_token_attr_t& attr : attrs)
    {
        if (!attr.ns || attr.ns == NS_ooxml_xlsx)
        {
            switch (attr.name)
            {
                case XML_pane:
                {
                    pane = sheet_pane::get().find(attr.value);
                    break;
                }
                case XML_activeCell:
                    // Single cell where the cursor is. Ignore this for now.
                    break;
                case XML_sqref:
                {
                    // Single cell address for a non-range cursor, or range
                    // address if a range selection is present.
                    range = to_rc_range(m_resolver.resolve_range(attr.value));
                    break;
                }
                default:
                    ;
            }
        }
    }

    if (pane == ss::sheet_pane_t::unspecified)
        pane = ss::sheet_pane_t::top_left;

    view->set_selected_range(pane, range);
}

void xlsx_sheet_context::start_element_pane(
    const xml_token_pair_t& parent, const xml_token_attrs_t& attrs)
{
    xml_elem_stack_t elems;
    elems.emplace_back(NS_ooxml_xlsx, XML_sheetView);
    elems.emplace_back(NS_ooxml_xlsx, XML_customSheetView);
    xml_element_expected(parent, elems);

    ss::iface::import_sheet_view* view = m_sheet.get_sheet_view();
    if (!view)
        return;

    // <pane xSplit="4" ySplit="8" topLeftCell="E9" activePane="bottomRight" state="frozen"/>

    double xsplit = 0.0, ysplit = 0.0;
    ss::address_t top_left_cell;
    ss::sheet_pane_t active_pane = ss::sheet_pane_t::unspecified;
    ss::pane_state_t pane_state = ss::pane_state_t::unspecified;

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns && attr.ns != NS_ooxml_xlsx)
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
                top_left_cell = to_rc_address(m_resolver.resolve_address(attr.value));
                break;
            case XML_activePane:
                active_pane = sheet_pane::get().find(attr.value);
                break;
            case XML_state:
                pane_state = pane_state::get().find(attr.value);
                break;
            default:
                ;
        }
    }

    if (active_pane == ss::sheet_pane_t::unspecified)
        active_pane = ss::sheet_pane_t::top_left;

    if (pane_state == ss::pane_state_t::unspecified)
        pane_state = ss::pane_state_t::split;

    switch (pane_state)
    {
        case ss::pane_state_t::frozen:
            view->set_frozen_pane(xsplit, ysplit, top_left_cell, active_pane);
            break;
        case ss::pane_state_t::split:
            view->set_split_pane(xsplit, ysplit, top_left_cell, active_pane);
            break;
        case ss::pane_state_t::frozen_split:
            warn("FIXME: frozen-split state not yet handled.");
            break;
        default:
            ;
    }
}

void xlsx_sheet_context::start_element_cell(const xml_token_pair_t& parent, const xml_token_attrs_t& attrs)
{
    xlsx_cell_t cell_type = xlsx_ct_numeric;
    ss::address_t address;
    address.column = 0;
    address.row = 0;
    size_t xf = 0;
    bool contains_address = false;

    xml_element_expected(parent, NS_ooxml_xlsx, XML_row);

    for (const xml_token_attr_t& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_r:
                // cell address in A1 notation.
                address = to_rc_address(
                    m_resolver.resolve_address(attr.value));

                contains_address = true;
                break;
            case XML_t:
                // cell type
                cell_type = to_xlsx_cell_type(attr.value);
                break;
            case XML_s:
                // cell style
                xf = to_long(attr.value);
                break;
        }
    }

    if (contains_address)
    {
        if (m_cur_row != address.row)
        {
            std::ostringstream os;
            os << "row numbers differ! (current=" << m_cur_row << ")";
            throw xml_structure_error(os.str());
        }

        m_cur_col = address.column;
    }
    else
    {
        ++m_cur_col;
    }

    m_cur_cell_type = cell_type;
    m_cur_cell_xf = xf;
}

void xlsx_sheet_context::start_element_col(const xml_token_attrs_t& attrs)
{
    long col_min = 0; // 1-based
    long col_max = 0; // 1-based
    bool col_hidden = false;
    std::optional<double> col_width;
    std::optional<std::size_t> xfid;

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.value.empty())
            continue;

        switch (attr.name)
        {
            case XML_min:
                col_min = to_long(attr.value);
                break;
            case XML_max:
                col_max = to_long(attr.value);
                break;
            case XML_width:
                col_width = to_double(attr.value);
                break;
            case XML_hidden:
                col_hidden = to_long(attr.value);
                break;
            case XML_style:
                xfid = to_long(attr.value);
                break;
        }
    }

    if (!col_min || !col_max || col_min > col_max)
    {
        std::ostringstream os;
        os << "column element has invalid column indices: (min=" << col_min << "; max=" << col_max << ")";
        warn(os.str());
        return;
    }

    if (xfid)
        m_sheet.set_column_format(col_min - 1, col_max - col_min + 1, *xfid);

    ss::iface::import_sheet_properties* sheet_props = m_sheet.get_sheet_properties();
    if (sheet_props)
    {
        if (col_width)
            sheet_props->set_column_width(
                col_min - 1, col_max - col_min + 1, *col_width, length_unit_t::xlsx_column_digit);

        sheet_props->set_column_hidden(col_min - 1, col_max - col_min + 1, col_hidden);
    }
}

void xlsx_sheet_context::start_element_row(const xml_token_attrs_t& attrs)
{
    std::optional<ss::row_t> row;
    length_t height;
    bool hidden = false;
    bool custom_format = false;
    std::optional<std::size_t> xfid;

    for (const xml_token_attr_t& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_r:
            {
                // row index
                long this_row = to_long(attr.value);
                if (!this_row)
                    throw xml_structure_error("row number can never be zero!");

                this_row -= 1; // from 1-based to 0-based.
                row = this_row;
                break;
            }
            case XML_ht:
            {
                height.value = to_double(attr.value);
                height.unit = length_unit_t::point;
                break;
            }
            case XML_hidden:
                hidden = to_long(attr.value) != 0;
                break;
            case XML_s:
                xfid = to_long(attr.value);
                break;
            case XML_customFormat:
                custom_format = to_bool(attr.value);
                break;
        }
    }

    if (row)
        m_cur_row = *row;
    else
        ++m_cur_row;

    m_cur_col = -1;

    if (custom_format && xfid)
        // The specs say we only honor this style id only when the custom format is set.
        m_sheet.set_row_format(m_cur_row, *xfid);

    ss::iface::import_sheet_properties* sheet_props = m_sheet.get_sheet_properties();
    if (sheet_props)
    {
        if (height.unit != length_unit_t::unknown)
            sheet_props->set_row_height(m_cur_row, height.value, height.unit);

        sheet_props->set_row_hidden(m_cur_row, hidden);
    }
}

void xlsx_sheet_context::end_element_cell()
{
    session_context& cxt = get_session_context();
    auto& session_data = cxt.get_data<xlsx_session_data>();

    bool array_formula_result = handle_array_formula_result(session_data);

    if (array_formula_result)
    {
        // Do nothing.
    }
    else if (!m_cur_formula.str.empty())
    {
        if (m_cur_formula.type == ss::formula_t::shared && m_cur_formula.shared_id >= 0)
        {
            // shared formula expression
            session_data.m_shared_formulas.push_back(
                std::make_unique<xlsx_session_data::shared_formula>(
                    m_sheet_id, m_cur_row, m_cur_col, m_cur_formula.shared_id,
                    m_cur_formula.str
                )
            );

            xlsx_session_data::shared_formula& f = *session_data.m_shared_formulas.back();
            push_raw_cell_result(f.result, session_data);
        }
        else if (m_cur_formula.type == ss::formula_t::array)
        {
            // array formula expression
            session_data.m_array_formulas.push_back(
                std::make_unique<xlsx_session_data::array_formula>(
                    m_sheet_id, m_cur_formula.ref, m_cur_formula.str
                )
            );

            xlsx_session_data::array_formula& af = *session_data.m_array_formulas.back();
            push_raw_cell_result(*af.results, 0, 0, session_data);
            m_array_formula_results.push_back(std::make_pair(m_cur_formula.ref, af.results));
        }
        else
        {
            // normal (non-shared) formula expression
            session_data.m_formulas.push_back(
                std::make_unique<xlsx_session_data::formula>(
                    m_sheet_id, m_cur_row, m_cur_col, m_cur_formula.str
                )
            );

            xlsx_session_data::formula& f = *session_data.m_formulas.back();
            push_raw_cell_result(f.result, session_data);
        }
    }
    else if (m_cur_formula.type == ss::formula_t::shared && m_cur_formula.shared_id >= 0)
    {
        // shared formula without formula expression
        session_data.m_shared_formulas.push_back(
            std::make_unique<xlsx_session_data::shared_formula>(
                m_sheet_id, m_cur_row, m_cur_col, m_cur_formula.shared_id));

        xlsx_session_data::shared_formula& f = *session_data.m_shared_formulas.back();
        push_raw_cell_result(f.result, session_data);
    }
    else if (m_cur_formula.type == ss::formula_t::data_table)
    {
        // Import data table.
        ss::iface::import_data_table* dt = m_sheet.get_data_table();
        if (dt)
        {
            if (m_cur_formula.data_table_2d)
            {
                dt->set_type(ss::data_table_type_t::both);
                dt->set_range(m_cur_formula.ref);
                dt->set_first_reference(
                    m_cur_formula.data_table_ref1,
                    m_cur_formula.data_table_ref1_deleted);
                dt->set_second_reference(
                    m_cur_formula.data_table_ref2,
                    m_cur_formula.data_table_ref2_deleted);
            }
            else if (m_cur_formula.data_table_row_based)
            {
                dt->set_type(ss::data_table_type_t::row);
                dt->set_range(m_cur_formula.ref);
                dt->set_first_reference(
                    m_cur_formula.data_table_ref1,
                    m_cur_formula.data_table_ref1_deleted);
            }
            else
            {
                dt->set_type(ss::data_table_type_t::column);
                dt->set_range(m_cur_formula.ref);
                dt->set_first_reference(
                    m_cur_formula.data_table_ref1,
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
    m_cur_value = std::string_view{};
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
    range_formula_results& res, size_t row_offset, size_t col_offset, xlsx_session_data& /*session_data*/) const
{
    if (m_cur_value.empty())
        return;

    switch (m_cur_cell_type)
    {
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

void xlsx_sheet_context::push_raw_cell_result(formula_result& res, xlsx_session_data& session_data) const
{
    switch (m_cur_cell_type)
    {
        case xlsx_ct_numeric:
            res.type = formula_result::result_type::numeric;
            res.value_numeric = to_double(m_cur_value);
            break;
        case xlsx_ct_formula_string:
        {
            std::string_view interned = session_data.m_formula_result_strings.intern(m_cur_value).first;
            res.type = formula_result::result_type::string;
            res.value_string.p = interned.data();
            res.value_string.n = interned.size();
            break;
        }
        default:
        {
            std::ostringstream os;
            os << "unhandled cached formula result (type=" << m_cur_cell_type << ")";
            warn(os.str().data());
        }
    }
}

bool xlsx_sheet_context::handle_array_formula_result(xlsx_session_data& session_data)
{
    // See if the current cell is within an array formula range.
    auto it = m_array_formula_results.begin(), ite = m_array_formula_results.end();

    while (it != ite)
    {
        const ss::range_t& ref = it->first;

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
        push_raw_cell_result(res, row_offset, col_offset, session_data);

        return true;
    }

    return false;
}

std::string_view xlsx_sheet_context::intern_in_context(const xml_token_attr_t& attr)
{
    return intern_in_context(attr.value, attr.transient);
}

std::string_view xlsx_sheet_context::intern_in_context(const std::string_view& str, bool transient)
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
