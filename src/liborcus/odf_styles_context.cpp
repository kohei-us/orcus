/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "odf_styles_context.hpp"
#include "odf_namespace_types.hpp"
#include "odf_token_constants.hpp"
#include "ods_session_data.hpp"
#include "impl_utils.hpp"

#include <orcus/spreadsheet/import_interface_styles.hpp>
#include <iostream>

namespace ss = orcus::spreadsheet;

namespace orcus {

styles_context::styles_context(
    session_context& session_cxt, const tokens& tk,
    spreadsheet::iface::import_styles* iface_styles) :
    xml_context_base(session_cxt, tk),
    mp_styles(iface_styles),
    m_automatic_styles(false),
    m_cxt_style(session_cxt, tk, mp_styles),
    m_cxt_number_format(session_cxt, tk, mp_styles),
    m_cxt_number_style(session_cxt, tk, mp_styles)
{
    register_child(&m_cxt_style);
    register_child(&m_cxt_number_format);
    register_child(&m_cxt_number_style);

    commit_default_styles();
}

xml_context_base* styles_context::create_child_context(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_odf_number)
    {
        if (name == XML_number_style)
        {
            m_cxt_number_style.reset();
            return &m_cxt_number_style;
        }

        m_cxt_number_format.reset();
        return &m_cxt_number_format;
    }

    if (ns == NS_odf_style && name == XML_style)
    {
        m_cxt_style.reset();
        return &m_cxt_style;
    }

    return nullptr;
}

void styles_context::end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child)
{
    if (ns == NS_odf_number)
    {
        switch (name)
        {
            case XML_number_style:
            {
                assert(child == &m_cxt_number_style);
                push_number_style(m_cxt_number_style.pop_style());
                break;
            }
            default:;
        }
    }
    else if (ns == NS_odf_style && name == XML_style)
    {
        assert(child == &m_cxt_style);
        std::unique_ptr<odf_style> current_style = m_cxt_style.pop_style();

        if (mp_styles && current_style->family == style_family_table_cell)
        {
            auto& cell = std::get<odf_style::cell>(current_style->data);

            if (m_automatic_styles)
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

                cell_style->set_name(current_style->name);
                cell_style->set_xf(style_xf_id);
                cell_style->set_parent_name(current_style->parent_name);

                cell.xf = cell_style->commit();
            }
        }

        std::string_view style_name = current_style->name;
        m_styles.emplace(style_name, std::move(current_style));

        return;
    }
}

void styles_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_attrs_t& /*attrs*/)
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
    else
        warn_unhandled();
}

bool styles_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    return pop_stack(ns, name);
}

void styles_context::characters(std::string_view /*str*/, bool /*transient*/)
{
}

void styles_context::reset()
{
    m_styles.clear();
}

odf_styles_map_type styles_context::pop_styles()
{
    return std::move(m_styles);
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

void styles_context::push_number_style(std::unique_ptr<odf_number_format> num_style)
{
    if (!mp_styles)
        return;

    if (num_style->code.empty())
        return;

    auto* number_format = mp_styles->get_number_format();
    ENSURE_INTERFACE(number_format, import_number_format);

    number_format->set_code(num_style->code);
    std::size_t id = number_format->commit();

    if (get_config().debug)
    {
        std::cout << "number-style: name='" << num_style->name
            << "'; code='" << num_style->code
            << "'; id=" << id << std::endl;
    }

    auto& sess_cxt = get_session_context();
    auto& numfmts_store = sess_cxt.get_data<ods_session_data>().number_formats;

    if (auto res = numfmts_store.name2id_map.insert_or_assign(sess_cxt.intern(num_style->name), id); !res.second)
    {
        std::ostringstream os;
        os << "number style named '" << num_style->name << "' has been overwritten.";
        warn(os.str());
    }

    if (auto res = numfmts_store.id2code_map.insert_or_assign(id, std::move(num_style->code)); !res.second)
    {
        std::ostringstream os;
        os << "number style associated with the id of " << id << " has been overwritten.";
        warn(os.str());
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
