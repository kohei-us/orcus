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
#include "orcus/spreadsheet/import_interface_view.hpp"
#include "orcus/measurement.hpp"

#include <mdds/sorted_string_map.hpp>

#include <iostream>

using namespace std;

namespace orcus {

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

xls_xml_data_context::string_segment_type::string_segment_type(const pstring& _str) :
    str(_str) {}

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

xls_xml_data_context::~xls_xml_data_context() {}

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
                            fmt.color = spreadsheet::to_color_rgb(
                                attr.value.data(), attr.value.size());
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

void xls_xml_data_context::reset(
    spreadsheet::iface::import_sheet* sheet,
    spreadsheet::row_t row, spreadsheet::col_t col,
    const pstring& cell_formula)
{
    m_format_stack.clear();
    m_format_stack.emplace_back(); // set default format.
    update_current_format();

    m_cell_type = ct_unknown;
    m_cell_string.clear();

    mp_cur_sheet = sheet;
    m_row = row;
    m_col = col;
    m_cell_formula = cell_formula;
    m_cell_value = std::numeric_limits<double>::quiet_NaN();
    m_cell_datetime = date_time_t();

    if (get_config().debug)
    {
        cout << "cell data: (row: " << m_row
             << "; column: " << m_col
             << "; formula: '" << m_cell_formula << "')" << endl;
    }
}

void xls_xml_data_context::start_element_data(
    const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
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
        m_cell_type = ct_unknown;
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
                break;

            if (m_cell_string.empty())
                break;

            if (m_cell_string.size() == 1 && !m_cell_string[0].formatted)
            {
                // Unformatted string.
                const pstring& s = m_cell_string.back().str;
                mp_cur_sheet->set_string(m_row, m_col, ss->append(s.data(), s.size()));
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

                    ss->append_segment(sstr.str.data(), sstr.str.size());
                }

                size_t si = ss->commit_segments();
                mp_cur_sheet->set_string(m_row, m_col, si);
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

}

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

xls_xml_context::xls_xml_context(session_context& session_cxt, const tokens& tokens, spreadsheet::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory),
    mp_cur_sheet(nullptr),
    mp_sheet_props(nullptr),
    m_cur_sheet(-1),
    m_cur_row(0), m_cur_col(0),
    m_cur_prop_col(0),
    m_cur_merge_down(0), m_cur_merge_across(0),
    m_cc_data(session_cxt, tokens, factory)
{
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
                m_cc_data.transfer_common(*this);
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

                ++m_cur_sheet;
                pstring sheet_name = for_each(attrs.begin(), attrs.end(), sheet_attr_parser()).get_name();
                mp_cur_sheet = mp_factory->append_sheet(m_cur_sheet, sheet_name.get(), sheet_name.size());
                spreadsheet::iface::import_named_expression* sheet_named_exp = nullptr;
                if (mp_cur_sheet)
                {
                    mp_sheet_props = mp_cur_sheet->get_sheet_properties();
                    sheet_named_exp = mp_cur_sheet->get_named_expression();
                }

                m_sheet_named_exps.push_back(sheet_named_exp);

                m_cur_row = 0;
                m_cur_col = 0;
                break;
            }
            case XML_Table:
                xml_element_expected(parent, NS_xls_xml_ss, XML_Worksheet);
                break;
            case XML_Row:
                start_element_row(parent, attrs);
                break;
            case XML_Cell:
                start_element_cell(parent, attrs);
                break;
            case XML_Column:
                start_element_column(parent, attrs);
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
            case XML_Borders:
                start_element_borders(parent, attrs);
                break;
            case XML_Border:
                start_element_border(parent, attrs);
                break;
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
                        case XML_Color:
                        {
                            m_current_style->font.color =
                                spreadsheet::to_color_rgb(attr.value.data(), attr.value.size());
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
                xml_element_expected(parent, NS_xls_xml_ss, XML_Style);

                for (const xml_token_attr_t& attr : attrs)
                {
                    if (attr.ns != NS_xls_xml_ss)
                        continue;

                    switch (attr.name)
                    {
                        case XML_Color:
                        {
                            m_current_style->fill.color =
                                spreadsheet::to_color_rgb(attr.value.data(), attr.value.size());
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
                xml_element_expected(parent, NS_xls_xml_ss, XML_Style);

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
                xml_element_expected(parent, NS_xls_xml_ss, XML_Worksheet);
                m_split_pane.reset();
                break;
            case XML_FreezePanes:
                xml_element_expected(parent, NS_xls_xml_x, XML_WorksheetOptions);
                // TODO : check if this is correct.
                m_split_pane.pane_state = spreadsheet::pane_state_t::frozen_split;
                break;
            case XML_FrozenNoSplit:
                xml_element_expected(parent, NS_xls_xml_x, XML_WorksheetOptions);
                m_split_pane.pane_state = spreadsheet::pane_state_t::frozen;
                break;
            case XML_ActivePane:
                xml_element_expected(parent, NS_xls_xml_x, XML_WorksheetOptions);
                m_split_pane.active_pane = spreadsheet::sheet_pane_t::unspecified;
                break;
            case XML_SplitHorizontal:
                xml_element_expected(parent, NS_xls_xml_x, XML_WorksheetOptions);
                m_split_pane.split_horizontal = 0.0;
                break;
            case XML_SplitVertical:
                xml_element_expected(parent, NS_xls_xml_x, XML_WorksheetOptions);
                m_split_pane.split_vertical = 0.0;
                break;
            case XML_TopRowBottomPane:
                xml_element_expected(parent, NS_xls_xml_x, XML_WorksheetOptions);
                m_split_pane.top_row_bottom_pane = 0;
                break;
            case XML_LeftColumnRightPane:
                xml_element_expected(parent, NS_xls_xml_x, XML_WorksheetOptions);
                m_split_pane.left_col_right_pane = 0;
                break;
            case XML_Panes:
                xml_element_expected(parent, NS_xls_xml_x, XML_WorksheetOptions);
                break;
            case XML_Pane:
                xml_element_expected(parent, NS_xls_xml_x, XML_Panes);
                m_cursor_selection.reset();
                break;
            case XML_Number:
                xml_element_expected(parent, NS_xls_xml_x, XML_Pane);
                break;
            case XML_ActiveCol:
                xml_element_expected(parent, NS_xls_xml_x, XML_Pane);
                break;
            case XML_ActiveRow:
                xml_element_expected(parent, NS_xls_xml_x, XML_Pane);
                break;
            case XML_RangeSelection:
                xml_element_expected(parent, NS_xls_xml_x, XML_Pane);
                break;
            case XML_Selected:
            {
                xml_element_expected(parent, NS_xls_xml_x, XML_WorksheetOptions);
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
            case XML_Row:
                end_element_row();
                break;
            case XML_Cell:
                end_element_cell();
                break;
            case XML_Column:
                end_element_column();
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

void xls_xml_context::characters(const pstring& str, bool /*transient*/)
{
    if (str.empty())
        return;

    const xml_token_pair_t& ce = get_current_element();

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
                    mp_factory->get_reference_resolver();

                if (resolver)
                    m_cursor_selection.range = resolver->resolve_range(str.data(), str.size());

                break;
            }
            default:
                ;
        }
    }
}

void xls_xml_context::start_element_borders(const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    xml_element_expected(parent, NS_xls_xml_ss, XML_Style);
    m_current_style->borders.clear();
}

void xls_xml_context::start_element_border(const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    xml_element_expected(parent, NS_xls_xml_ss, XML_Borders);

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
                color = spreadsheet::to_color_rgb(attr.value.data(), attr.value.size());
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

void xls_xml_context::start_element_cell(const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    xml_element_expected(parent, NS_xls_xml_ss, XML_Row);

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
            case XML_ArrayRange:
            {
                spreadsheet::iface::import_reference_resolver* resolver = mp_factory->get_reference_resolver();
                if (resolver)
                    m_cur_array_range = resolver->resolve_range(attr.value.data(), attr.value.size());

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
        m_cur_col = col_index - 1;
    }
}

void xls_xml_context::start_element_column(const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    xml_element_expected(parent, NS_xls_xml_ss, XML_Table);

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

void xls_xml_context::start_element_row(const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    xml_element_expected(parent, NS_xls_xml_ss, XML_Table);
    m_cur_col = 0;
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

void xls_xml_context::end_element_borders()
{
}

void xls_xml_context::end_element_border()
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
        mp_cur_sheet->set_formula(
            m_cur_row, m_cur_col, spreadsheet::formula_grammar_t::xls_xml,
            m_cur_cell_formula.data(), m_cur_cell_formula.size());
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

        // TODO : handle text indent level.

        size_t xf_id = styles->commit_cell_xf();

        m_style_map.insert({style->id, xf_id});
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
