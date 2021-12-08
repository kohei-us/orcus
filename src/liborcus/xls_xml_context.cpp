/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xls_xml_context.hpp"
#include "xls_xml_namespace_types.hpp"
#include "xls_xml_token_constants.hpp"
#include "spreadsheet_iface_util.hpp"

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/spreadsheet/import_interface_view.hpp>
#include <orcus/measurement.hpp>

#include <mdds/sorted_string_map.hpp>

#include <iostream>
#include <limits>

using namespace std;
namespace ss = orcus::spreadsheet;

namespace orcus {

namespace {

spreadsheet::color_rgb_t to_rgb(std::string_view s)
{
    if (!s.empty() && s[0] == '#')
        return spreadsheet::to_color_rgb(s);
    else
    {
        // This may be a color name.  Lower-case it before sending it to the
        // function.
        std::string s_lower(s.size(), '\0');
        const char* p = s.data();
        std::transform(p, p + s.size(), s_lower.begin(),
            [](char c) -> char
            {
                if ('A' <= c && c <= 'Z')
                    c += 'a' - 'A';
                return c;
            }
        );

        return spreadsheet::to_color_rgb_from_name(s_lower);
    }
}

}

void xls_xml_data_context::format_type::merge(const format_type& fmt)
{
    if (fmt.bold)
        bold = true;
    if (fmt.italic)
        italic = true;

    if (fmt.color.red)
        color.red = fmt.color.red;
    if (fmt.color.green)
        color.green = fmt.color.green;
    if (fmt.color.blue)
        color.blue = fmt.color.blue;
}

bool xls_xml_data_context::format_type::formatted() const
{
    if (bold || italic)
        return true;

    if (color.red || color.green || color.blue)
        return true;

    return false;
}

xls_xml_data_context::string_segment_type::string_segment_type(std::string_view _str) :
    str(_str) {}

xls_xml_data_context::xls_xml_data_context(
    session_context& session_cxt, const tokens& tokens, xls_xml_context& parent_cxt) :
    xml_context_base(session_cxt, tokens),
    m_parent_cxt(parent_cxt),
    m_cell_type(ct_unknown),
    m_cell_value(std::numeric_limits<double>::quiet_NaN())
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_xls_xml_ss, XML_Data }, // root element
        { NS_xls_xml_html, XML_B, NS_xls_xml_html, XML_Font },
        { NS_xls_xml_html, XML_I, NS_xls_xml_html, XML_Font },
        { NS_xls_xml_ss, XML_Data, NS_xls_xml_html, XML_B },
        { NS_xls_xml_ss, XML_Data, NS_xls_xml_html, XML_Font },
        { NS_xls_xml_ss, XML_Data, NS_xls_xml_html, XML_I },
    };

    init_element_validator(rules, std::size(rules));
}

xls_xml_data_context::~xls_xml_data_context() {}

xml_context_base* xls_xml_data_context::create_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/)
{
    return nullptr;
}

void xls_xml_data_context::end_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/, xml_context_base* /*child*/)
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
    else if (ns == NS_xls_xml_html)
    {
        switch (name)
        {
            case XML_B:
                m_format_stack.emplace_back();
                m_format_stack.back().bold = true;
                update_current_format();
                break;
            case XML_I:
                m_format_stack.emplace_back();
                m_format_stack.back().italic = true;
                update_current_format();
                break;
            case XML_Font:
            {
                m_format_stack.emplace_back();
                format_type& fmt = m_format_stack.back();

                for (const xml_token_attr_t& attr : attrs)
                {
                    if (ns != NS_xls_xml_html)
                        continue;

                    switch (attr.name)
                    {
                        case XML_Color:
                            fmt.color = to_rgb(attr.value);
                            break;
                        default:
                            ;
                    }
                }

                // TODO : pick up the color.
                update_current_format();
                break;
            }
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

void xls_xml_data_context::characters(std::string_view str, bool transient)
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
                m_cell_string.emplace_back(intern(str));
            else
                m_cell_string.emplace_back(str);

            if (m_current_format.formatted())
            {
                // Apply the current format to this string segment.
                string_segment_type& ss = m_cell_string.back();
                ss.format = m_current_format;
                ss.formatted = true;
            }

            break;
        }
        case ct_number:
        {
            m_cell_value = to_double(str);
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
    else if (ns == NS_xls_xml_html)
    {
        switch (name)
        {
            case XML_B:
            case XML_I:
            case XML_Font:
                assert(!m_format_stack.empty());
                m_format_stack.pop_back();
                update_current_format();
                break;
            default:
                ;
        }
    }

    return pop_stack(ns, name);
}

void xls_xml_data_context::reset()
{
    m_format_stack.clear();
    m_format_stack.emplace_back(); // set default format.
    update_current_format();

    m_cell_type = ct_unknown;
    m_cell_string.clear();

    m_cell_value = std::numeric_limits<double>::quiet_NaN();
    m_cell_datetime = date_time_t();
}

void xls_xml_data_context::start_element_data(
    const xml_token_pair_t& /*parent*/, const xml_attrs_t& attrs)
{
    m_cell_type = ct_unknown;
    m_cell_string.clear();
    m_cell_datetime = date_time_t();

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns != NS_xls_xml_ss)
            continue;

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
                break;
            }
            default:
                ;
        }
    }
}

