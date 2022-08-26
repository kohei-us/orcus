/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "odf_document_styles_context.hpp"
#include "odf_token_constants.hpp"
#include "odf_namespace_types.hpp"

#include <orcus/spreadsheet/import_interface_styles.hpp>

#include <iostream>

namespace ss = orcus::spreadsheet;

namespace orcus {

document_styles_context::document_styles_context(session_context& session_cxt, const tokens& tk, odf_styles_map_type& styles_map, ss::iface::import_styles* xstyles) :
    xml_context_base(session_cxt, tk),
    m_styles_map(styles_map),
    mp_styles(xstyles),
    m_cxt_styles(session_cxt, tk, styles_map, xstyles)
{
}

xml_context_base* document_styles_context::create_child_context(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_odf_office && name == XML_styles)
    {
        m_cxt_styles.transfer_common(*this);
        return &m_cxt_styles;
    }

    return nullptr;
}

void document_styles_context::end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child)
{
    if (ns == NS_odf_office && name == XML_styles)
    {
        assert(child == &m_cxt_styles);
    }
}

void document_styles_context::start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs)
{
    auto parent = push_stack(ns, name);
    (void)parent;

    (void)ns;
    (void)name;
    (void)attrs;

    warn_unhandled();
}

bool document_styles_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    return pop_stack(ns, name);
}

void document_styles_context::characters(std::string_view /*str*/, bool /*transient*/)
{
}


} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
