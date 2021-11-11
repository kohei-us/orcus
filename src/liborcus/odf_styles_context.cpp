/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "odf_styles_context.hpp"
#include "odf_namespace_types.hpp"
#include "odf_token_constants.hpp"
#include "odf_helper.hpp"
#include "odf_number_formatting_context.hpp"

#include <orcus/measurement.hpp>
#include <orcus/spreadsheet/import_interface.hpp>

#include <mdds/sorted_string_map.hpp>
#include <mdds/global.hpp>

#include <iostream>

namespace orcus {

namespace {

class invalid_odf_styles : public std::exception
{
    std::string m_msg;
public:
    invalid_odf_styles(const std::string& msg):
        std::exception(),
        m_msg(msg)
    {
    }

    virtual const char* what() const noexcept override
    {
        return m_msg.c_str();
    }
};

typedef mdds::sorted_string_map<odf_style_family> style_family_map;

style_family_map::entry style_family_entries[] =
{
    { MDDS_ASCII("graphic"), style_family_graphic },
    { MDDS_ASCII("paragraph"), style_family_paragraph },
    { MDDS_ASCII("table"), style_family_table },
    { MDDS_ASCII("table-cell"), style_family_table_cell },
    { MDDS_ASCII("table-column"), style_family_table_column },
    { MDDS_ASCII("table-row"), style_family_table_row },
    { MDDS_ASCII("text"), style_family_text }
};

odf_style_family to_style_family(const pstring& val)
{
    static style_family_map map(style_family_entries, ORCUS_N_ELEMENTS(style_family_entries), style_family_unknown);
    return map.find(val.get(), val.size());
}

class style_attr_parser
{
    pstring m_name;
    odf_style_family m_family;

    pstring m_parent_name;
public:
    style_attr_parser() :
        m_family(style_family_unknown) {}

    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.ns == NS_odf_style)
        {
            switch (attr.name)
            {
                case XML_name:
                    m_name = attr.value;
                break;
                case XML_family:
                    m_family = to_style_family(attr.value);
                break;
                case XML_parent_style_name:
                    m_parent_name = attr.value;
            }
        }
    }

    const pstring& get_name() const { return m_name; }
    odf_style_family get_family() const { return m_family; }
    const pstring& get_parent() const { return m_parent_name; }
};

class col_prop_attr_parser
{
    length_t m_width;
public:
    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.ns == NS_odf_style)
        {
            switch (attr.name)
            {
                case XML_column_width:
                    m_width = to_length(attr.value);
                break;
            }
        }
    }

    const length_t& get_width() const { return m_width; }
};

typedef mdds::sorted_string_map<spreadsheet::strikethrough_style_t> strikethrough_style_map;

strikethrough_style_map::entry strikethrough_style_entries[] =
{
    { MDDS_ASCII("dash"), spreadsheet::strikethrough_style_t::dash },
    { MDDS_ASCII("dot-dash"), spreadsheet::strikethrough_style_t::dot_dash },
    { MDDS_ASCII("dot-dot-dash"), spreadsheet::strikethrough_style_t::dot_dot_dash },
    { MDDS_ASCII("dotted"), spreadsheet::strikethrough_style_t::dotted },
    { MDDS_ASCII("long-dash"), spreadsheet::strikethrough_style_t::long_dash},
    { MDDS_ASCII("none"), spreadsheet::strikethrough_style_t::none },
    { MDDS_ASCII("solid"), spreadsheet::strikethrough_style_t::solid },
    { MDDS_ASCII("wave"), spreadsheet::strikethrough_style_t::wave },
};

class cell_prop_attr_parser
{
public:
    typedef std::map<spreadsheet::border_direction_t, odf_helper::odf_border_details> border_map_type;

    cell_prop_attr_parser():
        m_background_red(0),
        m_background_green(0),
        m_background_blue(0),
        m_background_color(false),
        m_locked(false),
        m_hidden(false),
        m_formula_hidden(false),
        m_print_content(false),
        m_cell_protection(false),
        m_ver_alignment(spreadsheet::ver_alignment_t::unknown),
        m_has_ver_alignment(false)
    {}

private:

