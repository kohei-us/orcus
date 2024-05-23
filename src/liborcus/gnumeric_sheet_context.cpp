/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gnumeric_sheet_context.hpp"
#include "gnumeric_cell_context.hpp"
#include "gnumeric_token_constants.hpp"
#include "gnumeric_namespace_types.hpp"
#include "impl_utils.hpp"

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/spreadsheet/import_interface_styles.hpp>
#include <orcus/measurement.hpp>

namespace ss = orcus::spreadsheet;

namespace orcus {

namespace {

ss::condition_operator_t get_condition_operator(int val)
{
    switch (val)
    {
        case 0:
            return ss::condition_operator_t::between;
        case 1:
            return ss::condition_operator_t::not_between;
        case 2:
            return ss::condition_operator_t::equal;
        case 3:
            return ss::condition_operator_t::not_equal;
        case 4:
            return ss::condition_operator_t::greater;
        case 5:
            return ss::condition_operator_t::less;
        case 6:
            return ss::condition_operator_t::greater_equal;
        case 7:
            return ss::condition_operator_t::less_equal;
        case 8:
            return ss::condition_operator_t::expression;
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
            break;
        case 16:
            return ss::condition_operator_t::contains;
        case 17:
            return ss::condition_operator_t::not_contains;
        case 18:
            return ss::condition_operator_t::begins_with;
        case 19:
            break;
        case 20:
            return ss::condition_operator_t::ends_with;
        case 21:
            break;
        case 22:
            return ss::condition_operator_t::contains_error;
        case 23:
            return ss::condition_operator_t::contains_no_error;
        default:
            break;
    }
    return ss::condition_operator_t::unknown;
}

} // anonymous namespace

gnumeric_sheet_context::gnumeric_sheet_context(
    session_context& session_cxt, const tokens& tokens, ss::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory),
    m_cxt_cell(session_cxt, tokens, factory),
    m_cxt_filter(session_cxt, tokens, factory),
    m_cxt_names(session_cxt, tokens, factory),
    m_cxt_styles(session_cxt, tokens, factory)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_gnumeric_gnm, XML_Sheet }, // root element
        { NS_gnumeric_gnm, XML_Cells, NS_gnumeric_gnm, XML_Cell },
        { NS_gnumeric_gnm, XML_Cols, NS_gnumeric_gnm, XML_ColInfo },
        { NS_gnumeric_gnm, XML_MergedRegions, NS_gnumeric_gnm, XML_Merge },
        { NS_gnumeric_gnm, XML_Rows, NS_gnumeric_gnm, XML_RowInfo },
        { NS_gnumeric_gnm, XML_Sheet, NS_gnumeric_gnm, XML_Cells },
        { NS_gnumeric_gnm, XML_Sheet, NS_gnumeric_gnm, XML_Cols },
        { NS_gnumeric_gnm, XML_Sheet, NS_gnumeric_gnm, XML_Filters },
        { NS_gnumeric_gnm, XML_Sheet, NS_gnumeric_gnm, XML_MergedRegions },
        { NS_gnumeric_gnm, XML_Sheet, NS_gnumeric_gnm, XML_Name },
        { NS_gnumeric_gnm, XML_Sheet, NS_gnumeric_gnm, XML_Names },
        { NS_gnumeric_gnm, XML_Sheet, NS_gnumeric_gnm, XML_Rows },
        { NS_gnumeric_gnm, XML_Sheet, NS_gnumeric_gnm, XML_Selections },
        { NS_gnumeric_gnm, XML_Sheet, NS_gnumeric_gnm, XML_SheetLayout },
        { NS_gnumeric_gnm, XML_Sheet, NS_gnumeric_gnm, XML_Styles },
    };

    init_element_validator(rules, std::size(rules));

    register_child(&m_cxt_cell);
    register_child(&m_cxt_filter);
    register_child(&m_cxt_names);
    register_child(&m_cxt_styles);
}

gnumeric_sheet_context::~gnumeric_sheet_context() = default;