void xls_xml_data_context::end_element_data()
{
    pstring formula = m_parent_cxt.pop_and_clear_formula();

    if (!formula.empty())
    {
        if (m_parent_cxt.is_array_formula())
            store_array_formula_parent_cell(formula);
        else
            push_formula_cell(formula);
        m_cell_type = ct_unknown;
        return;
    }

    if (handle_array_formula_result())
    {
        m_cell_type = ct_unknown;
        return;
    }

    spreadsheet::iface::import_sheet* sheet = m_parent_cxt.get_import_sheet();
    spreadsheet::address_t pos = m_parent_cxt.get_current_pos();

    switch (m_cell_type)
    {
        case ct_unknown:
            break;
        case ct_number:
            sheet->set_value(pos.row, pos.column, m_cell_value);
            break;
        case ct_string:
        {
            spreadsheet::iface::import_shared_strings* ss =
                m_parent_cxt.get_import_factory()->get_shared_strings();

            if (!ss)
                break;

            if (m_cell_string.empty())
                break;

            if (m_cell_string.size() == 1 && !m_cell_string[0].formatted)
            {
                // Unformatted string.
                std::string_view s = m_cell_string.back().str;
                sheet->set_string(pos.row, pos.column, ss->append(s));
            }
            else
            {
                // Formatted string.
                for (const string_segment_type& sstr : m_cell_string)
                {
                    if (sstr.formatted)
                    {
                        ss->set_segment_bold(sstr.format.bold);
                        ss->set_segment_italic(sstr.format.italic);
                        ss->set_segment_font_color(
                            0,
                            sstr.format.color.red,
                            sstr.format.color.green,
                            sstr.format.color.blue);
                    }

                    ss->append_segment(sstr.str);
                }

                size_t si = ss->commit_segments();
                sheet->set_string(pos.row, pos.column, si);
            }

            m_cell_string.clear();

            break;
        }
        case ct_datetime:
        {
            sheet->set_date_time(
                pos.row, pos.column,
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

bool xls_xml_data_context::handle_array_formula_result()
{
    xls_xml_context::array_formulas_type& store = m_parent_cxt.get_array_formula_store();
    spreadsheet::address_t cur_pos = m_parent_cxt.get_current_pos();

    // See if the current cell is within an array formula range.
    auto it = store.begin(), ite = store.end();

    while (it != ite)
    {
        const spreadsheet::range_t& ref = it->first;
        xls_xml_context::array_formula_type& af = *it->second;

        if (ref.last.row < cur_pos.row)
        {
            // If this result range lies above the current row, push the array
            // and delete it from the list.

            spreadsheet::iface::import_sheet* sheet = m_parent_cxt.get_import_sheet();
            spreadsheet::iface::import_array_formula* array = nullptr;

            if (sheet)
                array = sheet->get_array_formula();

            if (array)
            {
                push_array_formula(
                    array, ref, af.formula, spreadsheet::formula_grammar_t::xls_xml, af.results);
            }

            store.erase(it++);
            continue;
        }

        if (cur_pos.column < ref.first.column || ref.last.column < cur_pos.column ||
            cur_pos.row < ref.first.row || ref.last.row < cur_pos.row)
        {
            // This cell is not within this array formula range.  Move on to
            // the next one.
            ++it;
            continue;
        }

        size_t row_offset = cur_pos.row - ref.first.row;
        size_t col_offset = cur_pos.column - ref.first.column;
        range_formula_results& res = af.results;
        push_array_result(res, row_offset, col_offset);

        return true;
    }

    return false;
}

void xls_xml_data_context::push_array_result(
    range_formula_results& res, size_t row_offset, size_t col_offset)
{
    switch (m_cell_type)
    {
        case ct_number:
        {
            res.set(row_offset, col_offset, m_cell_value);
            break;
        }
        case ct_unknown:
        case ct_datetime:
        case ct_string:
        default:
            if (get_config().debug)
                cout << "warning: unknown cell type '" << m_cell_type << "': value not pushed." << endl;
    }
}

void xls_xml_data_context::push_formula_cell(const pstring& formula)
{
    switch (m_cell_type)
    {
        case ct_number:
            m_parent_cxt.store_cell_formula(formula, m_cell_value);
            break;
        default:
        {
            formula_result res;
            m_parent_cxt.store_cell_formula(formula, res);
        }
    }
}

void xls_xml_data_context::store_array_formula_parent_cell(const pstring& formula)
{
    spreadsheet::address_t pos = m_parent_cxt.get_current_pos();
    spreadsheet::range_t range = m_parent_cxt.get_array_range();
    xls_xml_context::array_formulas_type& store = m_parent_cxt.get_array_formula_store();

    range += pos;

    store.push_back(
        std::make_pair(
            range,
            std::make_unique<xls_xml_context::array_formula_type>(range, formula)));

    xls_xml_context::array_formula_type& af = *store.back().second;

    switch (m_cell_type)
    {
        case ct_number:
            af.results.set(0, 0, m_cell_value);
            break;
        default:
            ;
    }
}

void xls_xml_data_context::update_current_format()
{
    // format stack should have at least one entry at any given moment.
    assert(!m_format_stack.empty());

    // Grab the bottom format.
    auto it = m_format_stack.begin();
    m_current_format = *it;
    ++it;

    // Merge in the rest of the format data.
    std::for_each(it, m_format_stack.end(),
        [&](const format_type& fmt)
        {
            m_current_format.merge(fmt);
        }
    );
}

namespace {

namespace border_dir {

typedef mdds::sorted_string_map<spreadsheet::border_direction_t> map_type;

// Keys must be sorted.
const std::vector<map_type::entry> entries =
{
    { ORCUS_ASCII("Bottom"),        spreadsheet::border_direction_t::bottom         },
    { ORCUS_ASCII("DiagonalLeft"),  spreadsheet::border_direction_t::diagonal_tl_br },
    { ORCUS_ASCII("DiagonalRight"), spreadsheet::border_direction_t::diagonal_bl_tr },
    { ORCUS_ASCII("Left"),          spreadsheet::border_direction_t::left           },
    { ORCUS_ASCII("Right"),         spreadsheet::border_direction_t::right          },
    { ORCUS_ASCII("Top"),           spreadsheet::border_direction_t::top            },
};

const map_type& get()
{
    static map_type mt(entries.data(), entries.size(), spreadsheet::border_direction_t::unknown);
    return mt;
}

}

namespace border_style {

typedef mdds::sorted_string_map<spreadsheet::border_style_t> map_type;

// Keys must be sorted.
const std::vector<map_type::entry> entries =
{
    { ORCUS_ASCII("Continuous"),   spreadsheet::border_style_t::solid          },
    { ORCUS_ASCII("Dash"),         spreadsheet::border_style_t::dashed         },
    { ORCUS_ASCII("DashDot"),      spreadsheet::border_style_t::dash_dot       },
    { ORCUS_ASCII("DashDotDot"),   spreadsheet::border_style_t::dash_dot_dot   },
    { ORCUS_ASCII("Dot"),          spreadsheet::border_style_t::dotted         },
    { ORCUS_ASCII("Double"),       spreadsheet::border_style_t::double_border  },
    { ORCUS_ASCII("SlantDashDot"), spreadsheet::border_style_t::slant_dash_dot },
};

const map_type& get()
{
    static map_type mt(entries.data(), entries.size(), spreadsheet::border_style_t::unknown);
    return mt;
}

}

namespace hor_align {

typedef mdds::sorted_string_map<spreadsheet::hor_alignment_t> map_type;

// Keys must be sorted.
const std::vector<map_type::entry> entries =
{
    { ORCUS_ASCII("Center"),      spreadsheet::hor_alignment_t::center      },
    { ORCUS_ASCII("Distributed"), spreadsheet::hor_alignment_t::distributed },
    { ORCUS_ASCII("Justify"),     spreadsheet::hor_alignment_t::justified   },
    { ORCUS_ASCII("Left"),        spreadsheet::hor_alignment_t::left        },
    { ORCUS_ASCII("Right"),       spreadsheet::hor_alignment_t::right       },
};

const map_type& get()
{
    static map_type mt(entries.data(), entries.size(), spreadsheet::hor_alignment_t::unknown);
    return mt;
}

}

namespace ver_align {

typedef mdds::sorted_string_map<spreadsheet::ver_alignment_t> map_type;

// Keys must be sorted.
const std::vector<map_type::entry> entries =
{
    { ORCUS_ASCII("Bottom"),      spreadsheet::ver_alignment_t::bottom      },
    { ORCUS_ASCII("Center"),      spreadsheet::ver_alignment_t::middle      },
    { ORCUS_ASCII("Distributed"), spreadsheet::ver_alignment_t::distributed },
    { ORCUS_ASCII("Justify"),     spreadsheet::ver_alignment_t::justified   },
    { ORCUS_ASCII("Top"),         spreadsheet::ver_alignment_t::top         },
};

const map_type& get()
{
    static map_type mt(entries.data(), entries.size(), spreadsheet::ver_alignment_t::unknown);
    return mt;
}

}

namespace num_format {

typedef mdds::sorted_string_map<pstring> map_type;

// Keys must be sorted.
const std::vector<map_type::entry> entries =
{
    { ORCUS_ASCII("Currency"), "$#,##0.00_);[Red]($#,##0.00)" },
    { ORCUS_ASCII("Euro Currency"), "[$\xe2\x82\xac-x-euro2] #,##0.00_);[Red]([$\xe2\x82\xac-x-euro2] #,##0.00)" },
    { ORCUS_ASCII("Fixed"), "0.00" },
    { ORCUS_ASCII("General Date"), "m/d/yyyy h:mm" },
    { ORCUS_ASCII("General Number"), "General" },
    { ORCUS_ASCII("Long Date"), "d-mmm-yy" },
    { ORCUS_ASCII("Long Time"), "h:mm:ss AM/PM" },
    { ORCUS_ASCII("Medium Date"), "d-mmm-yy" },
    { ORCUS_ASCII("Medium Time"), "h:mm AM/PM" },
    { ORCUS_ASCII("On/Off"), "\"On\";\"On\";\"Off\"" },
    { ORCUS_ASCII("Percent"), "0.00%" },
    { ORCUS_ASCII("Scientific"), "0.00E+00" },
    { ORCUS_ASCII("Short Date"), "m/d/yyyy" },
    { ORCUS_ASCII("Short Time"), "h:mm" },
    { ORCUS_ASCII("Standard"), "#,##0.00" },
    { ORCUS_ASCII("True/False"), "\"True\";\"True\";\"False\"" },
    { ORCUS_ASCII("Yes/No"), "\"Yes\";\"Yes\";\"No\"" },
};

const map_type& get()
{
    static map_type mt(entries.data(), entries.size(), pstring());
    return mt;
}

} // namespace num_format

} // anonymous namespace

xls_xml_context::array_formula_type::array_formula_type(
    const spreadsheet::range_t& _range, const pstring& _formula) :
    formula(_formula),
    results(_range.last.row-_range.first.row+1, _range.last.column-_range.first.column+1) {}

xls_xml_context::named_exp::named_exp(const pstring& _name, const pstring& _expression, spreadsheet::sheet_t _scope) :
    name(_name), expression(_expression), scope(_scope) {}

xls_xml_context::selection::selection() : pane(spreadsheet::sheet_pane_t::unspecified), col(-1), row(-1)
{
    range.first.column = -1;
    range.first.row = -1;
    range.last.column = -1;
    range.last.row = -1;
}

void xls_xml_context::selection::reset()
{
    pane = spreadsheet::sheet_pane_t::unspecified;
    col = 0;
    row = 0;

    range.first.column = -1;
    range.first.row = -1;
    range.last.column = -1;
    range.last.row = -1;
}

bool xls_xml_context::selection::valid_cursor() const
{
    return col >= 0 && row >= 0;
}

bool xls_xml_context::selection::valid_range() const
{
    return range.first.column >= 0 && range.first.row >= 0 && range.last.column >= 0 && range.last.row >= 0;
}

xls_xml_context::split_pane::split_pane() :
    pane_state(spreadsheet::pane_state_t::split),
    active_pane(spreadsheet::sheet_pane_t::top_left),
    split_horizontal(0.0), split_vertical(0.0),
    top_row_bottom_pane(0), left_col_right_pane(0) {}

void xls_xml_context::split_pane::reset()
{
    pane_state = spreadsheet::pane_state_t::split;
    active_pane = spreadsheet::sheet_pane_t::top_left;
    split_horizontal = 0.0;
    split_vertical = 0.0;
    top_row_bottom_pane = 0;
    left_col_right_pane = 0;
}

bool xls_xml_context::split_pane::split() const
{
    return (split_horizontal || split_vertical) && (top_row_bottom_pane || left_col_right_pane);
}

spreadsheet::address_t xls_xml_context::split_pane::get_top_left_cell() const
{
    spreadsheet::address_t pos;
    pos.column = left_col_right_pane;
    pos.row = top_row_bottom_pane;
    return pos;
}

xls_xml_context::table_properties::table_properties()
{
    reset();
}

void xls_xml_context::table_properties::reset()
{
    pos.row = 0;
    pos.column = 0;
}

xls_xml_context::xls_xml_context(session_context& session_cxt, const tokens& tokens, spreadsheet::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory),
    mp_cur_sheet(nullptr),
    mp_sheet_props(nullptr),
    m_cur_sheet(-1),
    m_cur_row(0), m_cur_col(0),
    m_cur_prop_col(0),
    m_cur_merge_down(0), m_cur_merge_across(0),
    m_cc_data(session_cxt, tokens, *this)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_xls_xml_ss, XML_Workbook }, // root element
        { NS_xls_xml_o, XML_DocumentProperties, NS_xls_xml_o, XML_Author },
        { NS_xls_xml_o, XML_DocumentProperties, NS_xls_xml_o, XML_Company },
        { NS_xls_xml_o, XML_DocumentProperties, NS_xls_xml_o, XML_Created },
        { NS_xls_xml_o, XML_DocumentProperties, NS_xls_xml_o, XML_LastAuthor },
        { NS_xls_xml_o, XML_DocumentProperties, NS_xls_xml_o, XML_LastSaved },
        { NS_xls_xml_o, XML_DocumentProperties, NS_xls_xml_o, XML_Version },
        { NS_xls_xml_o, XML_OfficeDocumentSettings, NS_xls_xml_o, XML_AllowPNG },
        { NS_xls_xml_o, XML_OfficeDocumentSettings, NS_xls_xml_o, XML_DownloadComponents },
        { NS_xls_xml_o, XML_OfficeDocumentSettings, NS_xls_xml_o, XML_LocationOfComponents },
        { NS_xls_xml_ss, XML_Borders, NS_xls_xml_ss, XML_Border },
        { NS_xls_xml_ss, XML_Cell, NS_xls_xml_ss, XML_Data },
        { NS_xls_xml_ss, XML_Cell, NS_xls_xml_ss, XML_NamedCell },
        { NS_xls_xml_ss, XML_Names, NS_xls_xml_ss, XML_NamedRange },
        { NS_xls_xml_ss, XML_Row, NS_xls_xml_ss, XML_Cell },
        { NS_xls_xml_ss, XML_Style, NS_xls_xml_ss, XML_Alignment },
        { NS_xls_xml_ss, XML_Style, NS_xls_xml_ss, XML_Borders },
        { NS_xls_xml_ss, XML_Style, NS_xls_xml_ss, XML_Font },
        { NS_xls_xml_ss, XML_Style, NS_xls_xml_ss, XML_Interior },
        { NS_xls_xml_ss, XML_Style, NS_xls_xml_ss, XML_NumberFormat },
        { NS_xls_xml_ss, XML_Style, NS_xls_xml_ss, XML_Protection },
        { NS_xls_xml_ss, XML_Styles, NS_xls_xml_ss, XML_Style },
        { NS_xls_xml_ss, XML_Table, NS_xls_xml_ss, XML_Column },
        { NS_xls_xml_ss, XML_Table, NS_xls_xml_ss, XML_Row },
        { NS_xls_xml_ss, XML_Workbook, NS_xls_xml_o, XML_DocumentProperties },
        { NS_xls_xml_ss, XML_Workbook, NS_xls_xml_o, XML_OfficeDocumentSettings },
        { NS_xls_xml_ss, XML_Workbook, NS_xls_xml_ss, XML_Names },
        { NS_xls_xml_ss, XML_Workbook, NS_xls_xml_ss, XML_Styles },
        { NS_xls_xml_ss, XML_Workbook, NS_xls_xml_ss, XML_Worksheet },
        { NS_xls_xml_ss, XML_Workbook, NS_xls_xml_x, XML_ExcelWorkbook },
        { NS_xls_xml_ss, XML_Worksheet, NS_xls_xml_ss, XML_Names },
        { NS_xls_xml_ss, XML_Worksheet, NS_xls_xml_ss, XML_Table },
        { NS_xls_xml_ss, XML_Worksheet, NS_xls_xml_x, XML_AutoFilter },
        { NS_xls_xml_ss, XML_Worksheet, NS_xls_xml_x, XML_WorksheetOptions },
        { NS_xls_xml_x, XML_AutoFilter, NS_xls_xml_x, XML_AutoFilterColumn },
        { NS_xls_xml_x, XML_AutoFilterColumn, NS_xls_xml_x, XML_AutoFilterCondition },
        { NS_xls_xml_x, XML_AutoFilterColumn, NS_xls_xml_x, XML_AutoFilterOr },
        { NS_xls_xml_x, XML_AutoFilterOr, NS_xls_xml_x, XML_AutoFilterCondition },
        { NS_xls_xml_x, XML_ExcelWorkbook, NS_xls_xml_x, XML_ActiveSheet },
        { NS_xls_xml_x, XML_ExcelWorkbook, NS_xls_xml_x, XML_FirstVisibleSheet },
        { NS_xls_xml_x, XML_ExcelWorkbook, NS_xls_xml_x, XML_ProtectStructure },
        { NS_xls_xml_x, XML_ExcelWorkbook, NS_xls_xml_x, XML_ProtectWindows },
        { NS_xls_xml_x, XML_ExcelWorkbook, NS_xls_xml_x, XML_RefModeR1C1 },
        { NS_xls_xml_x, XML_ExcelWorkbook, NS_xls_xml_x, XML_TabRatio },
        { NS_xls_xml_x, XML_ExcelWorkbook, NS_xls_xml_x, XML_WindowHeight },
        { NS_xls_xml_x, XML_ExcelWorkbook, NS_xls_xml_x, XML_WindowTopX },
        { NS_xls_xml_x, XML_ExcelWorkbook, NS_xls_xml_x, XML_WindowTopY },
        { NS_xls_xml_x, XML_ExcelWorkbook, NS_xls_xml_x, XML_WindowWidth },
        { NS_xls_xml_x, XML_PageSetup, NS_xls_xml_x, XML_Footer },
        { NS_xls_xml_x, XML_PageSetup, NS_xls_xml_x, XML_Header },
        { NS_xls_xml_x, XML_PageSetup, NS_xls_xml_x, XML_PageMargins },
        { NS_xls_xml_x, XML_Pane, NS_xls_xml_x, XML_ActiveCol },
        { NS_xls_xml_x, XML_Pane, NS_xls_xml_x, XML_ActiveRow },
        { NS_xls_xml_x, XML_Pane, NS_xls_xml_x, XML_Number },
        { NS_xls_xml_x, XML_Pane, NS_xls_xml_x, XML_RangeSelection },
        { NS_xls_xml_x, XML_Panes, NS_xls_xml_x, XML_Pane },
        { NS_xls_xml_x, XML_Print, NS_xls_xml_x, XML_HorizontalResolution },
        { NS_xls_xml_x, XML_Print, NS_xls_xml_x, XML_ValidPrinterInfo },
        { NS_xls_xml_x, XML_Print, NS_xls_xml_x, XML_VerticalResolution },
        { NS_xls_xml_x, XML_WorksheetOptions, NS_xls_xml_x, XML_ActivePane },
        { NS_xls_xml_x, XML_WorksheetOptions, NS_xls_xml_x, XML_FilterOn },
        { NS_xls_xml_x, XML_WorksheetOptions, NS_xls_xml_x, XML_FreezePanes },
        { NS_xls_xml_x, XML_WorksheetOptions, NS_xls_xml_x, XML_FrozenNoSplit },
        { NS_xls_xml_x, XML_WorksheetOptions, NS_xls_xml_x, XML_LeftColumnRightPane },
        { NS_xls_xml_x, XML_WorksheetOptions, NS_xls_xml_x, XML_PageSetup },
        { NS_xls_xml_x, XML_WorksheetOptions, NS_xls_xml_x, XML_Panes },
        { NS_xls_xml_x, XML_WorksheetOptions, NS_xls_xml_x, XML_Print },
        { NS_xls_xml_x, XML_WorksheetOptions, NS_xls_xml_x, XML_ProtectObjects },
        { NS_xls_xml_x, XML_WorksheetOptions, NS_xls_xml_x, XML_ProtectScenarios },
        { NS_xls_xml_x, XML_WorksheetOptions, NS_xls_xml_x, XML_Selected },
        { NS_xls_xml_x, XML_WorksheetOptions, NS_xls_xml_x, XML_SplitHorizontal },
        { NS_xls_xml_x, XML_WorksheetOptions, NS_xls_xml_x, XML_SplitVertical },
        { NS_xls_xml_x, XML_WorksheetOptions, NS_xls_xml_x, XML_TopRowBottomPane },
        { NS_xls_xml_x, XML_WorksheetOptions, NS_xls_xml_x, XML_TopRowVisible },
        { NS_xls_xml_x, XML_WorksheetOptions, NS_xls_xml_x, XML_Unsynced },
        { NS_xls_xml_x, XML_WorksheetOptions, NS_xls_xml_x, XML_Zoom },
    };

    init_element_validator(rules, std::size(rules));

    m_cur_array_range.first.column = -1;
    m_cur_array_range.first.row = -1;
    m_cur_array_range.last = m_cur_array_range.first;
}

xls_xml_context::~xls_xml_context()
{
}

void xls_xml_context::declaration(const xml_declaration_t& decl)
{
    spreadsheet::iface::import_global_settings* gs = mp_factory->get_global_settings();
    if (!gs)
        return;

    gs->set_character_set(decl.encoding);
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
                m_cc_data.transfer_common(*this);
                m_cc_data.reset();
                return &m_cc_data;
            }
            default:
                ;
        }
    }
    return nullptr;
}

