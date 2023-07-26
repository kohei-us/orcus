/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gnumeric_context.hpp"
#include "gnumeric_token_constants.hpp"
#include "gnumeric_namespace_types.hpp"
#include "gnumeric_sheet_context.hpp"
#include "impl_utils.hpp"

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/spreadsheet/import_interface_styles.hpp>

#include <fstream>
#include <algorithm>

namespace ss = orcus::spreadsheet;

namespace orcus {

gnumeric_content_xml_context::gnumeric_content_xml_context(
    session_context& session_cxt, const tokens& tokens, spreadsheet::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory),
    m_sheet_pos(0),
    m_cxt_names(session_cxt, tokens, factory),
    m_cxt_sheet(session_cxt, tokens, factory)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_gnumeric_gnm, XML_Workbook }, // root element
        { NS_gnumeric_gnm, XML_SheetNameIndex, NS_gnumeric_gnm, XML_SheetName },
        { NS_gnumeric_gnm, XML_Sheets, NS_gnumeric_gnm, XML_Sheet },
        { NS_gnumeric_gnm, XML_Workbook, NS_gnumeric_gnm, XML_Names },
        { NS_gnumeric_gnm, XML_Workbook, NS_gnumeric_gnm, XML_SheetNameIndex },
        { NS_gnumeric_gnm, XML_Workbook, NS_gnumeric_gnm, XML_Sheets },
    };

    init_element_validator(rules, std::size(rules));

    register_child(&m_cxt_names);
    register_child(&m_cxt_sheet);
}

gnumeric_content_xml_context::~gnumeric_content_xml_context() = default;

xml_context_base* gnumeric_content_xml_context::create_child_context(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_Sheet:
            {
                m_cxt_sheet.reset(m_sheet_pos++);
                return &m_cxt_sheet;
            }
            case XML_Names:
            {
                m_cxt_names.reset();
                return &m_cxt_names;
            }
        }
    }

    return nullptr;
}

void gnumeric_content_xml_context::end_child_context(
    xmlns_id_t ns, xml_token_t name, xml_context_base* child)
{
    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_Sheet:
            {
                assert(child == &m_cxt_sheet);
                end_sheet();
                break;
            }
            case XML_Names:
            {
                assert(child == &m_cxt_names);
                end_names();
                break;
            }
        }
    }
}

void gnumeric_content_xml_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& /*attrs*/)
{
    push_stack(ns, name);

    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_SheetNameIndex:
                m_sheet_pos = 0;
                break;
            case XML_SheetName:
                break;
            case XML_Sheets:
                m_sheet_pos = 0;
                break;
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool gnumeric_content_xml_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_Sheets:
                end_sheets();
                break;
        }
    }
    return pop_stack(ns, name);
}

void gnumeric_content_xml_context::characters(std::string_view str, bool /*transient*/)
{
    if (str.empty())
        return;

    auto current = get_current_element();

    if (current.first == NS_gnumeric_gnm)
    {
        switch (current.second)
        {
            case XML_SheetName:
            {
                auto* p = mp_factory->append_sheet(m_sheet_pos++, str);
                if (!p)
                {
                    std::ostringstream os;
                    os << "failed to append a new sheet named '" << str << "'";
                    warn(os.str());
                }

                break;
            }
        }
    }
}