xml_context_base* gnumeric_sheet_context::create_child_context(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_Cells:
            {
                m_cxt_cell.reset(mp_sheet);
                return &m_cxt_cell;
            }
            case XML_Filter:
            {
                m_cxt_filter.reset(mp_sheet);
                return &m_cxt_filter;
            }
            case XML_Names:
            {
                m_cxt_names.reset();
                return &m_cxt_names;
            }
            case XML_Styles:
            {
                m_cxt_styles.reset(m_sheet);
                return &m_cxt_styles;
            }
        }
    }

    return nullptr;
}

void gnumeric_sheet_context::end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child)
{
    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_Names:
            {
                assert(child == &m_cxt_names);
                end_names();
                break;
            }
            case XML_Styles:
            {
                assert(child == &m_cxt_styles);
                end_styles();
                break;
            }
        }
    }
}

void gnumeric_sheet_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
{
    push_stack(ns, name);

    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_Font:
                start_font(attrs);
                break;
            case XML_Merge:
                m_merge_area = std::string_view{};
                break;
            case XML_Name:
                start_name(attrs);
                break;
            case XML_Style:
                start_style(attrs);
                break;
            case XML_StyleRegion:
                start_style_region(attrs);
                break;
            case XML_ColInfo:
                start_col(attrs);
                break;
            case XML_RowInfo:
                start_row(attrs);
                break;
            case XML_Condition:
            {
                if (!m_region_data->contains_conditional_format)
                {
                    m_region_data->contains_conditional_format = true;
                    end_style(false);
                }
                start_condition(attrs);
                break;
            }
            case XML_Expression0:
            case XML_Expression1:
                break;
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool gnumeric_sheet_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_gnumeric_gnm)
    {
        switch(name)
        {
            case XML_Font:
                end_font();
                break;
            case XML_Merge:
                end_merge();
                break;
            case XML_Name:
                end_name();
                break;
            case XML_Style:
            {
                xml_token_pair_t parent = get_parent_element();
                if (parent.second == XML_Condition)
                {
                    end_style(true);
                }
                else
                {
                    // The conditional format entry contains a mandatory style element
                    // Therefore when we have a conditional format the end_style method
                    // is already called in start_element of the XML_Condition case.
                    if (!m_region_data->contains_conditional_format)
                    {
                        end_style(false);
                    }
                }
                break;
            }
            case XML_StyleRegion:
                end_style_region();
                break;
            case XML_Condition:
                end_condition();
                break;
            case XML_Expression0:
            case XML_Expression1:
                end_expression();
                break;
        }
    }

    return pop_stack(ns, name);
}

void gnumeric_sheet_context::characters(std::string_view str, bool transient)
{
    const auto [ns, name] = get_current_element();

    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_Merge:
            {
                m_merge_area = transient ? intern(str) : str;
                break;
            }
            case XML_Name:
            {
                m_name = transient ? intern(str) : str;
                break;
            }
            default:
                m_chars = transient ? intern(str) : str;
        }
    }
    else
    {
        m_chars = transient ? intern(str) : str;
    }
}

void gnumeric_sheet_context::reset(ss::sheet_t sheet)
{
    mp_sheet = nullptr;
    mp_xf = nullptr;
    m_sheet = sheet;

    m_region_data.reset();

    m_front_color.red = 0;
    m_front_color.green = 0;
    m_front_color.blue = 0;

    m_chars = std::string_view{};
    m_name = std::string_view{};
    m_merge_area = std::string_view{};
}

std::vector<gnumeric_style> gnumeric_sheet_context::pop_styles()
{
    return std::move(m_styles);
}