void xls_xml_context::end_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/, xml_context_base* /*child*/)
{
}

void xls_xml_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_attrs_t& attrs)
{
    push_stack(ns, name);

    if (ns == NS_xls_xml_ss)
    {
        switch (name)
        {
            case XML_Workbook:
                // Do nothing.
                break;
            case XML_Worksheet:
            {
                start_element_worksheet(attrs);
                break;
            }
            case XML_Table:
                start_element_table(attrs);
                break;
            case XML_Row:
                start_element_row(attrs);
                break;
            case XML_Cell:
                start_element_cell(attrs);
                break;
            case XML_Column:
                start_element_column(attrs);
                break;
            case XML_Names:
                break;
            case XML_NamedRange:
            {
                pstring name_s, exp;

                for (const xml_token_attr_t& attr : attrs)
                {
                    if (attr.ns != NS_xls_xml_ss)
                        continue;

                    switch (attr.name)
                    {
                        case XML_Name:
                            name_s = intern(attr);
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

                if (!name_s.empty() && !exp.empty())
                {
                    if (m_cur_sheet >= 0)
                        m_named_exps_sheet.emplace_back(name_s, exp, m_cur_sheet);
                    else
                        m_named_exps_global.emplace_back(name_s, exp, -1);
                }

                break;
            }
            case XML_Styles:
                break;
            case XML_Style:
            {
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

                m_current_style = std::make_unique<style_type>();
                m_current_style->id = style_id;
                m_current_style->name = style_name;

                break;
            }
            case XML_Borders:
                start_element_borders(attrs);
                break;
            case XML_Border:
                start_element_border(attrs);
                break;
            case XML_NumberFormat:
                start_element_number_format(attrs);
                break;
            case XML_Font:
            {
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
                        case XML_Color:
                        {
                            m_current_style->font.color = to_rgb(attr.value);
                            break;
                        }
                        default:
                            ;
                    }
                }
                break;
            }
            case XML_Interior:
            {
                for (const xml_token_attr_t& attr : attrs)
                {
                    if (attr.ns != NS_xls_xml_ss)
                        continue;

                    switch (attr.name)
                    {
                        case XML_Color:
                        {
                            m_current_style->fill.color = to_rgb(attr.value);
                            break;
                        }
                        case XML_Pattern:
                        {
                            // TODO : support fill types other than 'solid'.
                            m_current_style->fill.solid = (attr.value == "Solid");
                            break;
                        }
                        default:
                            ;
                    }
                }
                break;
            }
            case XML_Alignment:
            {
                for (const xml_token_attr_t& attr : attrs)
                {
                    if (attr.ns != NS_xls_xml_ss)
                        continue;

                    switch (attr.name)
                    {
                        case XML_Horizontal:
                        {
                            m_current_style->text_alignment.hor =
                                hor_align::get().find(attr.value.data(), attr.value.size());
                            break;
                        }
                        case XML_Vertical:
                        {
                            m_current_style->text_alignment.ver =
                                ver_align::get().find(attr.value.data(), attr.value.size());
                            break;
                        }
                        case XML_Indent:
                        {
                            m_current_style->text_alignment.indent = to_long(attr.value);
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
    else if (ns == NS_xls_xml_x)
    {
        switch (name)
        {
            case XML_WorksheetOptions:
                m_split_pane.reset();
                break;
            case XML_FreezePanes:
                // TODO : check if this is correct.
                m_split_pane.pane_state = spreadsheet::pane_state_t::frozen_split;
                break;
            case XML_FrozenNoSplit:
                m_split_pane.pane_state = spreadsheet::pane_state_t::frozen;
                break;
            case XML_ActivePane:
                m_split_pane.active_pane = spreadsheet::sheet_pane_t::unspecified;
                break;
            case XML_SplitHorizontal:
                m_split_pane.split_horizontal = 0.0;
                break;
            case XML_SplitVertical:
                m_split_pane.split_vertical = 0.0;
                break;
            case XML_TopRowBottomPane:
                m_split_pane.top_row_bottom_pane = 0;
                break;
            case XML_LeftColumnRightPane:
                m_split_pane.left_col_right_pane = 0;
                break;
            case XML_Panes:
                break;
            case XML_Pane:
                m_cursor_selection.reset();
                break;
            case XML_Number:
                break;
            case XML_ActiveCol:
                break;
            case XML_ActiveRow:
                break;
            case XML_RangeSelection:
                break;
            case XML_Selected:
            {
                if (mp_cur_sheet)
                {
                    spreadsheet::iface::import_sheet_view* sv = mp_cur_sheet->get_sheet_view();
                    if (sv)
                        sv->set_sheet_active();
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
            case XML_Borders:
                end_element_borders();
                break;
            case XML_Border:
                end_element_border();
                break;
            case XML_NumberFormat:
                end_element_number_format();
                break;
            case XML_Row:
                end_element_row();
                break;
            case XML_Cell:
                end_element_cell();
                break;
            case XML_Column:
                end_element_column();
                break;
            case XML_Table:
                end_element_table();
                break;
            case XML_Workbook:
                end_element_workbook();
                break;
            case XML_Worksheet:
                end_element_worksheet();
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
    else if (ns == NS_xls_xml_x)
    {
        switch (name)
        {
            case XML_Pane:
                end_element_pane();
                break;
            case XML_WorksheetOptions:
                end_element_worksheet_options();
                break;
            default:
                ;
        }
    }
    return pop_stack(ns, name);
}

namespace {

spreadsheet::sheet_pane_t to_sheet_pane(long v)
{
    static const std::vector<spreadsheet::sheet_pane_t> mapping = {
        spreadsheet::sheet_pane_t::bottom_right,  // 0
        spreadsheet::sheet_pane_t::top_right,     // 1
        spreadsheet::sheet_pane_t::bottom_left,   // 2
        spreadsheet::sheet_pane_t::top_left,      // 3
    };

    if (v < 0 || size_t(v) >= mapping.size())
        return spreadsheet::sheet_pane_t::unspecified;

    return mapping[v];
}

}

void xls_xml_context::characters(std::string_view str, bool /*transient*/)
{
    if (str.empty())
        return;

    xml_token_pair_t ce = get_current_element();

    if (ce.first == NS_xls_xml_x)
    {
        switch (ce.second)
        {
            case XML_Number:
                // sheet pane position.
                // 3 | 1
                //---+---
                // 2 | 0
                m_cursor_selection.pane = to_sheet_pane(to_long(str));
                break;
            case XML_ActiveCol:
                m_cursor_selection.col = to_long(str);
                break;
            case XML_ActiveRow:
                m_cursor_selection.row = to_long(str);
                break;
            case XML_ActivePane:
                m_split_pane.active_pane = to_sheet_pane(to_long(str));
                break;
            case XML_SplitHorizontal:
                m_split_pane.split_horizontal = to_double(str);
                break;
            case XML_SplitVertical:
                m_split_pane.split_vertical = to_double(str);
                break;
            case XML_TopRowBottomPane:
                m_split_pane.top_row_bottom_pane = to_long(str);
                break;
            case XML_LeftColumnRightPane:
                m_split_pane.left_col_right_pane = to_long(str);
                break;
            case XML_RangeSelection:
            {
                spreadsheet::iface::import_reference_resolver* resolver =
                    mp_factory->get_reference_resolver(spreadsheet::formula_ref_context_t::global);

                if (resolver)
                    m_cursor_selection.range = to_rc_range(resolver->resolve_range(str));

                break;
            }
            default:
                ;
        }
    }
}

void xls_xml_context::start_element_borders(const xml_attrs_t& /*attrs*/)
{
    m_current_style->borders.clear();
}

void xls_xml_context::start_element_border(const xml_attrs_t& attrs)
{
    spreadsheet::border_direction_t dir = spreadsheet::border_direction_t::unknown;
    spreadsheet::border_style_t style = spreadsheet::border_style_t::unknown;
    spreadsheet::color_rgb_t color;
    long weight = 0;

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns != NS_xls_xml_ss)
            continue;

        switch (attr.name)
        {
            case XML_Position:
            {
                dir = border_dir::get().find(attr.value.data(), attr.value.size());
                break;
            }
            case XML_LineStyle:
            {
                style = border_style::get().find(attr.value.data(), attr.value.size());
                break;
            }
            case XML_Weight:
            {
                weight = to_long(attr.value);
                break;
            }
            case XML_Color:
            {
                color = to_rgb(attr.value);
                break;
            }
            default:
                ;
        }
    }

    if (dir == spreadsheet::border_direction_t::unknown || style == spreadsheet::border_style_t::unknown)
        return;

    m_current_style->borders.emplace_back();
    border_style_type& bs = m_current_style->borders.back();
    bs.dir = dir;
    bs.style = style;
    bs.color = color;

    switch (bs.style)
    {
        case spreadsheet::border_style_t::solid:
        {
            switch (weight)
            {
                case 0:
                    bs.style = spreadsheet::border_style_t::hair;
                    break;
                case 1:
                    bs.style = spreadsheet::border_style_t::thin;
                    break;
                case 2:
                    bs.style = spreadsheet::border_style_t::medium;
                    break;
                case 3:
                    bs.style = spreadsheet::border_style_t::thick;
                    break;
                default:
                    ;
            }
            break;
        }
        case spreadsheet::border_style_t::dashed:
            if (weight > 1)
                bs.style = spreadsheet::border_style_t::medium_dashed;
            break;
        case spreadsheet::border_style_t::dash_dot:
            if (weight > 1)
                bs.style = spreadsheet::border_style_t::medium_dash_dot;
            break;
        case spreadsheet::border_style_t::dash_dot_dot:
            if (weight > 1)
                bs.style = spreadsheet::border_style_t::medium_dash_dot_dot;
            break;
        default:
            ;
    }
}

void xls_xml_context::start_element_number_format(const xml_attrs_t& attrs)
{
    m_current_style->number_format.clear();

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns != NS_xls_xml_ss)
            continue;

        switch (attr.name)
        {
            case XML_Format:
            {
                std::string_view code = num_format::get().find(attr.value.data(), attr.value.size());
                m_current_style->number_format = code.empty() ? intern(attr) : code;
                break;
            }
            default:
                ;
        }
    }
}

void xls_xml_context::start_element_cell(const xml_attrs_t& attrs)
{
    long col_index = 0;
    pstring formula;
    m_cur_cell_style_id.clear();

    m_cur_merge_across = 0; // extra column(s) that are part of the merged cell.
    m_cur_merge_down = 0; // extra row(s) that are part of the merged cell.

    m_cur_array_range.first.column = -1;
    m_cur_array_range.first.row = -1;
    m_cur_array_range.last = m_cur_array_range.first;

    for (const xml_token_attr_t& attr : attrs)
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
                    pstring s(attr.value.data()+1, attr.value.size()-1);
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
            case XML_ArrayRange:
            {
                spreadsheet::iface::import_reference_resolver* resolver =
                    mp_factory->get_reference_resolver(spreadsheet::formula_ref_context_t::global);
                if (resolver)
                    m_cur_array_range = to_rc_range(resolver->resolve_range(attr.value));

                break;
            }
            default:
                ;
        }
    }

    if (!formula.empty())
        m_cur_cell_formula = formula;

    if (col_index > 0)
    {
        // 1-based column index. Convert it to a 0-based one.
        m_cur_col = m_table_props.pos.column + col_index - 1;
    }
}

void xls_xml_context::start_element_column(const xml_attrs_t& attrs)
{
    if (!mp_sheet_props)
        return;

    spreadsheet::col_t col_index = m_cur_prop_col;
    spreadsheet::col_t span = 0;
    double width = 0.0;
    bool hidden = false;

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
                    // Convert from 1-based to 0-based.
                    col_index = to_long(attr.value) - 1;
                    break;
                case XML_Width:
                    width = to_double(attr.value);
                    break;
                case XML_Span:
                    span = to_long(attr.value);
                    break;
                case XML_Hidden:
                    hidden = to_long(attr.value) != 0;
                default:
                    ;
            }
        }
    );

    for (; span >= 0; --span, ++col_index)
    {
        // Column widths are stored as points.
        mp_sheet_props->set_column_width(col_index, width, orcus::length_unit_t::point);
        mp_sheet_props->set_column_hidden(col_index, hidden);
    }

    m_cur_prop_col = col_index;
}

void xls_xml_context::start_element_row(const xml_attrs_t& attrs)
{
    m_cur_col = m_table_props.pos.column;
    spreadsheet::row_t row_index = -1;
    bool has_height = false;
    bool hidden = false;
    double height = 0.0;

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.value.empty())
            return;

        if (attr.ns == NS_xls_xml_ss)
        {
            switch (attr.name)
            {
                case XML_Index:
                    row_index = to_long(attr.value);
                    break;
                case XML_Height:
                    has_height = true;
                    height = to_double(attr.value);
                    break;
                case XML_Hidden:
                    hidden = to_long(attr.value) != 0;
                    break;
                default:
                    ;
            }
        }
    }

    if (row_index > 0)
    {
        // 1-based row index. Convert it to a 0-based one.
        m_cur_row = row_index - 1;
    }

    if (mp_sheet_props)
    {
        if (has_height)
            mp_sheet_props->set_row_height(m_cur_row, height, length_unit_t::point);

        if (hidden)
            mp_sheet_props->set_row_hidden(m_cur_row, true);
    }
}

