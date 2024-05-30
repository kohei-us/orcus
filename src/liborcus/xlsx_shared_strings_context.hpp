/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <orcus/spreadsheet/types.hpp>
#include <orcus/string_pool.hpp>
#include <orcus/cell_buffer.hpp>

#include "xml_context_base.hpp"
#include "xlsx_types.hpp"

namespace orcus {

namespace spreadsheet { namespace iface {
    class import_shared_strings;
}}

/**
 * Context for xl/sharedStrings.xml part.
 */
class xlsx_shared_strings_context : public xml_context_base
{
public:
    xlsx_shared_strings_context(
        session_context& session_cxt, const tokens& tokens,
        spreadsheet::iface::import_shared_strings* strings);
    virtual ~xlsx_shared_strings_context();

    virtual void start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs);
    virtual bool end_element(xmlns_id_t ns, xml_token_t name);
    virtual void characters(std::string_view str, bool transient);

private:
    void start_strike(const xml_token_attrs_t& attrs);
    void start_underline(const xml_token_attrs_t& attrs);

private:
    spreadsheet::iface::import_shared_strings* mp_strings = nullptr;
    string_pool m_pool;
    cell_buffer m_cell_buffer;
    std::string_view m_cur_str;
    bool m_in_segments;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
