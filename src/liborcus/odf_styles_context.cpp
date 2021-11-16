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
#include <optional>

namespace ss = orcus::spreadsheet;

namespace orcus {

namespace {

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

namespace st_style {

typedef mdds::sorted_string_map<ss::strikethrough_style_t> map_type;

// Keys must be sorted.
const std::vector<map_type::entry> entries =
{
    { MDDS_ASCII("dash"), ss::strikethrough_style_t::dash },
    { MDDS_ASCII("dot-dash"), ss::strikethrough_style_t::dot_dash },
    { MDDS_ASCII("dot-dot-dash"), ss::strikethrough_style_t::dot_dot_dash },
    { MDDS_ASCII("dotted"), ss::strikethrough_style_t::dotted },
    { MDDS_ASCII("long-dash"), ss::strikethrough_style_t::long_dash},
    { MDDS_ASCII("none"), ss::strikethrough_style_t::none },
    { MDDS_ASCII("solid"), ss::strikethrough_style_t::solid },
    { MDDS_ASCII("wave"), ss::strikethrough_style_t::wave },
};

const map_type& get()
{
    static map_type mt(entries.data(), entries.size(), ss::strikethrough_style_t::none);
    return mt;
}

} // namespace st_style

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
                    m_has_hor_alignment = odf::extract_hor_alignment_style(attr.value, m_hor_alignment);
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
                const xml_elem_set_t expected_parents = {
                    { NS_odf_office, XML_automatic_styles },
                    { NS_odf_office, XML_styles },
                };
                xml_element_expected(parent, expected_parents);

                style_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);
                m_current_style.reset(new odf_style(func.get_name(), func.get_family(), func.get_parent()));
                break;
            }
            case XML_table_column_properties:
            {
                xml_element_expected(parent, NS_odf_style, XML_style);
                col_prop_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);
                assert(m_current_style->family == style_family_table_column);
                m_current_style->column_data->width = func.get_width();
                break;
            }
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

                break;
            }
            case XML_text_properties:
                start_text_properties(parent, attrs);
                break;
            case XML_table_cell_properties:
                start_table_cell_properties(parent, attrs);
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

                break;
            }
        }
    }
    return pop_stack(ns, name);
}

void styles_context::characters(const pstring& /*str*/, bool /*transient*/)
{
}