void xls_xml_context::start_element_table(const xml_attrs_t& attrs)
{
    spreadsheet::row_t row_index = -1;
    spreadsheet::col_t col_index = -1;

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.value.empty())
            return;

        if (attr.ns == NS_xls_xml_ss)
        {
            switch (attr.name)
            {
                case XML_TopCell:
                    col_index = to_long(attr.value);
                    break;
                case XML_LeftCell:
                    row_index = to_long(attr.value);
                    break;
                default:
                    ;
            }
        }
    }

    // Convert 1-based indices to 0-based.

    if (row_index > 0)
    {
        m_table_props.pos.row = row_index - 1;
        m_cur_row = m_table_props.pos.row;
    }

    if (col_index > 0)
        m_table_props.pos.column = col_index - 1;
}

void xls_xml_context::start_element_worksheet(const xml_attrs_t& attrs)
{
    ++m_cur_sheet;
    pstring sheet_name;
    m_cell_formulas.emplace_back();

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns == NS_xls_xml_ss)
        {
            switch (attr.name)
            {
                case XML_Name:
                    sheet_name = attr.value;
                    break;
                default:
                    ;
            }
        }
    }

    mp_cur_sheet = mp_factory->append_sheet(m_cur_sheet, sheet_name);
    spreadsheet::iface::import_named_expression* sheet_named_exp = nullptr;
    if (mp_cur_sheet)
    {
        mp_sheet_props = mp_cur_sheet->get_sheet_properties();
        sheet_named_exp = mp_cur_sheet->get_named_expression();
    }

    m_sheet_named_exps.push_back(sheet_named_exp);

    m_cur_row = 0;
    m_cur_col = 0;

    if (get_config().debug)
        cout << "worksheet: name: '" << sheet_name << "'" << endl;
}

