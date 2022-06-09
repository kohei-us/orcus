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
#include "impl_utils.hpp"

#include <orcus/measurement.hpp>
#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/spreadsheet/import_interface_styles.hpp>

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

odf_style_family to_style_family(std::string_view val)
{
    static style_family_map map(style_family_entries, ORCUS_N_ELEMENTS(style_family_entries), style_family_unknown);
    return map.find(val.data(), val.size());
}

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

} // anonymous namespace

styles_context::styles_context(
    session_context& session_cxt, const tokens& tk, odf_styles_map_type& styles,
    spreadsheet::iface::import_styles* iface_styles) :
    xml_context_base(session_cxt, tk),
    mp_styles(iface_styles),
    m_styles(styles),
    m_automatic_styles(false),
    m_cxt_number_format(session_cxt, tk, mp_styles)
{
    m_cxt_number_format.transfer_common(*this);

    commit_default_styles();
}

xml_context_base* styles_context::create_child_context(xmlns_id_t ns, xml_token_t /*name*/)
{
    if (ns == NS_odf_number)
    {
        m_cxt_number_format.reset();
        return &m_cxt_number_format;
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

                std::string_view style_name;
                std::string_view parent_style_name;
                odf_style_family family = style_family_unknown;

                for (const xml_token_attr_t& attr : attrs)
                {
                    if (attr.ns == NS_odf_style)
                    {
                        switch (attr.name)
                        {
                            case XML_name:
                                style_name = attr.value;
                                break;
                            case XML_family:
                                family = to_style_family(attr.value);
                                break;
                            case XML_parent_style_name:
                                parent_style_name = attr.value;
                                break;
                        }
                    }
                }

                m_current_style = std::make_unique<odf_style>(style_name, family, parent_style_name);
                break;
            }
            case XML_table_column_properties:
            {
                xml_element_expected(parent, NS_odf_style, XML_style);
                assert(m_current_style->family == style_family_table_column);

                for (const xml_token_attr_t& attr: attrs)
                {
                    if (attr.ns == NS_odf_style)
                    {
                        switch (attr.name)
                        {
                            case XML_column_width:
                            {
                                std::get<odf_style::column>(m_current_style->data).width = to_length(attr.value);
                                break;
                            }
                        }
                    }
                }

                break;
            }
            case XML_table_row_properties:
            {
                xml_element_expected(parent, NS_odf_style, XML_style);
                assert(m_current_style->family == style_family_table_row);

                for (const xml_token_attr_t& attr : attrs)
                {
                    if (attr.ns == NS_odf_style)
                    {
                        switch (attr.name)
                        {
                            case XML_row_height:
                            {
                                auto& data = std::get<odf_style::row>(m_current_style->data);
                                data.height = to_length(attr.value);
                                data.height_set = true;
                                break;
                            }
                        }
                    }
                }

                break;
            }
            case XML_table_properties:
                xml_element_expected(parent, NS_odf_style, XML_style);
                break;
            case XML_paragraph_properties:
                start_paragraph_properties(parent, attrs);
                break;
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
                if (!m_current_style)
                    break;

                if (mp_styles && m_current_style->family == style_family_table_cell)
                {
                    auto& cell = std::get<odf_style::cell>(m_current_style->data);

                    if (cell.automatic_style)
                    {
                        // Import it into the direct cell style store
                        auto* xf = mp_styles->get_xf(ss::xf_category_t::cell);
                        ENSURE_INTERFACE(xf, import_xf);
                        xf->set_font(cell.font);
                        xf->set_fill(cell.fill);
                        xf->set_border(cell.border);
                        xf->set_protection(cell.protection);
                        xf->set_horizontal_alignment(cell.hor_align);
                        xf->set_vertical_alignment(cell.ver_align);
                        cell.xf = xf->commit();
                    }
                    else
                    {
                        // Import it into the cell style xf store, and reference
                        // its index in the cell style name store.
                        auto* xf = mp_styles->get_xf(ss::xf_category_t::cell_style);
                        ENSURE_INTERFACE(xf, import_xf);
                        xf->set_font(cell.font);
                        xf->set_fill(cell.fill);
                        xf->set_border(cell.border);
                        xf->set_protection(cell.protection);
                        xf->set_horizontal_alignment(cell.hor_align);
                        xf->set_vertical_alignment(cell.ver_align);
                        size_t style_xf_id = xf->commit();

                        auto* cell_style = mp_styles->get_cell_style();
                        ENSURE_INTERFACE(cell_style, import_cell_style);

                        cell_style->set_name(m_current_style->name);
                        cell_style->set_xf(style_xf_id);
                        cell_style->set_parent_name(m_current_style->parent_name);

                        cell.xf = cell_style->commit();
                    }
                }

                std::string_view style_name = m_current_style->name;
                m_styles.emplace(style_name, std::move(m_current_style));
                assert(!m_current_style);

                break;
            }
        }
    }
    return pop_stack(ns, name);
}

