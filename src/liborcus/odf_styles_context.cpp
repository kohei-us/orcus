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
    m_cxt_number_style(session_cxt, tk),
    m_cxt_currency_style(session_cxt, tk),
    m_cxt_boolean_style(session_cxt, tk),
    m_cxt_text_style(session_cxt, tk),
    m_cxt_percentage_style(session_cxt, tk),
    m_cxt_date_style(session_cxt, tk),
    m_cxt_time_style(session_cxt, tk)
{
    register_child(&m_cxt_style);
    register_child(&m_cxt_number_style);
    register_child(&m_cxt_currency_style);
    register_child(&m_cxt_boolean_style);
    register_child(&m_cxt_text_style);
    register_child(&m_cxt_percentage_style);
    register_child(&m_cxt_date_style);
    register_child(&m_cxt_time_style);

    commit_default_styles();
}

xml_context_base* styles_context::create_child_context(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_odf_number)
    {
        switch (name)
        {
            case XML_number_style:
            {
                m_cxt_number_style.reset();
                return &m_cxt_number_style;
            }
            case XML_currency_style:
            {
                m_cxt_currency_style.reset();
                return &m_cxt_currency_style;
            }
            case XML_boolean_style:
            {
                m_cxt_boolean_style.reset();
                return &m_cxt_boolean_style;
            }
            case XML_text_style:
            {
                m_cxt_text_style.reset();
                return &m_cxt_text_style;
            }
            case XML_percentage_style:
            {
                m_cxt_percentage_style.reset();
                return &m_cxt_percentage_style;
            }
            case XML_date_style:
            {
                m_cxt_date_style.reset();
                return &m_cxt_date_style;
            }
            case XML_time_style:
            {
                m_cxt_time_style.reset();
                return &m_cxt_time_style;
            }
        }
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
            case XML_currency_style:
            {
                assert(child == &m_cxt_currency_style);
                push_number_style(m_cxt_currency_style.pop_style());
                break;
            }
            case XML_boolean_style:
            {
                assert(child == &m_cxt_boolean_style);
                push_number_style(m_cxt_boolean_style.pop_style());
                break;
            }
            case XML_text_style:
            {
                assert(child == &m_cxt_text_style);
                push_number_style(m_cxt_text_style.pop_style());
                break;
            }
            case XML_percentage_style:
            {
                assert(child == &m_cxt_percentage_style);
                push_number_style(m_cxt_percentage_style.pop_style());
                break;
            }
            case XML_date_style:
            {
                assert(child == &m_cxt_date_style);
                push_number_style(m_cxt_date_style.pop_style());
                break;
            }
            case XML_time_style:
            {
                assert(child == &m_cxt_time_style);
                push_number_style(m_cxt_time_style.pop_style());
                break;
            }
            default:;
        }
    }
    else if (ns == NS_odf_style && name == XML_style)
    {
        assert(child == &m_cxt_style);
        std::unique_ptr<odf_style> current_style = m_cxt_style.pop_style();

        std::optional<std::size_t> parent_xfid = query_parent_style_xfid(current_style->parent_name);

        if (mp_styles && current_style->family == style_family_table_cell)
        {
            auto& cell = std::get<odf_style::cell>(current_style->data);

            if (m_automatic_styles)
            {
                // Import it into the direct cell style store
                auto* xf = mp_styles->start_xf(ss::xf_category_t::cell);
                ENSURE_INTERFACE(xf, import_xf);
                xf->set_font(cell.font);
                xf->set_fill(cell.fill);
                xf->set_border(cell.border);
                xf->set_protection(cell.protection);
                xf->set_number_format(cell.number_format);

                if (cell.hor_align != ss::hor_alignment_t::unknown)
                    xf->set_horizontal_alignment(cell.hor_align);
                if (cell.ver_align != ss::ver_alignment_t::unknown)
                    xf->set_vertical_alignment(cell.ver_align);
                if (cell.wrap_text)
                    xf->set_wrap_text(*cell.wrap_text);
                if (cell.shrink_to_fit)
                    xf->set_shrink_to_fit(*cell.shrink_to_fit);

                if (parent_xfid)
                    xf->set_style_xf(*parent_xfid);

                cell.xf = xf->commit();
            }
            else
            {
                // Import it into the cell style xf store, and reference
                // its index in the cell style name store.
                auto* xf = mp_styles->start_xf(ss::xf_category_t::cell_style);
                ENSURE_INTERFACE(xf, import_xf);
                xf->set_font(cell.font);
                xf->set_fill(cell.fill);
                xf->set_border(cell.border);
                xf->set_protection(cell.protection);
                xf->set_number_format(cell.number_format);

                if (cell.hor_align != ss::hor_alignment_t::unknown)
                    xf->set_horizontal_alignment(cell.hor_align);
                if (cell.ver_align != ss::ver_alignment_t::unknown)
                    xf->set_vertical_alignment(cell.ver_align);
                if (cell.wrap_text)
                    xf->set_wrap_text(*cell.wrap_text);
                if (cell.shrink_to_fit)
                    xf->set_shrink_to_fit(*cell.shrink_to_fit);

                if (parent_xfid)
                    xf->set_style_xf(*parent_xfid);

                size_t style_xf_id = xf->commit();
                cell.xf = style_xf_id;

                auto* cell_style = mp_styles->start_cell_style();
                ENSURE_INTERFACE(cell_style, import_cell_style);

                if (!current_style->display_name.empty())
                    cell_style->set_display_name(current_style->display_name);

                cell_style->set_name(current_style->name);
                cell_style->set_xf(style_xf_id);
                cell_style->set_parent_name(current_style->parent_name);
                cell_style->commit();
            }
        }

        std::string_view style_name = get_session_context().intern(current_style->name);
        m_styles.emplace(style_name, std::move(current_style));
    }
}