void xls_xml_context::end_element_borders()
{
}

void xls_xml_context::end_element_border()
{
}

void xls_xml_context::end_element_number_format()
{
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

    if (mp_cur_sheet && !m_cur_cell_style_id.empty())
    {
        auto it = m_style_map.find(m_cur_cell_style_id);
        if (it != m_style_map.end())
        {
            size_t xf_id = it->second;
            mp_cur_sheet->set_format(m_cur_row, m_cur_col, xf_id);
        }
    }

    if (mp_cur_sheet && !m_cur_cell_formula.empty())
    {
        // Likely a Cell element without a child Data element.
        store_cell_formula(m_cur_cell_formula, formula_result());
    }

    m_cur_cell_formula.clear();

    ++m_cur_col;
    if (m_cur_merge_across > 0)
        m_cur_col += m_cur_merge_across;
}

void xls_xml_context::end_element_column()
{
}

void xls_xml_context::end_element_row()
{
    ++m_cur_row;
}

void xls_xml_context::end_element_table()
{
    push_all_array_formulas();
    m_array_formulas.clear();
    m_table_props.reset();
}

void xls_xml_context::end_element_worksheet()
{
    mp_cur_sheet = nullptr;
}

void xls_xml_context::end_element_workbook()
{
    if (!mp_factory)
        return;

    spreadsheet::iface::import_named_expression* ne_global = mp_factory->get_named_expression();
    if (ne_global)
    {
        // global scope named expressions.

        for (const named_exp& ne : m_named_exps_global)
        {
            ne_global->set_named_expression(ne.name, ne.expression);
            ne_global->commit();
        }
    }

    // sheet-local named expressions follow.

    for (const named_exp& ne : m_named_exps_sheet)
    {
        spreadsheet::iface::import_named_expression* p = nullptr;
        if (ne.scope >= 0 && size_t(ne.scope) < m_sheet_named_exps.size())
            p = m_sheet_named_exps[ne.scope]; // it may be nullptr.

        if (p)
        {
            p->set_named_expression(ne.name, ne.expression);
            p->commit();
        }
    }

    // push all cell formulas
    for (size_t sheet_pos = 0; sheet_pos < m_cell_formulas.size(); ++sheet_pos)
    {
        spreadsheet::iface::import_sheet* sheet = mp_factory->get_sheet(sheet_pos);
        if (!sheet)
            continue;

        spreadsheet::iface::import_formula* xformula = sheet->get_formula();
        if (!xformula)
            continue;

        const std::deque<cell_formula_type>& store = m_cell_formulas[sheet_pos];
        for (const cell_formula_type& cf : store)
        {
            xformula->set_position(cf.pos.row, cf.pos.column);
            xformula->set_formula(ss::formula_grammar_t::xls_xml, cf.formula);

            switch (cf.result.type)
            {
                case formula_result::result_type::numeric:
                    xformula->set_result_value(cf.result.value_numeric);
                    break;
                case formula_result::result_type::string:
                case formula_result::result_type::boolean:
                case formula_result::result_type::empty:
                    ;
            }

            xformula->commit();
        }
    }
}

