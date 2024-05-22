/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "odf_style_context.hpp"
#include "impl_utils.hpp"
#include "odf_namespace_types.hpp"
#include "odf_token_constants.hpp"
#include "odf_helper.hpp"
#include "session_context.hpp"
#include "ods_session_data.hpp"

#include <mdds/sorted_string_map.hpp>

#include <orcus/spreadsheet/import_interface_styles.hpp>
#include <orcus/spreadsheet/import_interface_strikethrough.hpp>

namespace ss = orcus::spreadsheet;

namespace orcus {

namespace {

namespace style_family {

using map_type = mdds::sorted_string_map<odf_style_family>;

constexpr map_type::entry_type entries[] =
{
    { "graphic", style_family_graphic },
    { "paragraph", style_family_paragraph },
    { "table", style_family_table },
    { "table-cell", style_family_table_cell },
    { "table-column", style_family_table_column },
    { "table-row", style_family_table_row },
    { "text", style_family_text }
};

const map_type& get()
{
    static const map_type mt(entries, std::size(entries), style_family_unknown);
    return mt;
}

} // namespace style_family

odf_style_family to_style_family(std::string_view val)
{
    return style_family::get().find(val.data(), val.size());
}

std::string_view to_string(odf_style_family family)
{
    return style_family::get().find_key(family);
}

namespace st_style {

typedef mdds::sorted_string_map<ss::strikethrough_style_t> map_type;

// Keys must be sorted.
constexpr map_type::entry_type entries[] =
{
    { "dash", ss::strikethrough_style_t::dash },
    { "dot-dash", ss::strikethrough_style_t::dot_dash },
    { "dot-dot-dash", ss::strikethrough_style_t::dot_dot_dash },
    { "dotted", ss::strikethrough_style_t::dotted },
    { "long-dash", ss::strikethrough_style_t::long_dash },
    { "none", ss::strikethrough_style_t::none },
    { "solid", ss::strikethrough_style_t::solid },
    { "wave", ss::strikethrough_style_t::wave },
};

const map_type& get()
{
    static const map_type mt(entries, std::size(entries), ss::strikethrough_style_t::none);
    return mt;
}

} // namespace st_style

} // anonymous namespace

style_context::style_context(session_context& session_cxt, const tokens& tk, ss::iface::import_styles* iface_styles) :
    xml_context_base(session_cxt, tk),
    mp_styles(iface_styles)
{
}

void style_context::start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);

    if (ns == NS_odf_style)
    {
        switch (name)
        {
            case XML_style:
            {
                xml_element_expected(parent, XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN);

                std::string_view style_name;
                std::string_view display_style_name;
                std::string_view parent_style_name;
                std::optional<std::string_view> data_style_name;
                odf_style_family family = style_family_unknown;

                for (const xml_token_attr_t& attr : attrs)
                {
                    if (attr.ns == NS_odf_style)
                    {
                        switch (attr.name)
                        {
                            case XML_name:
                                style_name = intern(attr.value);
                                break;
                            case XML_display_name:
                                display_style_name = intern(attr.value);
                                break;
                            case XML_family:
                                family = to_style_family(attr.value);
                                break;
                            case XML_parent_style_name:
                                parent_style_name = intern(attr.value);
                                break;
                            case XML_data_style_name:
                                data_style_name = attr.value; // no need to intern
                                break;
                        }
                    }
                }

                m_current_style = std::make_unique<odf_style>(
                    style_name, display_style_name, family, parent_style_name);

                if (data_style_name && family == style_family_table_cell)
                {
                    const auto& ods_data = get_session_context().get_data<ods_session_data>();
                    const auto& numfmt_name2id = ods_data.number_formats.name2id_map;

                    if (auto it = numfmt_name2id.find(*data_style_name); it != numfmt_name2id.end())
                    {
                        // record the number format id associated with the name.
                        auto& data = std::get<odf_style::cell>(m_current_style->data);
                        data.number_format = it->second;
                    }
                    else
                    {
                        if (get_config().debug)
                        {
                            std::ostringstream os;
                            os << "no number style found for the data style name of '" << *data_style_name << "'";
                            warn(os.str());
                        }
                    }
                }
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

bool style_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    return pop_stack(ns, name);
}

void style_context::reset()
{
    m_current_style.reset();
}

std::unique_ptr<odf_style> style_context::pop_style()
{
    return std::move(m_current_style);
}

void style_context::characters(std::string_view /*str*/, bool /*transient*/)
{
}

void style_context::start_paragraph_properties(const xml_token_pair_t& parent, const xml_token_attrs_t& attrs)
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
                    auto v = odf::extract_hor_alignment_style(attr.value);

                    switch (m_current_style->family)
                    {
                        case style_family_table_cell:
                        {
                            auto& data = std::get<odf_style::cell>(m_current_style->data);
                            data.hor_align = v;
                            break;
                        }
                        case style_family_paragraph:
                        {
                            auto& data = std::get<odf_style::paragraph>(m_current_style->data);
                            data.hor_align = v;
                            break;
                        }
                        default:
                        {
                            if (get_config().debug)
                            {
                                std::ostringstream os;
                                os << "unhandled fo:text-align attribute (family=" << to_string(m_current_style->family) << ")";
                                warn(os.str());
                            }
                        }
                    }
                    break;
                }
                default:
                    ;
            }
        }
    }
}