void styles_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& /*attrs*/)
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

    auto* font_style = mp_styles->start_font_style();
    ENSURE_INTERFACE(font_style, import_font_style);

    auto* fill_style = mp_styles->start_fill_style();
    ENSURE_INTERFACE(fill_style, import_fill_style);

    auto* border_style = mp_styles->start_border_style();
    ENSURE_INTERFACE(border_style, import_border_style);

    auto* cell_protection = mp_styles->start_cell_protection();
    ENSURE_INTERFACE(cell_protection, import_cell_protection);

    auto* number_format = mp_styles->start_number_format();
    ENSURE_INTERFACE(number_format, import_number_format);

    // Set default styles. Default styles must be associated with an index of 0.
    // Set empty styles for all style types before importing real styles.
    font_style->commit();
    fill_style->commit();
    border_style->commit();
    cell_protection->commit();
    number_format->commit();

    auto* xf = mp_styles->start_xf(ss::xf_category_t::cell);
    ENSURE_INTERFACE(xf, import_xf);
    xf->commit();

    xf = mp_styles->start_xf(ss::xf_category_t::cell_style);
    ENSURE_INTERFACE(xf, import_xf);
    xf->commit();

    auto* cell_style = mp_styles->start_cell_style();
    ENSURE_INTERFACE(cell_style, import_cell_style);
    cell_style->commit();
}

void styles_context::push_number_style(std::unique_ptr<odf_number_format> num_style)
{
    if (!mp_styles)
        return;

    if (num_style->name.empty())
    {
        warn("ignoring a number style with empty name.");
        return;
    }

    if (num_style->code.empty())
    {
        std::ostringstream os;
        os << "number style named '" << num_style->name << "' has empty code.";
        warn(os.str());
        return;
    }

    auto* number_format = mp_styles->start_number_format();
    ENSURE_INTERFACE(number_format, import_number_format);

    number_format->set_code(num_style->code);
    std::size_t id = number_format->commit();

    if (get_config().debug)
    {
        std::cerr << "number-style: name='" << num_style->name
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

std::optional<std::size_t> styles_context::query_parent_style_xfid(std::string_view parent_name) const
{
    std::optional<std::size_t> parent_xfid;

    if (parent_name.empty())
        return parent_xfid;

    const ods_session_data& ods_data = get_session_context().get_data<ods_session_data>();

    auto it = ods_data.styles_map.find(parent_name);
    if (it == ods_data.styles_map.end())
    {
        // Not found in the session store. Check the current styles map too.
        auto it2 = m_styles.find(parent_name);
        if (it2 != m_styles.end())
        {
            const odf_style& s = *it2->second;
            if (s.family == style_family_table_cell)
            {
                const odf_style::cell& c = std::get<odf_style::cell>(s.data);
                parent_xfid = c.xf;
            }
        }
        return parent_xfid;
    }

    const odf_style& s = *it->second;
    if (s.family != style_family_table_cell)
        return parent_xfid;

    const odf_style::cell& c = std::get<odf_style::cell>(s.data);
    parent_xfid = c.xf;

    return parent_xfid;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