void xls_xml_context::end_element_styles()
{
    commit_default_style(); // Commit the default style first.
    commit_styles();
}

void xls_xml_context::end_element_pane()
{
    spreadsheet::iface::import_sheet_view* sv = mp_cur_sheet->get_sheet_view();
    if (!sv)
        return;

    if (m_cursor_selection.pane == spreadsheet::sheet_pane_t::unspecified)
        return;

    if (m_cursor_selection.valid_range())
    {
        sv->set_selected_range(m_cursor_selection.pane, m_cursor_selection.range);
    }
    else if (m_cursor_selection.valid_cursor())
    {
        spreadsheet::range_t sel;
        sel.first.column = m_cursor_selection.col;
        sel.first.row = m_cursor_selection.row;
        sel.last = sel.first;

        sv->set_selected_range(m_cursor_selection.pane, sel);
    }
}

void xls_xml_context::end_element_worksheet_options()
{
    commit_split_pane();
}

void xls_xml_context::commit_split_pane()
{
    spreadsheet::iface::import_sheet_view* sv = mp_cur_sheet->get_sheet_view();
    if (!sv)
        return;

    if (!m_split_pane.split())
        return;

    switch (m_split_pane.pane_state)
    {
        case spreadsheet::pane_state_t::split:
        {
            spreadsheet::address_t top_left_cell = m_split_pane.get_top_left_cell();

            // NB: The term "split vertical" in Excel 2003 XML refers to the
            // vertical split bar position which in this case corresponds with
            // the "horizontal split" position of the set_split_pane() call,
            // and vice versa.
            sv->set_split_pane(
                m_split_pane.split_vertical, m_split_pane.split_horizontal,
                top_left_cell, m_split_pane.active_pane);
            break;
        }
        case spreadsheet::pane_state_t::frozen:
        {
            spreadsheet::address_t top_left_cell = m_split_pane.get_top_left_cell();

            // NB: Note for the split pane above also applies here.
            spreadsheet::col_t visible_cols = m_split_pane.split_vertical;
            spreadsheet::row_t visible_rows = m_split_pane.split_horizontal;

            sv->set_frozen_pane(
                visible_cols, visible_rows,
                top_left_cell, m_split_pane.active_pane);
            break;
        }
        case spreadsheet::pane_state_t::frozen_split:
            // not handled yet.
            break;
        case spreadsheet::pane_state_t::unspecified:
        default:
            ;
    }

    m_split_pane.reset();
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
        styles->set_font_color(
            0,
            m_default_style->font.color.red,
            m_default_style->font.color.green,
            m_default_style->font.color.blue);
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
            styles->set_cell_style_name(name);
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
        styles->set_font_color(255,
            style->font.color.red,
            style->font.color.green,
            style->font.color.blue);

        size_t font_id = styles->commit_font();

        styles->set_xf_font(font_id);

        if (style->fill.solid)
        {
            // TODO : add support for fill types other than 'solid'.
            styles->set_fill_pattern_type(spreadsheet::fill_pattern_t::solid);
            styles->set_fill_fg_color(255,
                style->fill.color.red,
                style->fill.color.green,
                style->fill.color.blue);

            size_t fill_id = styles->commit_fill();
            styles->set_xf_fill(fill_id);
        }

        if (!style->borders.empty())
        {
            styles->set_border_count(style->borders.size());

            for (const border_style_type& b : style->borders)
            {
                styles->set_border_style(b.dir, b.style);
                styles->set_border_color(b.dir, 255, b.color.red, b.color.green, b.color.blue);
            }

            size_t border_id = styles->commit_border();
            styles->set_xf_border(border_id);
        }

        bool apply_alignment =
            style->text_alignment.hor != spreadsheet::hor_alignment_t::unknown ||
            style->text_alignment.ver != spreadsheet::ver_alignment_t::unknown;

        styles->set_xf_apply_alignment(apply_alignment);
        styles->set_xf_horizontal_alignment(style->text_alignment.hor);
        styles->set_xf_vertical_alignment(style->text_alignment.ver);

        if (!style->number_format.empty())
        {
            styles->set_number_format_code(style->number_format);
            size_t number_format_id = styles->commit_number_format();
            styles->set_xf_number_format(number_format_id);
        }

        // TODO : handle text indent level.

        size_t xf_id = styles->commit_cell_xf();

        m_style_map.insert({style->id, xf_id});
    }
}