void gnumeric_content_xml_context::end_names()
{
    ss::iface::import_named_expression* named_exp = mp_factory->get_named_expression();
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

void gnumeric_content_xml_context::end_sheet()
{
    m_styles.push_back(m_cxt_sheet.pop_styles());
}

void gnumeric_content_xml_context::end_sheets()
{
    import_styles();
}

void gnumeric_content_xml_context::import_styles()
{
    ss::iface::import_styles* istyles = mp_factory->get_styles();
    if (!istyles)
        return;

    std::size_t xf_count = 1; // one for the default style
    for (const auto& sheet_styles : m_styles)
        xf_count += sheet_styles.size();

    istyles->set_xf_count(ss::xf_category_t::cell, xf_count);

    import_default_styles(istyles);
    import_cell_styles(istyles);
}

void gnumeric_content_xml_context::import_default_styles(ss::iface::import_styles* istyles)
{
    assert(istyles);

    ss::iface::import_font_style* ifont = istyles->start_font_style();
    ENSURE_INTERFACE(ifont, imort_font_style);
    std::size_t id = ifont->commit();
    assert(id == 0);

    ss::iface::import_fill_style* ifill = istyles->start_fill_style();
    ENSURE_INTERFACE(ifill, imort_fill_style);
    id = ifill->commit();
    assert(id == 0);

    ss::iface::import_border_style* iborder = istyles->start_border_style();
    ENSURE_INTERFACE(iborder, imort_border_style);
    id = iborder->commit();
    assert(id == 0);

    ss::iface::import_cell_protection* iprotection = istyles->start_cell_protection();
    ENSURE_INTERFACE(iprotection, imort_cell_protection);
    id = iprotection->commit();
    assert(id == 0);

    ss::iface::import_number_format* inumfmt = istyles->start_number_format();
    ENSURE_INTERFACE(inumfmt, import_number_format);
    id = inumfmt->commit();
    assert(id == 0);

    ss::iface::import_xf* ixf = istyles->start_xf(ss::xf_category_t::cell);
    ENSURE_INTERFACE(ixf, import_xf);
    id = ixf->commit();
    assert(id == 0);

    ss::iface::import_cell_style* xstyle = istyles->start_cell_style();
    ENSURE_INTERFACE(xstyle, import_cell_style);
    xstyle->set_xf(0);
    xstyle->commit();
}

void gnumeric_content_xml_context::import_cell_styles(ss::iface::import_styles* istyles)
{
    assert(istyles);

    for (const auto& sheet_styles : m_styles)
    {
        for (const gnumeric_style& style : sheet_styles)
        {
            assert(style.valid()); // should be validated before insertion

            ss::iface::import_sheet* isheet = mp_factory->get_sheet(style.sheet);
            if (!isheet)
                continue;

            ss::iface::import_font_style* ifont = istyles->start_font_style();
            ENSURE_INTERFACE(ifont, import_font_style);

            if (style.fore)
                ifont->set_color(255, style.fore->red, style.fore->green, style.fore->blue);

            std::size_t id_font = ifont->commit();

            std::optional<std::size_t> id_fill;

            if (style.pattern != ss::fill_pattern_t::none)
            {
                ss::iface::import_fill_style* ifill = istyles->start_fill_style();
                ENSURE_INTERFACE(ifill, import_fill_style);

                ifill->set_pattern_type(style.pattern);

                if (style.back)
                    ifill->set_fg_color(
                        255,
                        style.back->red,
                        style.back->green,
                        style.back->blue);

                if (style.pattern_color)
                    ifill->set_bg_color(
                        255,
                        style.pattern_color->red,
                        style.pattern_color->green,
                        style.pattern_color->blue);

                id_fill = ifill->commit();
            }

            ss::iface::import_xf* ixf = istyles->start_xf(ss::xf_category_t::cell);
            ENSURE_INTERFACE(ixf, import_xf);

            ixf->set_font(id_font);

            if (id_fill)
                ixf->set_fill(*id_fill);

            bool apply_alignment =
                style.hor_align != ss::hor_alignment_t::unknown ||
                style.ver_align != ss::ver_alignment_t::unknown ||
                style.wrap_text;

            ixf->set_apply_alignment(apply_alignment);

            if (style.hor_align != ss::hor_alignment_t::unknown)
                ixf->set_horizontal_alignment(style.hor_align);

            if (style.ver_align != ss::ver_alignment_t::unknown)
                ixf->set_vertical_alignment(style.ver_align);

            if (style.wrap_text)
                ixf->set_wrap_text(*style.wrap_text);

            std::size_t xfid = ixf->commit();
            isheet->set_format(
                style.region.first.row, style.region.first.column,
                style.region.last.row, style.region.last.column,
                xfid);
        }
    }
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