void style_context::start_text_properties(const xml_token_pair_t& parent, const xml_token_attrs_t& attrs)
{
    static const xml_elem_set_t expected = {
        { NS_odf_style, XML_style },
        { NS_odf_text, XML_list_level_style_number },
        { NS_odf_text, XML_list_level_style_bullet },
    };
    xml_element_expected(parent, expected);

    if (parent != xml_token_pair_t(NS_odf_style, XML_style))
        // TODO : handle this properly in the future.
        return;

    // NB: no need to intern the font names since they are consumed at the end
    // of this function.
    std::optional<std::string_view> font_name;
    std::optional<std::string_view> font_name_asian;
    std::optional<std::string_view> font_name_complex;
    std::optional<length_t> font_size;
    std::optional<length_t> font_size_asian;
    std::optional<length_t> font_size_complex;
    std::optional<bool> bold;
    std::optional<bool> bold_asian;
    std::optional<bool> bold_complex;
    std::optional<bool> italic;
    std::optional<bool> italic_asian;
    std::optional<bool> italic_complex;
    std::optional<ss::color_rgb_t> color;

    std::optional<ss::color_rgb_t> underline_color;
    std::optional<ss::underline_t> underline_style;
    std::optional<ss::underline_count_t> underline_type;
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
                case XML_font_name_asian:
                    font_name_asian = attr.value;
                    break;
                case XML_font_name_complex:
                    font_name_complex = attr.value;
                    break;
                case XML_font_size_asian:
                    font_size_asian = to_length(attr.value);
                    break;
                case XML_font_size_complex:
                    font_size_complex = to_length(attr.value);
                    break;
                case XML_font_style_asian:
                    italic_asian = attr.value == "italic";
                    break;
                case XML_font_style_complex:
                    italic_complex = attr.value == "italic";
                    break;
                case XML_font_weight_asian:
                    bold_asian = attr.value == "bold";
                    break;
                case XML_font_weight_complex:
                    bold_complex = attr.value == "bold";
                    break;
                case XML_text_underline_color:
                    if (attr.value != "font-color")
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
                        underline_type = ss::underline_count_t::none;
                    else if (attr.value == "single")
                        underline_type = ss::underline_count_t::single_count;
                    else if (attr.value == "double")
                        underline_type = ss::underline_count_t::double_count;
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
            }
        }
    }

    // Commit the font data.
    auto* font_style = mp_styles->start_font_style();
    ENSURE_INTERFACE(font_style, import_font_style);

    if (font_name)
        font_style->set_name(*font_name);

    if (font_name_asian)
        font_style->set_name_asian(*font_name_asian);

    if (font_name_complex)
        font_style->set_name_complex(*font_name_complex);

    if (font_size && font_size->unit == length_unit_t::point)
        font_style->set_size(font_size->value);

    if (font_size_asian && font_size_asian->unit == length_unit_t::point)
        font_style->set_size_asian(font_size_asian->value);

    if (font_size_complex && font_size_complex->unit == length_unit_t::point)
        font_style->set_size_complex(font_size_complex->value);

    if (bold)
        font_style->set_bold(*bold);

    if (bold_asian)
        font_style->set_bold_asian(*bold_asian);

    if (bold_complex)
        font_style->set_bold_complex(*bold_complex);

    if (italic)
        font_style->set_italic(*italic);

    if (italic_asian)
        font_style->set_italic_asian(*italic_asian);

    if (italic_complex)
        font_style->set_italic_complex(*italic_complex);

    if (color)
        font_style->set_color(255, color->red, color->green, color->blue);

    if (underline_color)
        // Separate underline color is specified.
        font_style->set_underline_color(255, underline_color->red, underline_color->green, underline_color->blue);

    if (underline_width)
        font_style->set_underline_width(*underline_width);

    if (underline_style)
        font_style->set_underline(*underline_style);

    if (underline_type)
        font_style->set_underline_type(*underline_type);

    if (underline_mode)
        font_style->set_underline_mode(*underline_mode);

    if (auto* st = font_style->start_strikethrough(); st)
    {
        if (strikethrough_style)
            st->set_style(*strikethrough_style);

        if (strikethrough_type)
            st->set_type(*strikethrough_type);

        if (strikethrough_width)
            st->set_width(*strikethrough_width);

        if (strikethrough_text)
            st->set_text(*strikethrough_text);

        st->commit();
    }

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

