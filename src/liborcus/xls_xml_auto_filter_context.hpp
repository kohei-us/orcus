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
#include <vector>

namespace orcus {

namespace spreadsheet { namespace iface {

class import_auto_filter;
class import_auto_filter_node;
class import_factory;
class import_sheet;

}}

class xls_xml_auto_filter_context : public xml_context_base
{
public:
    enum class filter_column_type
    {
        all,
        blanks,
        non_blanks,
        top,
        top_percent,
        bottom,
        bottom_percent,
        custom
    };

    xls_xml_auto_filter_context(
        session_context& session_cxt, const tokens& tokens,
        spreadsheet::iface::import_factory* factory);

    virtual ~xls_xml_auto_filter_context() override;

    virtual void start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs) override;
    virtual bool end_element(xmlns_id_t ns, xml_token_t name) override;

    void reset(spreadsheet::iface::import_sheet* parent_sheet);

private:
    void start_auto_filter(const xml_token_attrs_t& attrs);
    void end_auto_filter();
    void start_column(const xml_token_attrs_t& attrs);
    void end_column();
    void start_condition(const xml_token_attrs_t& attrs);
    void end_condition();

private:

    struct column_attrs
    {
        std::optional<spreadsheet::col_t> index;
        std::optional<filter_column_type> type;

        void reset()
        {
            index.reset();
            type.reset();
        }
    };

    spreadsheet::iface::import_factory* mp_factory = nullptr;
    spreadsheet::iface::import_sheet* mp_sheet = nullptr;
    spreadsheet::iface::import_auto_filter* mp_auto_filter = nullptr;

    std::vector<spreadsheet::iface::import_auto_filter_node*> m_column_filter_stack;

    column_attrs m_column;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
