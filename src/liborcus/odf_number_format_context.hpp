/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ODF_NUMBER_FORMATTING_CONTEXT_HPP
#define ODF_NUMBER_FORMATTING_CONTEXT_HPP

#include "xml_context_base.hpp"
#include "odf_styles.hpp"

#include "orcus/string_pool.hpp"

namespace orcus {

namespace spreadsheet { namespace iface {
    class import_styles;
}}


/**
 * Context that handles <number:xyz> scope.
 */
class number_format_context : public xml_context_base
{
public:
    number_format_context(
        session_context& session_cxt, const tokens& tk,
        spreadsheet::iface::import_styles* iface_styles);

    virtual xml_context_base* create_child_context(xmlns_id_t ns, xml_token_t name) override;
    virtual void end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child) override;
    virtual void start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs) override;
    virtual bool end_element(xmlns_id_t ns, xml_token_t name) override;
    virtual void characters(std::string_view str, bool transient) override;

    void reset();

private:
    spreadsheet::iface::import_styles* mp_styles;
    odf_number_format m_current_style;

    string_pool m_pool;
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