void gnumeric_sheet_context::start_font(const xml_token_attrs_t& attrs)
{
    auto* styles = mp_factory->get_styles();
    if (!styles)
        return;

    auto* font_style = styles->start_font_style();
    ENSURE_INTERFACE(font_style, import_font_style);

    for (const auto& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_Unit:
            {
                double n = atoi(attr.value.data());
                font_style->set_size(n);
                break;
            }
            case XML_Bold:
            {
                bool b = atoi(attr.value.data()) != 0;
                font_style->set_bold(b);
                break;
            }
            case XML_Italic:
            {
                bool b = atoi(attr.value.data()) != 0;
                font_style->set_italic(b);
                break;
            }
            case XML_Underline:
            {
                int n = atoi(attr.value.data());
                switch (n)
                {
                    case 0:
                        font_style->set_underline(ss::underline_style_t::none);
                        break;
                    case 1:
                        font_style->set_underline(ss::underline_style_t::single_line);
                        break;
                    case 2:
                        font_style->set_underline(ss::underline_style_t::double_line);
                        break;
                }
                break;
            }
        }
    }
}

void gnumeric_sheet_context::start_col(const xml_token_attrs_t& attrs)
{
    if (!mp_sheet)
        return;

    ss::iface::import_sheet_properties* sheet_props = mp_sheet->get_sheet_properties();
    if (!sheet_props)
        return;

    long col = 0;
    long col_span = 1;
    double col_width = 0.0;
    bool col_hidden = false;

    for (const xml_token_attr_t& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_No:
            {
                col = to_long(attr.value);
                break;
            }
            case XML_Unit:
            {
                col_width = to_double(attr.value);
                break;
            }
            case XML_Count:
            {
                col_span = to_long(attr.value);
                break;
            }
            case XML_Hidden:
            {
                col_hidden = to_bool(attr.value);
                break;
            }
        }
    }

    sheet_props->set_column_width(col, col_span, col_width, length_unit_t::point);
    sheet_props->set_column_hidden(col, col_span, col_hidden);
}

void gnumeric_sheet_context::start_row(const xml_token_attrs_t& attrs)
{
    if (!mp_sheet)
        return;

    ss::iface::import_sheet_properties* sheet_props = mp_sheet->get_sheet_properties();
    if (!sheet_props)
        return;

    long row = 0;
    long row_span = 1;
    double row_height = 0.0;
    bool row_hidden = false;

    for (const xml_token_attr_t& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_No:
            {
                row = to_long(attr.value);
                break;
            }
            case XML_Unit:
            {
                row_height = to_double(attr.value);
                break;
            }
            case XML_Count:
            {
                row_span = to_long(attr.value);
                break;
            }
            case XML_Hidden:
            {
                row_hidden = to_bool(attr.value);
                break;
            }
        }
    }

    sheet_props->set_row_height(row, row_span, row_height, length_unit_t::point);
    sheet_props->set_row_hidden(row, row_span, row_hidden);
}