void xls_xml_context::push_all_array_formulas()
{
    if (!mp_cur_sheet)
        return;

    spreadsheet::iface::import_array_formula* array = mp_cur_sheet->get_array_formula();
    if (!array)
        return;

    for (const array_formula_pair_type& pair : m_array_formulas)
    {
        const array_formula_type& af = *pair.second;
        push_array_formula(
            array, pair.first, af.formula, spreadsheet::formula_grammar_t::xls_xml, af.results);
    }
}

spreadsheet::iface::import_factory* xls_xml_context::get_import_factory()
{
    return mp_factory;
}

spreadsheet::iface::import_sheet* xls_xml_context::get_import_sheet()
{
    return mp_cur_sheet;
}

spreadsheet::address_t xls_xml_context::get_current_pos() const
{
    spreadsheet::address_t pos;
    pos.row = m_cur_row;
    pos.column = m_cur_col;
    return pos;
}

pstring xls_xml_context::pop_and_clear_formula()
{
    pstring f = m_cur_cell_formula;
    m_cur_cell_formula.clear();
    return f;
}

bool xls_xml_context::is_array_formula() const
{
    if (m_cur_array_range.first.column < 0 || m_cur_array_range.first.row < 0)
        return false;

    if (m_cur_array_range.last.column < 0 || m_cur_array_range.last.row < 0)
        return false;

    if (m_cur_array_range.first.column > m_cur_array_range.last.column ||
        m_cur_array_range.first.row > m_cur_array_range.last.row)
        return false;

    return true;
}

const spreadsheet::range_t& xls_xml_context::get_array_range() const
{
    return m_cur_array_range;
}

xls_xml_context::array_formulas_type& xls_xml_context::get_array_formula_store()
{
    return m_array_formulas;
}

void xls_xml_context::store_cell_formula(const pstring& formula, const formula_result& res)
{
    assert(m_cur_sheet < ss::sheet_t(m_cell_formulas.size()));

    cell_formula_type cf;
    cf.pos = get_current_pos();
    cf.formula = formula;
    cf.result = res;
    std::deque<cell_formula_type>& store = m_cell_formulas[m_cur_sheet];
    store.push_back(std::move(cf));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
