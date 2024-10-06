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
class import_sheet;
class import_auto_filter;
class import_auto_filter_node;

}}

class gnumeric_filter_context : public xml_context_base
{
public:
    gnumeric_filter_context(
        session_context& session_cxt, const tokens& tokens,
        spreadsheet::iface::import_factory* factory);
    virtual ~gnumeric_filter_context() override;

    virtual void start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs) override;

    virtual bool end_element(xmlns_id_t ns, xml_token_t name) override;

    void reset(spreadsheet::iface::import_sheet* sheet);

private:
    void start_filter(const xml_token_attrs_t& attrs);
    void start_field(const xml_token_attrs_t& attrs);

    void end_filter();
    void end_field();

    void push_field_expression(
        spreadsheet::col_t field,
        spreadsheet::auto_filter_op_t op,
        long value_type,
        std::string_view value);

private:
    spreadsheet::iface::import_factory* mp_factory = nullptr;
    spreadsheet::iface::import_sheet* mp_sheet = nullptr;
    spreadsheet::iface::import_auto_filter* mp_auto_filter = nullptr;
    spreadsheet::iface::import_auto_filter_node* mp_node = nullptr;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