void gnumeric_sheet_context::start_style(const xml_token_attrs_t& attrs)
{
    auto* styles = mp_factory->get_styles();
    if (!styles)
        return;

    auto* fill_style = styles->start_fill_style();
    ENSURE_INTERFACE(fill_style, import_fill_style);

    auto* cell_protection = styles->start_cell_protection();
    ENSURE_INTERFACE(cell_protection, import_cell_protection);

    auto* number_format = styles->start_number_format();
    ENSURE_INTERFACE(number_format, import_number_format);

    mp_xf = styles->start_xf(ss::xf_category_t::cell);
    ENSURE_INTERFACE(mp_xf, import_xf);

    bool fill_set = false;
    bool protection_set = false;

    for (const xml_token_attr_t& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_Fore:
            {
                auto color = parse_gnumeric_rgb(attr.value);
                if (!color)
                    break;

                fill_style->set_fg_color(255, color->red, color->green, color->blue);
                fill_set = true;
                m_front_color = *color;
                break;
            }
            case XML_Back:
            {
                auto color = parse_gnumeric_rgb(attr.value);
                if (!color)
                    break;

                fill_style->set_bg_color(255, color->red, color->green, color->blue);
                fill_set = true;
                break;
            }
            case XML_Hidden:
            {
                bool b = atoi(attr.value.data());
                cell_protection->set_hidden(b);

                protection_set = true;
                break;
            }
            case XML_Locked:
            {
                bool b = atoi(attr.value.data());
                cell_protection->set_locked(b);

                protection_set = true;
                break;
            }
            case XML_Format:
            {
                if (attr.value != "General")
                {
                    number_format->set_code(attr.value);
                    std::size_t index = number_format->commit();
                    mp_xf->set_number_format(index);
                }
                break;
            }
            case XML_HAlign:
            {
                ss::hor_alignment_t hor_alignment = ss::hor_alignment_t::unknown;
                if (attr.value == "GNM_HALIGN_CENTER")
                    hor_alignment = ss::hor_alignment_t::center;
                else if (attr.value == "GNM_HALIGN_RIGHT")
                    hor_alignment = ss::hor_alignment_t::right;
                else if (attr.value == "GNM_HALIGN_LEFT")
                    hor_alignment = ss::hor_alignment_t::left;
                else if (attr.value == "GNM_HALIGN_JUSTIFY")
                    hor_alignment = ss::hor_alignment_t::justified;
                else if (attr.value == "GNM_HALIGN_DISTRIBUTED")
                    hor_alignment = ss::hor_alignment_t::distributed;
                else if (attr.value == "GNM_HALIGN_FILL")
                    hor_alignment = ss::hor_alignment_t::filled;

                if (hor_alignment != ss::hor_alignment_t::unknown)
                    mp_xf->set_apply_alignment(true);
                mp_xf->set_horizontal_alignment(hor_alignment);
                break;
            }
            case XML_VAlign:
            {
                ss::ver_alignment_t ver_alignment = ss::ver_alignment_t::unknown;
                if (attr.value == "GNM_VALIGN_BOTTOM")
                    ver_alignment = ss::ver_alignment_t::bottom;
                else if (attr.value == "GNM_VALIGN_TOP")
                    ver_alignment = ss::ver_alignment_t::top;
                else if (attr.value == "GNM_VALIGN_CENTER")
                    ver_alignment = ss::ver_alignment_t::middle;
                else if (attr.value == "GNM_VALIGN_JUSTIFY")
                    ver_alignment = ss::ver_alignment_t::justified;
                else if (attr.value == "GNM_VALIGN_DISTRIBUTED")
                    ver_alignment = ss::ver_alignment_t::distributed;

                if (ver_alignment != ss::ver_alignment_t::unknown)
                    mp_xf->set_apply_alignment(true);
                mp_xf->set_vertical_alignment(ver_alignment);
                break;
            }
        }
    }

    if (fill_set)
    {
        size_t fill_id = fill_style->commit();
        mp_xf->set_fill(fill_id);
    }
    if (protection_set)
    {
        size_t protection_id = cell_protection->commit();
        mp_xf->set_protection(protection_id);
    }
}

void gnumeric_sheet_context::start_style_region(const xml_token_attrs_t& attrs)
{
    m_region_data = style_region{};

    for (const xml_token_attr_t& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_startCol:
            {
                size_t n = atoi(attr.value.data());
                m_region_data->start_col = n;
                break;
            }
            case XML_startRow:
            {
                size_t n = atoi(attr.value.data());
                m_region_data->start_row = n;
                break;
            }
            case XML_endCol:
            {
                size_t n = atoi(attr.value.data());
                m_region_data->end_col = n;
                break;
            }
            case XML_endRow:
            {
                size_t n = atoi(attr.value.data());
                m_region_data->end_row = n;
                break;
            }
            default:
                ;
        }
    }
}

void gnumeric_sheet_context::start_condition(const xml_token_attrs_t& attrs)
{
    if (!mp_sheet)
        return;

    ss::iface::import_conditional_format* cond_format =
        mp_sheet->get_conditional_format();

    if (!cond_format)
        return;

    for (const xml_token_attr_t& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_Operator:
            {
                int val = atoi(attr.value.data());
                ss::condition_operator_t op = get_condition_operator(val);
                cond_format->set_operator(op);
                break;
            }
        }
    }
}

void gnumeric_sheet_context::start_name(const xml_token_attrs_t& /*attrs*/)
{
    m_name = std::string_view{};
}