    spreadsheet::color_elem_t m_background_red;
    spreadsheet::color_elem_t m_background_green;
    spreadsheet::color_elem_t m_background_blue;

    bool m_background_color;
    bool m_locked;
    bool m_hidden;
    bool m_formula_hidden;
    bool m_print_content;
    bool m_cell_protection;

    border_map_type m_border_style_dir_pair;

    spreadsheet::ver_alignment_t m_ver_alignment;
    bool m_has_ver_alignment;

public:

    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.ns == NS_odf_fo)
        {
            switch (attr.name)
            {
                case XML_background_color:
                    m_background_color = odf_helper::convert_fo_color(attr.value, m_background_red,
                            m_background_green, m_background_blue);
                break;

                case XML_border:
                {
                    odf_helper::odf_border_details border_details = odf_helper::extract_border_details(attr.value);
                    m_border_style_dir_pair.insert(std::make_pair(spreadsheet::border_direction_t::top, border_details));
                    m_border_style_dir_pair.insert(std::make_pair(spreadsheet::border_direction_t::bottom, border_details));
                    m_border_style_dir_pair.insert(std::make_pair(spreadsheet::border_direction_t::left, border_details));
                    m_border_style_dir_pair.insert(std::make_pair(spreadsheet::border_direction_t::right, border_details));
                }
                break;

                case XML_border_top:
                {
                    odf_helper::odf_border_details border_details = odf_helper::extract_border_details(attr.value);
                    m_border_style_dir_pair.insert(std::make_pair(spreadsheet::border_direction_t::top, border_details));
                }
                break;

                case XML_border_bottom:
                {
                    odf_helper::odf_border_details border_details = odf_helper::extract_border_details(attr.value);
                    m_border_style_dir_pair.insert(std::make_pair(spreadsheet::border_direction_t::bottom, border_details));
                }
                break;

                case XML_border_left:
                {
                    odf_helper::odf_border_details border_details = odf_helper::extract_border_details(attr.value);
                    m_border_style_dir_pair.insert(std::make_pair(spreadsheet::border_direction_t::left, border_details));
                }
                break;

                case XML_border_right:
                {
                    odf_helper::odf_border_details border_details = odf_helper::extract_border_details(attr.value);
                    m_border_style_dir_pair.insert(std::make_pair(spreadsheet::border_direction_t::right, border_details));
                }
                break;
                case XML_diagonal_bl_tr:
                {
                    odf_helper::odf_border_details border_details = odf_helper::extract_border_details(attr.value);
                    m_border_style_dir_pair.insert(std::make_pair(spreadsheet::border_direction_t::diagonal_bl_tr, border_details));
                }
                break;
                case XML_diagonal_tl_br:
                {
                    odf_helper::odf_border_details border_details = odf_helper::extract_border_details(attr.value);
                    m_border_style_dir_pair.insert(std::make_pair(spreadsheet::border_direction_t::diagonal_tl_br, border_details));
                }
                break;

                default:
                    ;
            }
        }

        else if(attr.ns == NS_odf_style)
        {
            switch(attr.name)
            {
                case XML_print_content:
                {
                    m_cell_protection = true;
                    m_print_content = attr.value == "true";
                }
                break;
                case XML_cell_protect:
                {
                    m_cell_protection = true;
                    if (attr.value == "protected")
                        m_locked = true;
                    else if (attr.value == "hidden-and-protected")
                    {
                        m_locked = true;
                        m_hidden = true;
                    }
                    else if (attr.value == "formula-hidden")
                        m_formula_hidden = true;
                    else if (attr.value == "protected formula-hidden" || attr.value == "formula-hidden protected")
                    {
                        m_formula_hidden = true;
                        m_locked = true;
                    }
                }
                break;
                case XML_vertical_align:
                    m_has_ver_alignment = odf_helper::extract_ver_alignment_style(attr.value, m_ver_alignment);
                break;
                default:
                    ;
            }
        }
    }

    bool has_background_color() const { return m_background_color; }

    void get_background_color(spreadsheet::color_elem_t& red,
            spreadsheet::color_elem_t& green, spreadsheet::color_elem_t& blue)
    {
        red = m_background_red;
        green = m_background_green;
        blue = m_background_blue;
    }

    bool has_border() const { return !m_border_style_dir_pair.empty(); }

    bool has_protection() const { return m_cell_protection; }
    bool is_locked() const { return m_locked; }
    bool is_hidden() const { return m_hidden; }
    bool is_formula_hidden() const { return m_formula_hidden; }
    bool is_print_content() const { return m_print_content; }

    const border_map_type& get_border_attrib() const
    {
        return m_border_style_dir_pair;
    }
    bool has_ver_alignment() const { return m_has_ver_alignment;}
    const spreadsheet::ver_alignment_t& get_ver_alignment() const { return m_ver_alignment;}

};

