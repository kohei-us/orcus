/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_GNUMERICCONTEXT_HPP
#define INCLUDED_ORCUS_GNUMERICCONTEXT_HPP

#include "xml_context_base.hpp"
#include "gnumeric_sheet_context.hpp"
#include "gnumeric_names_context.hpp"
#include "gnumeric_types.hpp"

#include <orcus/spreadsheet/types.hpp>

#include <vector>

namespace orcus {

namespace spreadsheet { namespace iface {

class import_factory;
class import_sheet;

}}

class gnumeric_content_xml_context : public xml_context_base
{
public:
    gnumeric_content_xml_context(
        session_context& session_cxt, const tokens& tokens,
        spreadsheet::iface::import_factory* factory);

    virtual ~gnumeric_content_xml_context() override;

    virtual xml_context_base* create_child_context(xmlns_id_t ns, xml_token_t name) override;
    virtual void end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child) override;
    virtual void start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs) override;
    virtual bool end_element(xmlns_id_t ns, xml_token_t name) override;
    virtual void characters(std::string_view str, bool transient) override;

private:
    void end_names();
    void end_sheet();
    void end_sheets();

private:
    spreadsheet::iface::import_factory* mp_factory;
    spreadsheet::sheet_t m_sheet_pos;

    gnumeric_names_context m_cxt_names;
    gnumeric_sheet_context m_cxt_sheet;

    std::vector<std::vector<gnumeric_style>> m_styles;
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