void gnumeric_sheet_context::end_font()
{
    ss::iface::import_styles* styles = mp_factory->get_styles();
    if (!styles)
        return;

    auto* font_style = styles->start_font_style();
    ENSURE_INTERFACE(font_style, import_font_style);

    font_style->set_color(0, m_front_color.red, m_front_color.green, m_front_color.blue);
    font_style->set_name(m_chars);
    size_t font_id = font_style->commit();

    assert(mp_xf);
    mp_xf->set_font(font_id);
}

void gnumeric_sheet_context::end_style(bool conditional_format)
{
    ss::iface::import_styles* styles = mp_factory->get_styles();
    if (!styles)
        return;

    assert(mp_xf);
    size_t xf_id = mp_xf->commit();
    if (!conditional_format)
    {
        m_region_data->xf_id = xf_id;
    }
    else if (mp_sheet)
    {
        ss::iface::import_conditional_format* cond_format =
            mp_sheet->get_conditional_format();
        if (cond_format)
        {
            cond_format->set_xf_id(xf_id);
        }
    }
}

void gnumeric_sheet_context::end_style_region()
{
    if (!mp_sheet)
        return;

    mp_sheet->set_format(m_region_data->start_row, m_region_data->start_col,
            m_region_data->end_row, m_region_data->end_col, m_region_data->xf_id);

    if (m_region_data->contains_conditional_format)
    {
        ss::iface::import_conditional_format* cond_format =
            mp_sheet->get_conditional_format();
        if (cond_format)
        {
            cond_format->set_range(m_region_data->start_row, m_region_data->start_col,
                    m_region_data->end_row, m_region_data->end_col);
            cond_format->commit_format();
        }
    }
    m_region_data.reset();
}

void gnumeric_sheet_context::end_condition()
{
    if (!mp_sheet)
        return;

    ss::iface::import_conditional_format* cond_format =
        mp_sheet->get_conditional_format();
    if (cond_format)
    {
        cond_format->commit_entry();
    }
}

void gnumeric_sheet_context::end_expression()
{
    if (!mp_sheet)
        return;

    ss::iface::import_conditional_format* cond_format =
        mp_sheet->get_conditional_format();
    if (cond_format)
    {
        cond_format->set_formula(m_chars);
        cond_format->commit_condition();
    }
}

void gnumeric_sheet_context::end_merge()
{
    if (!mp_sheet || m_merge_area.empty())
        return;

    auto* sp = mp_sheet->get_sheet_properties();
    if (!sp)
        return;

    auto* resolver = mp_factory->get_reference_resolver(ss::formula_ref_context_t::global);
    if (!resolver)
        return;

    try
    {
        ss::range_t range = to_rc_range(resolver->resolve_range(m_merge_area));
        sp->set_merge_cell_range(range);
    }
    catch (const invalid_arg_error& e)
    {
        std::ostringstream os;
        os << "failed to parse a merged area '" << m_merge_area << "': " << e.what();
        warn(os.str());
    }
}

void gnumeric_sheet_context::end_name()
{
    if (m_name.empty())
        return;

    mp_sheet = mp_factory->get_sheet(m_name);
    m_name = std::string_view{};
}

void gnumeric_sheet_context::end_names()
{
    if (!mp_sheet)
        return;

    ss::iface::import_named_expression* named_exp = mp_sheet->get_named_expression();
    if (!named_exp)
        return;

    for (const auto& name : m_cxt_names.get_names())
    {
        try
        {
            named_exp->set_base_position(name.position);
            named_exp->set_named_expression(name.name, name.value);
            named_exp->commit();
        }
        catch (const std::exception& e)
        {
            std::ostringstream os;
            os << "failed to commit a named expression named '" << name.name
                << "': (reason='" << e.what() << "'; value='" << name.value << "')";
            warn(os.str());
        }
    }
}

void gnumeric_sheet_context::end_styles()
{
    m_styles = m_cxt_styles.pop_styles();
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