class paragraph_prop_attr_parser
{
    spreadsheet::hor_alignment_t m_hor_alignment;
    bool m_has_hor_alignment;

public:
    paragraph_prop_attr_parser():
        m_hor_alignment(spreadsheet::hor_alignment_t::unknown),
        m_has_hor_alignment(false)
    {}

    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.ns == NS_odf_fo)
        {
            switch (attr.name)
            {
                case XML_text_align:
                    m_has_hor_alignment = odf_helper::extract_hor_alignment_style(attr.value, m_hor_alignment);
                break;
                default:
                    ;
            }
        }
    }
    bool has_hor_alignment() const { return m_has_hor_alignment;}
    const spreadsheet::hor_alignment_t& get_hor_alignment() const { return m_hor_alignment;}
};

}

styles_context::styles_context(
    session_context& session_cxt, const tokens& tk, odf_styles_map_type& styles,
    spreadsheet::iface::import_styles* iface_styles) :
    xml_context_base(session_cxt, tk),
    mp_styles(iface_styles),
    m_styles(styles),
    m_automatic_styles(false)
{
    commit_default_styles();
}

bool styles_context::can_handle_element(xmlns_id_t ns, xml_token_t /*name*/) const
{
    if (ns == NS_odf_number)
        return false;

    return true;
}

xml_context_base* styles_context::create_child_context(xmlns_id_t ns, xml_token_t /*name*/)
{
    if (ns == NS_odf_number)
    {
        mp_child.reset(new number_formatting_context(get_session_context(), get_tokens(), m_styles, mp_styles));
        mp_child->transfer_common(*this);
        return mp_child.get();
    }

    return nullptr;
}

void styles_context::end_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/, xml_context_base* /*child*/)
{
}

