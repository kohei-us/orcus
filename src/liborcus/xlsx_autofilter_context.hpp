/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_XLSX_AUTOFILTER_CONTEXT_HPP
#define ORCUS_XLSX_AUTOFILTER_CONTEXT_HPP

#include "xml_context_base.hpp"
#include "xls_filter_utils.hpp"
#include <orcus/string_pool.hpp>
#include <orcus/spreadsheet/types.hpp>

#include <vector>
#include <map>
#include <functional>

namespace orcus {

namespace spreadsheet { namespace iface {

namespace old { class import_auto_filter; }

class import_auto_filter;
class import_auto_filter_node;
class import_auto_filter_multi_values;
class import_reference_resolver;

}}

class xlsx_autofilter_context : public xml_context_base
{
public:
    typedef std::vector<std::string_view> match_values_type;
    typedef std::map<spreadsheet::col_t, match_values_type> column_filters_type;

    /**
     * Function that 'starts' an auto filter import and returns an
     * import_auto_filter interface.
     */
    using iface_factory_type = std::function<spreadsheet::iface::import_auto_filter*(const spreadsheet::range_t&)>;

    xlsx_autofilter_context(
        session_context& session_cxt, const tokens& tokens,
        spreadsheet::iface::import_reference_resolver& resolver);
    virtual ~xlsx_autofilter_context() override;

    virtual void start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs) override;
    virtual bool end_element(xmlns_id_t ns, xml_token_t name) override;

    void push_to_model(spreadsheet::iface::old::import_auto_filter& af) const;

    void reset(iface_factory_type factory);

private:
    void start_auto_filter(const xml_token_attrs_t& attrs);
    void end_auto_filter();
    void start_custom_filters(const xml_token_attrs_t& attrs);
    void end_custom_filters();
    void start_custom_filter(const xml_token_attrs_t& attrs);
    void end_custom_filter();
    void start_filters(const xml_token_attrs_t& attrs);
    void end_filters();
    void start_filter(const xml_token_attrs_t& attrs);
    void start_top10(const xml_token_attrs_t& attrs);
    void start_dynamic_filter(const xml_token_attrs_t& attrs);
    void start_filter_column(const xml_token_attrs_t& attrs);
    void end_filter_column();

private:
    spreadsheet::iface::import_reference_resolver& m_resolver;
    spreadsheet::iface::import_auto_filter* mp_auto_filter = nullptr;
    spreadsheet::iface::import_auto_filter_multi_values* mp_multi_values = nullptr;
    spreadsheet::col_t m_cur_col;

    iface_factory_type m_factory;

    std::vector<spreadsheet::iface::import_auto_filter_node*> m_node_stack;
    detail::xls_filter_value_parser m_value_parser;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
