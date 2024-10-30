/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "xml_context_base.hpp"
#include <orcus/spreadsheet/types.hpp>

#include <optional>

namespace orcus {

namespace spreadsheet { namespace iface {

class import_factory;
class import_table;
class import_auto_filter;
class import_auto_filter_node;

}}

class ods_database_ranges_context : public xml_context_base
{
public:
    ods_database_ranges_context(
        session_context& session_cxt, const tokens& tokens,
        spreadsheet::iface::import_factory* factory);

    virtual ~ods_database_ranges_context() override;

    virtual void start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs) override;
    virtual bool end_element(xmlns_id_t ns, xml_token_t name) override;

    void reset();

private:
    void start_database_range(const xml_token_attrs_t& attrs);
    void end_database_range();

    void start_filter(const xml_token_attrs_t& attrs);
    void end_filter();

    void start_filter_condition(const xml_token_attrs_t& attrs);

    void start_filter_node(spreadsheet::auto_filter_node_op_t node_op);
    void end_filter_node();

private:
    spreadsheet::iface::import_factory* mp_factory = nullptr;
    spreadsheet::iface::import_table* mp_table = nullptr;
    spreadsheet::iface::import_auto_filter* mp_filter = nullptr;
    std::vector<spreadsheet::iface::import_auto_filter_node*> m_filter_node_stack;

    std::optional<spreadsheet::range_t> m_target_range;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