void styles_context::start_text_properties(const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    static const xml_elem_set_t expected = {
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

    std::optional<std::string_view> font_name;
    std::optional<length_t> font_size;
    std::optional<bool> bold;
    std::optional<bool> italic;
    std::optional<ss::color_rgb_t> color;

    bool underline_use_font_color = false;
    std::optional<ss::color_rgb_t> underline_color;
    std::optional<ss::underline_t> underline_style;
    std::optional<ss::underline_type_t> underline_type;
    std::optional<ss::underline_width_t> underline_width;
    std::optional<ss::underline_mode_t> underline_mode;

    std::optional<ss::strikethrough_style_t> strikethrough_style;
    std::optional<ss::strikethrough_type_t> strikethrough_type;
    std::optional<ss::strikethrough_width_t> strikethrough_width;
    std::optional<ss::strikethrough_text_t> strikethrough_text;

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
                    underline_use_font_color = (attr.value == "font-color");
                    underline_color = odf::convert_fo_color(attr.value);
                    break;
                case XML_text_underline_mode:
                    if (attr.value == "skip-white-space")
                        underline_mode = ss::underline_mode_t::skip_white_space;
                    else
                        underline_mode = ss::underline_mode_t::continuos;
                    break;
                case XML_text_underline_width:
                {
                    underline_width = odf::extract_underline_width(attr.value);
                    break;
                }
                case XML_text_underline_style:
                {
                    underline_style = odf::extract_underline_style(attr.value);
                    break;
                }
                case XML_text_underline_type:
                {
                    if (attr.value == "none")
                        underline_type = ss::underline_type_t::none;
                    else if (attr.value == "single")
                        underline_type = ss::underline_type_t::single;
                    else if (attr.value == "double")
                        underline_type = ss::underline_type_t::double_type;
                    break;
                }
                case XML_text_line_through_style:
                {
                    strikethrough_style = st_style::get().find(attr.value.data(), attr.value.size());
                    break;
                }
                case XML_text_line_through_type:
                {
                    if (attr.value == "single")
                        strikethrough_type = ss::strikethrough_type_t::single;
                    else if (attr.value == "double")
                        strikethrough_type = ss::strikethrough_type_t::double_type;
                    else
                        strikethrough_type = ss::strikethrough_type_t::unknown;
                    break;
                }
                case XML_text_line_through_width:
                {
                    if (attr.value == "bold")
                        strikethrough_width = ss::strikethrough_width_t::bold;
                    else
                        strikethrough_width = ss::strikethrough_width_t::unknown;
                    break;
                }
                case XML_text_line_through_text:
                {
                    if (attr.value == "/")
                        strikethrough_text = ss::strikethrough_text_t::slash;
                    else if (attr.value == "X")
                        strikethrough_text = ss::strikethrough_text_t::cross;
                    else
                        strikethrough_text = ss::strikethrough_text_t::unknown;
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
                    color = odf::convert_fo_color(attr.value);
                    break;
                default:
                    ;
            }
        }
    }

    // Commit the font data.

    if (font_name)
        mp_styles->set_font_name(*font_name);

    if (font_size && font_size->unit == length_unit_t::point)
        mp_styles->set_font_size(font_size->value);

    if (bold)
        mp_styles->set_font_bold(*bold);

    if (italic)
        mp_styles->set_font_italic(*italic);

    if (color)
        mp_styles->set_font_color(0, color->red, color->green, color->blue);

    if (underline_color)
        // Separate underline color is specified.
        mp_styles->set_font_underline_color(0, underline_color->red, underline_color->green, underline_color->blue);
    else if (color && underline_use_font_color)
        // Use the same color as the font.
        mp_styles->set_font_underline_color(0, color->red, color->green, color->blue);

    if (underline_width)
        mp_styles->set_font_underline_width(*underline_width);

    if (underline_style)
        mp_styles->set_font_underline(*underline_style);

    if (underline_type)
        mp_styles->set_font_underline_type(*underline_type);

    if (underline_mode)
        mp_styles->set_font_underline_mode(*underline_mode);

    if (strikethrough_style)
        mp_styles->set_strikethrough_style(*strikethrough_style);

    if (strikethrough_type)
        mp_styles->set_strikethrough_type(*strikethrough_type);

    if (strikethrough_width)
        mp_styles->set_strikethrough_width(*strikethrough_width);

    if (strikethrough_text)
        mp_styles->set_strikethrough_text(*strikethrough_text);

    size_t font_id = mp_styles->commit_font();

    switch (m_current_style->family)
    {
        case style_family_table_cell:
        {
            odf_style::cell* data = m_current_style->cell_data;
            data->font = font_id;
            break;
        }
        case style_family_text:
        {
            odf_style::text* data = m_current_style->text_data;
            data->font = font_id;
            break;
        }
        default:
            ;
    }
}

