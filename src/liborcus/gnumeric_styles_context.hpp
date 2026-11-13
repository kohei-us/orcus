/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "xml_context_base.hpp"
#include "gnumeric_types.hpp"

#include <orcus/spreadsheet/types.hpp>

namespace orcus {

namespace spreadsheet { namespace iface {

class import_factory;

}}

class gnumeric_styles_context : public xml_context_base
{
public:
    gnumeric_styles_context(
        session_context& session_cxt, const tokens& tokens,
        spreadsheet::iface::import_factory* factory);
    virtual ~gnumeric_styles_context() override;

    virtual void start_element(
        xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs) override;

    virtual bool end_element(xmlns_id_t ns, xml_token_t name) override;

    virtual void characters(std::string_view str, bool transient) override;

    void reset(spreadsheet::sheet_t sheet);

    std::vector<gnumeric_style> pop_styles();

private:
    void start_style_region(const std::vector<xml_token_attr_t>& attrs);
    void start_style(const std::vector<xml_token_attr_t>& attrs);
    void start_font(const std::vector<xml_token_attr_t>& attrs);
    void start_condition(const std::vector<xml_token_attr_t>& attrs);

    void end_style_region();
    void end_expression();

    /**
     * Get the style that the style attributes and child elements currently
     * being parsed belong to.  It is the style of the current condition when
     * a condition is being parsed, else the style of the current region.
     */
    gnumeric_style& get_target_style();

private:
    spreadsheet::iface::import_factory* mp_factory = nullptr;
    spreadsheet::sheet_t m_sheet = -1;

    std::vector<gnumeric_style> m_styles;
    gnumeric_style m_current_style;
    bool m_in_condition = false;

    std::string_view m_chars;
};

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