void style_context::start_table_cell_properties(const xml_token_pair_t& parent, const xml_token_attrs_t& attrs)
{
    xml_element_expected(parent, NS_odf_style, XML_style);

    if (m_current_style->family != style_family_table_cell)
        throw xml_structure_error("expected table_cell family style in cell_properties element");

    if (!mp_styles)
        return;

    auto& data = std::get<odf_style::cell>(m_current_style->data);

    std::optional<spreadsheet::color_rgb_t> bg_color;

    std::optional<bool> locked;
    std::optional<bool> hidden;
    std::optional<bool> formula_hidden;
    std::optional<bool> print_content;

    bool cell_protection_set = false;

    using border_map_type = std::map<ss::border_direction_t, odf::border_details_t>;
    border_map_type border_styles;

    ss::ver_alignment_t ver_alignment = ss::ver_alignment_t::unknown;
    std::optional<bool> wrap_text;
    std::optional<bool> shrink_to_fit;

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
                case XML_wrap_option:
                {
                    wrap_text = (attr.value == "wrap");
                    break;
                }
                default:
                    ;
            }
        }
        else if (attr.ns == NS_odf_style)
        {
            switch (attr.name)
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
                case XML_shrink_to_fit:
                    shrink_to_fit = to_bool(attr.value);
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
        auto* fill_style = mp_styles->start_fill_style();
        ENSURE_INTERFACE(fill_style, import_fill_style);

        fill_style->set_pattern_type(ss::fill_pattern_t::solid);
        fill_style->set_fg_color(255, bg_color->red, bg_color->green, bg_color->blue);
        fill_id = fill_style->commit();
    }

    if (!border_styles.empty())
    {
        auto* border_style = mp_styles->start_border_style();
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
        auto* cell_protection = mp_styles->start_cell_protection();
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
            data.wrap_text = wrap_text;
            data.shrink_to_fit = shrink_to_fit;
            break;
        }
        default:
            ;
    }
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