void styles_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_attrs_t& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);
    if (ns == NS_odf_office)
    {
        switch (name)
        {
            case XML_automatic_styles:
                xml_element_expected(parent, XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN);
                m_automatic_styles = true;
            break;
            case XML_styles:
                m_automatic_styles = false;
            break;
            default:
                warn_unhandled();
        }
    }
    else if (ns == NS_odf_style)
    {
        switch (name)
        {
            case XML_style:
            {
                xml_elem_stack_t expected_parents;
                expected_parents.push_back(std::pair<xmlns_id_t, xml_token_t>(NS_odf_office, XML_automatic_styles));
                expected_parents.push_back(std::pair<xmlns_id_t, xml_token_t>(NS_odf_office, XML_styles));
                xml_element_expected(parent, expected_parents);
                style_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);
                m_current_style.reset(new odf_style(func.get_name(), func.get_family(), func.get_parent()));
            }
            break;
            case XML_table_column_properties:
            {
                xml_element_expected(parent, NS_odf_style, XML_style);
                col_prop_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);
                assert(m_current_style->family == style_family_table_column);
                m_current_style->column_data->width = func.get_width();
            }
            break;
            case XML_table_row_properties:
            {
                xml_element_expected(parent, NS_odf_style, XML_style);

                std::for_each(attrs.begin(), attrs.end(),
                    [&](const xml_token_attr_t& attr)
                    {
                        if (attr.ns == NS_odf_style)
                        {
                            switch (attr.name)
                            {
                                case XML_row_height:
                                    m_current_style->row_data->height = to_length(attr.value);
                                    m_current_style->row_data->height_set = true;
                                    break;
                            }
                        }
                    }
                );

                assert(m_current_style->family == style_family_table_row);
                break;
            }
            case XML_table_properties:
                xml_element_expected(parent, NS_odf_style, XML_style);
            break;
            case XML_paragraph_properties:
            {
                xml_element_expected(parent, NS_odf_style, XML_style);
                paragraph_prop_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);
                if (func.has_hor_alignment())
                    mp_styles->set_xf_horizontal_alignment(func.get_hor_alignment());

            }
            break;
            case XML_text_properties:
                start_text_properties(parent, attrs);
                break;
            case XML_table_cell_properties:
            {
                xml_element_expected(parent, NS_odf_style, XML_style);
                if (m_current_style->family != style_family_table_cell)
                    throw invalid_odf_styles("expected table_cell family style in cell_properties element");

                m_current_style->cell_data->automatic_style = m_automatic_styles;
                if (mp_styles)
                {
                    cell_prop_attr_parser func;
                    func = std::for_each(attrs.begin(), attrs.end(), func);
                    if (func.has_background_color())
                    {
                        spreadsheet::color_elem_t red, green, blue;
                        func.get_background_color(red, green, blue);
                        mp_styles->set_fill_pattern_type(spreadsheet::fill_pattern_t::solid);
                        mp_styles->set_fill_fg_color(255, red, green, blue);
                    }

                    size_t fill_id = mp_styles->commit_fill();

                    if (func.has_border())
                    {
                        const cell_prop_attr_parser::border_map_type& border_map = func.get_border_attrib();
                        for (cell_prop_attr_parser::border_map_type::const_iterator itr = border_map.begin(); itr != border_map.end(); ++itr)
                        {
                            mp_styles->set_border_color(itr->first, 0, itr->second.red, itr->second.green, itr->second.blue);
                            mp_styles->set_border_style(itr->first, itr->second.border_style);
                            mp_styles->set_border_width(itr->first, itr->second.border_width.value, itr->second.border_width.unit);
                        }
                    }

                    size_t border_id = mp_styles->commit_border();

                    if (func.has_protection())
                    {
                        mp_styles->set_cell_hidden(func.is_hidden());
                        mp_styles->set_cell_locked(func.is_locked());
                        mp_styles->set_cell_print_content(func.is_print_content());
                        mp_styles->set_cell_formula_hidden(func.is_formula_hidden());
                    }

                    if (func.has_ver_alignment())
                        mp_styles->set_xf_vertical_alignment(func.get_ver_alignment());

                    size_t cell_protection_id = mp_styles->commit_cell_protection();
                    switch (m_current_style->family)
                    {
                        case style_family_table_cell:
                        {
                            odf_style::cell* data = m_current_style->cell_data;
                            data->fill = fill_id;
                            data->border = border_id;
                            data->protection = cell_protection_id;
                        }
                        break;
                        default:
                            ;
                    }
                }
            }
            break;
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool styles_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_odf_style)
    {
        switch (name)
        {
            case XML_style:
            {
                if (m_current_style)
                {
                    if (mp_styles && m_current_style->family == style_family_table_cell)
                    {
                        odf_style::cell& cell = *m_current_style->cell_data;
                        mp_styles->set_xf_font(cell.font);
                        mp_styles->set_xf_fill(cell.fill);
                        mp_styles->set_xf_border(cell.border);
                        mp_styles->set_xf_protection(cell.protection);
                        size_t xf_id = 0;
                        if (cell.automatic_style)
                            xf_id = mp_styles->commit_cell_xf();
                        else
                        {
                            size_t style_xf_id = mp_styles->commit_cell_style_xf();
                            mp_styles->set_cell_style_name(m_current_style->name);
                            mp_styles->set_cell_style_xf(style_xf_id);
                            mp_styles->set_cell_style_parent_name(m_current_style->parent_name);

                            xf_id = mp_styles->commit_cell_style();
                        }
                        cell.xf = xf_id;
                    }

                    // ptr_map's first argument must be a non-const reference.
                    pstring style_name = m_current_style->name;
                    m_styles.insert(
                        odf_styles_map_type::value_type(
                            style_name, std::move(m_current_style)));
                    assert(!m_current_style);
                }
            }
            break;
        }
    }
    return pop_stack(ns, name);
}

