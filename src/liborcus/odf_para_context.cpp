/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "odf_para_context.hpp"
#include "odf_token_constants.hpp"
#include "odf_namespace_types.hpp"
#include "xml_context_global.hpp"

#include "orcus/spreadsheet/import_interface.hpp"
#include "orcus/exception.hpp"

#include <iostream>
#include <cassert>

namespace orcus {

text_para_context::text_para_context(
    session_context& session_cxt, const tokens& tokens,
    spreadsheet::iface::import_shared_strings* ssb, odf_styles_map_type& styles) :
    xml_context_base(session_cxt, tokens),
    mp_sstrings(ssb), m_styles(styles),
    m_string_index(0), m_has_content(false)
{
}

text_para_context::~text_para_context() = default;

xml_context_base* text_para_context::create_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/)
{
    return nullptr;
}

void text_para_context::end_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/, xml_context_base* /*child*/)
{
    // not implemented yet.
}

void text_para_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);
    if (ns == NS_odf_text)
    {
        switch (name)
        {
            case XML_p:
                // paragraph
                xml_element_expected(parent, XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN);
                break;
            case XML_span:
            {
                // text span.
                xml_element_expected(parent, NS_odf_text, XML_p);
                flush_segment();
                std::string_view style_name = get_single_attr(attrs, NS_odf_text, XML_style_name, &m_pool);
                m_span_stack.push_back(style_name);
                break;
            }
            case XML_s:
                // control character.  ignored for now.
                break;
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool text_para_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_odf_text)
    {
        switch (name)
        {
            case XML_p:
            {
                // paragraph
                flush_segment();
                if (mp_sstrings)
                    m_string_index = mp_sstrings->commit_segments();
                break;
            }
            case XML_span:
            {
                // text span.
                if (m_span_stack.empty())
                    throw xml_structure_error("</text:span> encountered without matching opening element.");

                flush_segment();
                m_span_stack.pop_back();
                break;
            }
            default:
                ;
        }
    }
    return pop_stack(ns, name);
}

void text_para_context::characters(std::string_view str, bool transient)
{
    if (transient)
        m_contents.push_back(m_pool.intern(str).first);
    else
        m_contents.push_back(str);
}

void text_para_context::reset()
{
    m_string_index = 0;
    m_has_content = false;
    m_pool.clear();
    m_contents.clear();
}

size_t text_para_context::get_string_index() const
{
    return m_string_index;
}

bool text_para_context::empty() const
{
    return !m_has_content;
}

void text_para_context::flush_segment()
{
    if (m_contents.empty())
        // No content to flush.
        return;

    m_has_content = true;

    const odf_style* style = nullptr;
    if (!m_span_stack.empty())
    {
        std::string_view style_name = m_span_stack.back();
        auto it = m_styles.find({style_family_text, style_name});
        if (it != m_styles.end())
            style = it->second.get();
    }

    if (mp_sstrings)
    {
        if (style && style->family == style_family_text)
        {
            const auto& data = std::get<odf_style::text>(style->data);
            mp_sstrings->set_segment_font(data.font);
        }

        for (std::string_view ps : m_contents)
            mp_sstrings->append_segment(ps);
    }

    m_contents.clear();
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
