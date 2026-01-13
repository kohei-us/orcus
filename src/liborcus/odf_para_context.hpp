/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "xml_context_base.hpp"
#include "odf_styles.hpp"
#include "odf_text_para.hpp"

#include <vector>

namespace orcus {

namespace spreadsheet { namespace iface { class import_shared_strings; }}

/**
 * This class handles <text:p> contexts.
 */
class text_para_context : public xml_context_base
{
public:
    text_para_context(
       session_context& session_cxt, const tokens& tokens, odf_styles_map_type& styles);
    virtual ~text_para_context();

    virtual void start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs);
    virtual bool end_element(xmlns_id_t ns, xml_token_t name);
    virtual void characters(std::string_view str, bool transient);

    void reset();

    odf_text_paragraph pop_paragraph();
    bool empty() const;

private:
    void flush_segment();

private:
    odf_styles_map_type& m_styles;
    odf_text_paragraph m_paragraph;

    std::vector<std::string_view> m_span_stack; /// stack of text spans with style names
    std::vector<std::string_view> m_fragments; /// raw text fragments
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