void styles_context::characters(const pstring& /*str*/, bool /*transient*/)
{
}

void styles_context::start_text_properties(const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    static const xml_elem_stack_t expected = {
        { NS_odf_style, XML_style },
        { NS_odf_text, XML_list_level_style_number },
        { NS_odf_text, XML_list_level_style_bullet },
    };
    xml_element_expected(parent, expected);

    if (!mp_styles)
        return;

    if (parent != xml_token_pair_t(NS_odf_style, XML_style))
        // TODO : handle this properly in the future.
        return;

    std::string_view font_name;
    length_t font_size;
    bool bold = false;
    bool italic = false;
    bool has_color = false;

    spreadsheet::color_elem_t red = 0;
    spreadsheet::color_elem_t green = 0;
    spreadsheet::color_elem_t blue = 0;

    bool underline_is_text_color = true;
    bool has_underline = false;

    spreadsheet::color_elem_t underline_red = 0;
    spreadsheet::color_elem_t underline_green = 0;
    spreadsheet::color_elem_t underline_blue = 0;

    spreadsheet::underline_mode_t underline_mode = spreadsheet::underline_mode_t::continuos;
    spreadsheet::underline_width_t underline_width = spreadsheet::underline_width_t::none;
    spreadsheet::underline_t underline_style = spreadsheet::underline_t::none;
    spreadsheet::underline_type_t underline_type = spreadsheet::underline_type_t::none;

    spreadsheet::strikethrough_style_t strikethrough_style = spreadsheet::strikethrough_style_t::none;
    spreadsheet::strikethrough_type_t strikethrough_type = spreadsheet::strikethrough_type_t::unknown;
    spreadsheet::strikethrough_width_t strikethrough_width = spreadsheet::strikethrough_width_t::unknown;
    spreadsheet::strikethrough_text_t strikethrough_text = spreadsheet::strikethrough_text_t::unknown;

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns == NS_odf_style)
        {
            switch (attr.name)
            {
                case XML_font_name:
                    font_name = attr.value;
                    break;
                case XML_text_underline_color:
                    if (!odf_helper::convert_fo_color(attr.value, underline_red, underline_green, underline_blue))
                    {
                        has_underline = true;
                        underline_is_text_color = true;
                    }
                    else
                    {
                        underline_is_text_color = false;
                    }
                    break;
                case XML_text_underline_mode:
                    has_underline = true;
                    if (attr.value == "skip-white-space")
                        underline_mode = spreadsheet::underline_mode_t::skip_white_space;
                    else
                        underline_mode = spreadsheet::underline_mode_t::continuos;
                    break;
                case XML_text_underline_width:
                {
                    has_underline = true;
                    underline_width = odf_helper::extract_underline_width(attr.value);
                    break;
                }
                case XML_text_underline_style:
                {
                    has_underline = true;
                    underline_style = odf_helper::extract_underline_style(attr.value);
                    break;
                }
                case XML_text_underline_type:
                {
                    has_underline = true;
                    if (attr.value == "none")
                        underline_type = spreadsheet::underline_type_t::none;
                    if (attr.value == "single")
                        underline_type = spreadsheet::underline_type_t::single;
                    if (attr.value == "double")
                        underline_type = spreadsheet::underline_type_t::double_type;
                    break;
                }
                case XML_text_line_through_style:
                {
                    strikethrough_style_map style_map(strikethrough_style_entries, sizeof(strikethrough_style_entries)/sizeof(strikethrough_style_entries[0]), spreadsheet::strikethrough_style_t::none);
                    strikethrough_style = style_map.find(attr.value.data(), attr.value.size());
                    break;
                }
                case XML_text_line_through_type:
                {
                    if (attr.value == "single")
                        strikethrough_type = spreadsheet::strikethrough_type_t::single;
                    else if (attr.value == "double")
                        strikethrough_type = spreadsheet::strikethrough_type_t::double_type;
                    else
                        strikethrough_type = spreadsheet::strikethrough_type_t::unknown;
                    break;
                }
                case XML_text_line_through_width:
                {
                    if (attr.value == "bold")
                        strikethrough_width = spreadsheet::strikethrough_width_t::bold;
                    else
                        strikethrough_width = spreadsheet::strikethrough_width_t::unknown;
                    break;
                }
                case XML_text_line_through_text:
                {
                    if (attr.value == "/")
                        strikethrough_text = spreadsheet::strikethrough_text_t::slash;
                    else if (attr.value == "X")
                        strikethrough_text = spreadsheet::strikethrough_text_t::cross;
                    else
                        strikethrough_text = spreadsheet::strikethrough_text_t::unknown;
                    break;
                }
                default:
                    ;
            }
        }
        else if (attr.ns == NS_odf_fo)
        {
            switch (attr.name)
            {
                case XML_font_size:
                    font_size = to_length(attr.value);
                    break;
                case XML_font_style:
                    italic = attr.value == "italic";
                    break;
                case XML_font_weight:
                    bold = attr.value == "bold";
                    break;
                case XML_color:
                    has_color = odf_helper::convert_fo_color(attr.value, red, green, blue);
                    break;
                default:
                    ;
            }
        }
    }

    // Commit the font data.
    if (!font_name.empty())
        mp_styles->set_font_name(font_name);

    if (font_size.unit == length_unit_t::point)
        mp_styles->set_font_size(font_size.value);

    if (bold)
        mp_styles->set_font_bold(true);

    if (italic)
        mp_styles->set_font_italic(true);

    if (has_color)
        mp_styles->set_font_color(0, red, green, blue);

    if (has_underline)
    {
        if (underline_is_text_color && has_color)
            mp_styles->set_font_underline_color(0, red, green, blue);
        else
            mp_styles->set_font_underline_color(0, underline_red, underline_green, underline_blue);

        mp_styles->set_font_underline_width(underline_width);
        mp_styles->set_font_underline(underline_style);
        mp_styles->set_font_underline_type(underline_type);
        mp_styles->set_font_underline_mode(underline_mode);
    }

    if (strikethrough_style != spreadsheet::strikethrough_style_t::none)
    {
        mp_styles->set_strikethrough_style(strikethrough_style);
        mp_styles->set_strikethrough_width(strikethrough_width);
        mp_styles->set_strikethrough_type(strikethrough_type);
        mp_styles->set_strikethrough_text(strikethrough_text);
    }

    size_t font_id = mp_styles->commit_font();

    switch (m_current_style->family)
    {
        case style_family_table_cell:
        {
            odf_style::cell* data = m_current_style->cell_data;
            data->font = font_id;
        }
        break;
        case style_family_text:
        {
            odf_style::text* data = m_current_style->text_data;
            data->font = font_id;
        }
        break;
        default:
            ;
    }
}

void styles_context::commit_default_styles()
{
    if (!mp_styles)
        return;

    // Set default styles. Default styles must be associated with an index of 0.
    // Set empty styles for all style types before importing real styles.
    mp_styles->commit_font();
    mp_styles->commit_fill();
    mp_styles->commit_border();
    mp_styles->commit_cell_protection();
    mp_styles->commit_number_format();
    mp_styles->commit_cell_style_xf();
    mp_styles->commit_cell_xf();
    mp_styles->commit_cell_style();
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
