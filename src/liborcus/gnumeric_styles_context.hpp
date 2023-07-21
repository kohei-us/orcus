/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "xml_context_base.hpp"

#include <orcus/spreadsheet/types.hpp>

namespace orcus {

namespace spreadsheet { namespace iface {

class import_factory;

}}

class gnumeric_styles_context : public xml_context_base
{
    struct style
    {
        spreadsheet::range_t region = {{-1, -1}, {-1, -1}};
        spreadsheet::hor_alignment_t hor_align = spreadsheet::hor_alignment_t::unknown;
        spreadsheet::ver_alignment_t ver_align = spreadsheet::ver_alignment_t::unknown;

        bool valid() const;
    };

public:
    gnumeric_styles_context(
        session_context& session_cxt, const tokens& tokens,
        spreadsheet::iface::import_factory* factory);
    virtual ~gnumeric_styles_context() override;

    virtual void start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs) override;
    virtual bool end_element(xmlns_id_t ns, xml_token_t name) override;

    void reset();

private:
    void start_style_region(const std::vector<xml_token_attr_t>& attrs);
    void start_style(const std::vector<xml_token_attr_t>& attrs);

    void end_style_region();

private:
    spreadsheet::iface::import_factory* mp_factory = nullptr;

    std::vector<style> m_styles;
    style m_current_style;
};

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
