/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "xml_context_base.hpp"
#include "odf_styles.hpp"
#include "odf_styles_context.hpp"

namespace orcus {

namespace spreadsheet { namespace iface {

class import_styles;

}}

/**
 * Context that handles the <office:document-styles> element scope.
 *
 * <office:document-styles> is the root element of styles.xml stream inside an
 * ODF document.
 */
class document_styles_context : public xml_context_base
{
public:
    document_styles_context(
        session_context& session_cxt, const tokens& tk,
        odf_styles_map_type& styles_map, spreadsheet::iface::import_styles* xstyles);

    xml_context_base* create_child_context(xmlns_id_t ns, xml_token_t name) override;
    void end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child) override;
    void start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs) override;
    bool end_element(xmlns_id_t ns, xml_token_t name) override;
    void characters(std::string_view str, bool transient) override;

private:
    odf_styles_map_type& m_styles_map;
    spreadsheet::iface::import_styles* mp_styles = nullptr;

    styles_context m_cxt_styles;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
