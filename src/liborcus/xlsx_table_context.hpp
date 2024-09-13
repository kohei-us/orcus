/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_XLSX_TABLE_CONTEXT_HPP
#define ORCUS_XLSX_TABLE_CONTEXT_HPP

#include "xml_context_base.hpp"
#include "xlsx_autofilter_context.hpp"

namespace orcus {

namespace spreadsheet { namespace iface {

class import_table;
class import_reference_resolver;

}}

class xlsx_table_context : public xml_context_base
{
public:
    xlsx_table_context(
        session_context& session_cxt, const tokens& tokens,
        spreadsheet::iface::import_table& table,
        spreadsheet::iface::import_reference_resolver& resolver);
    virtual ~xlsx_table_context();

    virtual xml_context_base* create_child_context(xmlns_id_t ns, xml_token_t name) override;

    virtual void start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs) override;
    virtual bool end_element(xmlns_id_t ns, xml_token_t name) override;

private:
    void start_element_table(const xml_token_attrs_t& attrs);

private:
    spreadsheet::iface::import_table& m_table;
    spreadsheet::iface::import_reference_resolver& m_resolver;

    xlsx_autofilter_context m_cxt_autofilter;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
