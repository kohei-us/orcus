/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_ODF_STYLES_CONTEXT_HPP
#define ORCUS_ODF_STYLES_CONTEXT_HPP

#include "xml_context_base.hpp"
#include "odf_styles.hpp"
#include "odf_number_formatting_context.hpp"

#include <orcus/global.hpp>

#include <unordered_map>

namespace orcus {

namespace spreadsheet { namespace iface {
    class import_styles;
}}

/**
 * Context that handles <office:automatic-styles> or <office:styles> scope.
 */
class styles_context : public xml_context_base
{
public:
    styles_context(
        session_context& session_cxt, const tokens& tk, odf_styles_map_type& styles, spreadsheet::iface::import_styles* iface_styles);

    virtual bool can_handle_element(xmlns_id_t ns, xml_token_t name) const override;
    virtual xml_context_base* create_child_context(xmlns_id_t ns, xml_token_t name) override;
    virtual void end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child) override;
    virtual void start_element(xmlns_id_t ns, xml_token_t name, const xml_attrs_t& attrs) override;
    virtual bool end_element(xmlns_id_t ns, xml_token_t name) override;
    virtual void characters(const pstring& str, bool transient) override;

private:
    void start_text_properties(const xml_token_pair_t& parent, const xml_attrs_t& attrs);
    void start_table_cell_properties(const xml_token_pair_t& parent, const xml_attrs_t& attrs);

    void commit_default_styles();

private:
    spreadsheet::iface::import_styles* mp_styles;
    odf_styles_map_type& m_styles;

    std::unique_ptr<odf_style> m_current_style;

    // an automatic style corresponds to a cell format and not a real style
    bool m_automatic_styles;

    number_formatting_context m_cxt_number_format;
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