void styles_context::start_table_cell_properties(const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    xml_element_expected(parent, NS_odf_style, XML_style);

    if (m_current_style->family != style_family_table_cell)
        throw xml_structure_error("expected table_cell family style in cell_properties element");

    if (!mp_styles)
        return;

    m_current_style->cell_data->automatic_style = m_automatic_styles;

    using border_map_type = std::map<ss::border_direction_t, odf::border_details_t>;

    ss::color_elem_t background_red = 0;
    ss::color_elem_t background_green = 0;
    ss::color_elem_t background_blue = 0;

    bool background_color = false;
    bool locked = false;
    bool hidden = false;
    bool formula_hidden = false;
    bool print_content = false;
    bool cell_protection = false;

    border_map_type border_style_dir_pair;

    ss::ver_alignment_t ver_alignment = ss::ver_alignment_t::unknown;
    bool has_ver_alignment = false;

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns == NS_odf_fo)
        {
            switch (attr.name)
            {
                case XML_background_color:
                    background_color = odf::convert_fo_color(attr.value, background_red,
                            background_green, background_blue);
                    break;
                case XML_border:
                {
                    odf::border_details_t border_details = odf::extract_border_details(attr.value);
                    border_style_dir_pair.insert(std::make_pair(ss::border_direction_t::top, border_details));
                    border_style_dir_pair.insert(std::make_pair(ss::border_direction_t::bottom, border_details));
                    border_style_dir_pair.insert(std::make_pair(ss::border_direction_t::left, border_details));
                    border_style_dir_pair.insert(std::make_pair(ss::border_direction_t::right, border_details));
                    break;
                }
                case XML_border_top:
                {
                    odf::border_details_t border_details = odf::extract_border_details(attr.value);
                    border_style_dir_pair.insert(std::make_pair(ss::border_direction_t::top, border_details));
                    break;
                }
                case XML_border_bottom:
                {
                    odf::border_details_t border_details = odf::extract_border_details(attr.value);
                    border_style_dir_pair.insert(std::make_pair(ss::border_direction_t::bottom, border_details));
                    break;
                }
                case XML_border_left:
                {
                    odf::border_details_t border_details = odf::extract_border_details(attr.value);
                    border_style_dir_pair.insert(std::make_pair(ss::border_direction_t::left, border_details));
                    break;
                }
                case XML_border_right:
                {
                    odf::border_details_t border_details = odf::extract_border_details(attr.value);
                    border_style_dir_pair.insert(std::make_pair(ss::border_direction_t::right, border_details));
                    break;
                }
                case XML_diagonal_bl_tr:
                {
                    odf::border_details_t border_details = odf::extract_border_details(attr.value);
                    border_style_dir_pair.insert(std::make_pair(ss::border_direction_t::diagonal_bl_tr, border_details));
                    break;
                }
                case XML_diagonal_tl_br:
                {
                    odf::border_details_t border_details = odf::extract_border_details(attr.value);
                    border_style_dir_pair.insert(std::make_pair(ss::border_direction_t::diagonal_tl_br, border_details));
                    break;
                }
                default:
                    ;
            }
        }
        else if (attr.ns == NS_odf_style)
        {
            switch(attr.name)
            {
                case XML_print_content:
                {
                    cell_protection = true;
                    print_content = attr.value == "true";
                    break;
                }
                case XML_cell_protect:
                {
                    cell_protection = true;
                    if (attr.value == "protected")
                        locked = true;
                    else if (attr.value == "hidden-and-protected")
                    {
                        locked = true;
                        hidden = true;
                    }
                    else if (attr.value == "formula-hidden")
                        formula_hidden = true;
                    else if (attr.value == "protected formula-hidden" || attr.value == "formula-hidden protected")
                    {
                        formula_hidden = true;
                        locked = true;
                    }
                    break;
                }
                case XML_vertical_align:
                    has_ver_alignment = odf::extract_ver_alignment_style(attr.value, ver_alignment);
                    break;
                default:
                    ;
            }
        }
    }

    if (background_color)
    {
        mp_styles->set_fill_pattern_type(ss::fill_pattern_t::solid);
        mp_styles->set_fill_fg_color(255, background_red, background_green, background_blue);
    }

    size_t fill_id = mp_styles->commit_fill();

    if (!border_style_dir_pair.empty())
    {
        for (auto itr = border_style_dir_pair.begin(); itr != border_style_dir_pair.end(); ++itr)
        {
            mp_styles->set_border_color(itr->first, 0, itr->second.red, itr->second.green, itr->second.blue);
            mp_styles->set_border_style(itr->first, itr->second.border_style);
            mp_styles->set_border_width(itr->first, itr->second.border_width.value, itr->second.border_width.unit);
        }
    }

    size_t border_id = mp_styles->commit_border();

    if (cell_protection)
    {
        mp_styles->set_cell_hidden(hidden);
        mp_styles->set_cell_locked(locked);
        mp_styles->set_cell_print_content(print_content);
        mp_styles->set_cell_formula_hidden(formula_hidden);
    }

    if (has_ver_alignment)
        mp_styles->set_xf_vertical_alignment(ver_alignment);

    size_t cell_protection_id = mp_styles->commit_cell_protection();

    switch (m_current_style->family)
    {
        case style_family_table_cell:
        {
            odf_style::cell* data = m_current_style->cell_data;
            data->fill = fill_id;
            data->border = border_id;
            data->protection = cell_protection_id;
            break;
        }
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
