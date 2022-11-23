/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gnumeric_context.hpp"
#include "gnumeric_token_constants.hpp"
#include "gnumeric_namespace_types.hpp"
#include "gnumeric_sheet_context.hpp"
#include "orcus/spreadsheet/import_interface.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

namespace orcus {

namespace {

}

gnumeric_content_xml_context::gnumeric_content_xml_context(
    session_context& session_cxt, const tokens& tokens, spreadsheet::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory),
    m_sheet_count(0),
    m_cxt_sheet(session_cxt, tokens, factory)
{
    register_child(&m_cxt_sheet);
}

gnumeric_content_xml_context::~gnumeric_content_xml_context()
{
}

xml_context_base* gnumeric_content_xml_context::create_child_context(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_gnumeric_gnm && name == XML_Sheet)
    {
        m_cxt_sheet.reset(m_sheet_count++);
        return &m_cxt_sheet;
    }

    return nullptr;
}

void gnumeric_content_xml_context::end_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/, xml_context_base* /*child*/)
{
}

void gnumeric_content_xml_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& /*attrs*/)
{
    push_stack(ns, name);

    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool gnumeric_content_xml_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {

        }
    }
    return pop_stack(ns, name);
}

void gnumeric_content_xml_context::characters(std::string_view /*str*/, bool /*transient*/)
{
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
