/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "odf_para_context.hpp"
#include "odf_token_constants.hpp"
#include "odf_namespace_types.hpp"
#include "session_context.hpp"
#include "xml_context_global.hpp"

#include "orcus/spreadsheet/import_interface.hpp"
#include "orcus/exception.hpp"

#include <iostream>
#include <cassert>

namespace orcus {

text_para_context::text_para_context(
    session_context& session_cxt, const tokens& tokens, odf_styles_map_type& styles) :
    xml_context_base(session_cxt, tokens),
    m_styles(styles)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_odf_text, XML_p }, // root element
        { NS_odf_text, XML_p, NS_odf_text, XML_span },
    };

    init_element_validator(rules, std::size(rules));
}

text_para_context::~text_para_context() = default;

void text_para_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
{
    push_stack(ns, name);
    if (ns == NS_odf_text)
    {
        switch (name)
        {
            case XML_p:
                // paragraph
                break;
            case XML_span:
            {
                // text span.
                flush_segment();
                std::string_view style_name = get_single_attr(attrs, NS_odf_text, XML_style_name, &get_session_context().spool);
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
        m_fragments.push_back(get_session_context().spool.intern(str).first);
    else
        m_fragments.push_back(str);
}

void text_para_context::reset()
{
    m_fragments.clear();
    m_paragraph = odf_text_paragraph{};
}

odf_text_paragraph text_para_context::pop_paragraph()
{
    return std::move(m_paragraph);
}

bool text_para_context::empty() const
{
    return m_paragraph.empty();
}

void text_para_context::flush_segment()
{
    if (m_fragments.empty())
        // No content to flush.
        return;

    const odf_style* style = nullptr;
    if (!m_span_stack.empty())
    {
        std::string_view style_name = m_span_stack.back();
        auto it = m_styles.find({style_family_text, style_name});
        if (it != m_styles.end())
            style = it->second.get();
    }

    m_paragraph.emplace_back();
    auto& segment = m_paragraph.back();

    if (style && style->family == style_family_text)
    {
        const auto& data = std::get<odf_style::text>(style->data);
        segment.font = data.font;
    }

    segment.text_fragments.swap(m_fragments); // clears contents
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