void styles_context::characters(std::string_view /*str*/, bool /*transient*/)
{
}

void styles_context::start_paragraph_properties(const xml_token_pair_t& parent, const xml_attrs_t& attrs)
{
    xml_element_expected(parent, NS_odf_style, XML_style);

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns == NS_odf_fo)
        {
            switch (attr.name)
            {
                case XML_text_align:
                {
                    auto& data = std::get<odf_style::cell>(m_current_style->data);
                    data.hor_align = odf::extract_hor_alignment_style(attr.value);
                    break;
                }
                default:
                    ;
            }
        }
    }
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
                        underline_mode = ss::underline_mode_t::continuous;
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
                        underline_type = ss::underline_type_t::single_type;
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
                        strikethrough_type = ss::strikethrough_type_t::single_type;
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
    auto* font_style = mp_styles->get_font_style();
    ENSURE_INTERFACE(font_style, import_font_style);

    if (font_name)
        font_style->set_name(*font_name);

    if (font_size && font_size->unit == length_unit_t::point)
        font_style->set_size(font_size->value);

    if (bold)
        font_style->set_bold(*bold);

    if (italic)
        font_style->set_italic(*italic);

    if (color)
        font_style->set_color(255, color->red, color->green, color->blue);

    if (underline_color)
        // Separate underline color is specified.
        font_style->set_underline_color(255, underline_color->red, underline_color->green, underline_color->blue);
    else if (color && underline_use_font_color)
        // Use the same color as the font.
        font_style->set_underline_color(255, color->red, color->green, color->blue);

    if (underline_width)
        font_style->set_underline_width(*underline_width);

    if (underline_style)
        font_style->set_underline(*underline_style);

    if (underline_type)
        font_style->set_underline_type(*underline_type);

    if (underline_mode)
        font_style->set_underline_mode(*underline_mode);

    if (strikethrough_style)
        font_style->set_strikethrough_style(*strikethrough_style);

    if (strikethrough_type)
        font_style->set_strikethrough_type(*strikethrough_type);

    if (strikethrough_width)
        font_style->set_strikethrough_width(*strikethrough_width);

    if (strikethrough_text)
        font_style->set_strikethrough_text(*strikethrough_text);

    size_t font_id = font_style->commit();

    switch (m_current_style->family)
    {
        case style_family_table_cell:
        {
            auto& data = std::get<odf_style::cell>(m_current_style->data);
            data.font = font_id;
            break;
        }
        case style_family_text:
        {
            auto& data = std::get<odf_style::text>(m_current_style->data);
            data.font = font_id;
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

    auto& data = std::get<odf_style::cell>(m_current_style->data);
    data.automatic_style = m_automatic_styles;

    std::optional<spreadsheet::color_rgb_t> bg_color;

    std::optional<bool> locked;
    std::optional<bool> hidden;
    std::optional<bool> formula_hidden;
    std::optional<bool> print_content;

    bool cell_protection_set = false;

    using border_map_type = std::map<ss::border_direction_t, odf::border_details_t>;
    border_map_type border_styles;

    ss::ver_alignment_t ver_alignment = ss::ver_alignment_t::unknown;

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns == NS_odf_fo)
        {
            switch (attr.name)
            {
                case XML_background_color:
                    bg_color = odf::convert_fo_color(attr.value);
                    break;
                case XML_border:
                {
                    odf::border_details_t border_details = odf::extract_border_details(attr.value);

                    const ss::border_direction_t dirs[] =
                    {
                        ss::border_direction_t::top,
                        ss::border_direction_t::bottom,
                        ss::border_direction_t::left,
                        ss::border_direction_t::right
                    };

                    for (const auto dir : dirs)
                        border_styles.insert_or_assign(dir, border_details);

                    break;
                }
                case XML_border_top:
                {
                    odf::border_details_t border_details = odf::extract_border_details(attr.value);
                    border_styles.insert_or_assign(ss::border_direction_t::top, border_details);
                    break;
                }
                case XML_border_bottom:
                {
                    odf::border_details_t border_details = odf::extract_border_details(attr.value);
                    border_styles.insert_or_assign(ss::border_direction_t::bottom, border_details);
                    break;
                }
                case XML_border_left:
                {
                    odf::border_details_t border_details = odf::extract_border_details(attr.value);
                    border_styles.insert_or_assign(ss::border_direction_t::left, border_details);
                    break;
                }
                case XML_border_right:
                {
                    odf::border_details_t border_details = odf::extract_border_details(attr.value);
                    border_styles.insert_or_assign(ss::border_direction_t::right, border_details);
                    break;
                }
                case XML_diagonal_bl_tr:
                {
                    odf::border_details_t border_details = odf::extract_border_details(attr.value);
                    border_styles.insert_or_assign(ss::border_direction_t::diagonal_bl_tr, border_details);
                    break;
                }
                case XML_diagonal_tl_br:
                {
                    odf::border_details_t border_details = odf::extract_border_details(attr.value);
                    border_styles.insert_or_assign(ss::border_direction_t::diagonal_tl_br, border_details);
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
                    cell_protection_set = true;
                    print_content = to_bool(attr.value);
                    break;
                }
                case XML_cell_protect:
                {
                    if (attr.value == "protected")
                    {
                        cell_protection_set = true;
                        locked = true;
                    }
                    else if (attr.value == "hidden-and-protected")
                    {
                        cell_protection_set = true;
                        locked = true;
                        hidden = true;
                    }
                    else if (attr.value == "formula-hidden")
                    {
                        cell_protection_set = true;
                        formula_hidden = true;
                    }
                    else if (attr.value == "protected formula-hidden" || attr.value == "formula-hidden protected")
                    {
                        cell_protection_set = true;
                        formula_hidden = true;
                        locked = true;
                    }
                    else if (attr.value == "none")
                    {
                        cell_protection_set = true;
                        locked = false;
                        hidden = false;
                        formula_hidden = false;
                    }
                    break;
                }
                case XML_vertical_align:
                    ver_alignment = odf::extract_ver_alignment_style(attr.value);
                    break;
                default:
                    ;
            }
        }
    }

    std::size_t fill_id = 0;
    std::size_t border_id = 0;
    std::size_t cell_protection_id = 0;

    if (bg_color)
    {
        auto* fill_style = mp_styles->get_fill_style();
        ENSURE_INTERFACE(fill_style, import_fill_style);

        fill_style->set_pattern_type(ss::fill_pattern_t::solid);
        fill_style->set_fg_color(255, bg_color->red, bg_color->green, bg_color->blue);
        fill_id = fill_style->commit();
    }

    if (!border_styles.empty())
    {
        auto* border_style = mp_styles->get_border_style();
        ENSURE_INTERFACE(border_style, import_border_style);

        for (const auto& [dir, details] : border_styles)
        {
            border_style->set_color(dir, 255, details.red, details.green, details.blue);
            border_style->set_style(dir, details.border_style);
            border_style->set_width(dir, details.border_width.value, details.border_width.unit);
        }

        border_id = border_style->commit();
    }

    if (cell_protection_set)
    {
        auto* cell_protection = mp_styles->get_cell_protection();
        ENSURE_INTERFACE(cell_protection, import_cell_protection);

        if (hidden)
            cell_protection->set_hidden(*hidden);

        if (locked)
            cell_protection->set_locked(*locked);

        if (print_content)
            cell_protection->set_print_content(*print_content);

        if (formula_hidden)
            cell_protection->set_formula_hidden(*formula_hidden);

        cell_protection_id = cell_protection->commit();
    }

    switch (m_current_style->family)
    {
        case style_family_table_cell:
        {
            data.fill = fill_id;
            data.border = border_id;
            data.protection = cell_protection_id;
            data.ver_align = ver_alignment;
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

    auto* font_style = mp_styles->get_font_style();
    ENSURE_INTERFACE(font_style, import_font_style);

    auto* fill_style = mp_styles->get_fill_style();
    ENSURE_INTERFACE(fill_style, import_fill_style);

    auto* border_style = mp_styles->get_border_style();
    ENSURE_INTERFACE(border_style, import_border_style);

    auto* cell_protection = mp_styles->get_cell_protection();
    ENSURE_INTERFACE(cell_protection, import_cell_protection);

    auto* number_format = mp_styles->get_number_format();
    ENSURE_INTERFACE(number_format, import_number_format);

    // Set default styles. Default styles must be associated with an index of 0.
    // Set empty styles for all style types before importing real styles.
    font_style->commit();
    fill_style->commit();
    border_style->commit();
    cell_protection->commit();
    number_format->commit();

    auto* xf = mp_styles->get_xf(ss::xf_category_t::cell);
    ENSURE_INTERFACE(xf, import_xf);
    xf->commit();

    xf = mp_styles->get_xf(ss::xf_category_t::cell_style);
    ENSURE_INTERFACE(xf, import_xf);
    xf->commit();

    auto* cell_style = mp_styles->get_cell_style();
    ENSURE_INTERFACE(cell_style, import_cell_style);
    cell_style->commit();
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
