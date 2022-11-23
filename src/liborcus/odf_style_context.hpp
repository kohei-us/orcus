/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "xml_context_base.hpp"
#include "odf_styles.hpp"

#include <memory>

namespace orcus {

namespace spreadsheet { namespace iface {
    class import_styles;
}}

/**
 * Context for <style:style> element scope.
 *
 * This context populates one odf_style instance that represents a single set
 * of style properties.
 */
class style_context : public xml_context_base
{
public:
    style_context(session_context& session_cxt, const tokens& tk, spreadsheet::iface::import_styles* iface_styles);

    void start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs) override;
    void characters(std::string_view str, bool transient) override;
    bool end_element(xmlns_id_t ns, xml_token_t name) override;

    void reset();
    std::unique_ptr<odf_style> pop_style();

private:
    void start_paragraph_properties(const xml_token_pair_t& parent, const xml_token_attrs_t& attrs);
    void start_text_properties(const xml_token_pair_t& parent, const xml_token_attrs_t& attrs);
    void start_table_cell_properties(const xml_token_pair_t& parent, const xml_token_attrs_t& attrs);

private:
    spreadsheet::iface::import_styles* mp_styles = nullptr;
    std::unique_ptr<odf_style> m_current_style;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
