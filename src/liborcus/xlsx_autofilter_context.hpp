/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_XLSX_AUTOFILTER_CONTEXT_HPP
#define ORCUS_XLSX_AUTOFILTER_CONTEXT_HPP

#include "xml_context_base.hpp"
#include "orcus/string_pool.hpp"
#include "orcus/spreadsheet/types.hpp"

#include <vector>
#include <map>

namespace orcus {

namespace spreadsheet { namespace iface {

namespace old { class import_auto_filter; }
class import_reference_resolver;

}}

class xlsx_autofilter_context : public xml_context_base
{
public:
    typedef std::vector<std::string_view> match_values_type;
    typedef std::map<spreadsheet::col_t, match_values_type> column_filters_type;

    xlsx_autofilter_context(
        session_context& session_cxt, const tokens& tokens,
        spreadsheet::iface::import_reference_resolver& resolver);
    virtual ~xlsx_autofilter_context();

    virtual void start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs);
    virtual bool end_element(xmlns_id_t ns, xml_token_t name);
    virtual void characters(std::string_view str, bool transient);

    void push_to_model(spreadsheet::iface::old::import_auto_filter& af) const;

    void reset();

private:
    spreadsheet::iface::import_reference_resolver& m_resolver;

    string_pool m_pool;

    std::string_view m_ref_range;
    spreadsheet::col_t m_cur_col;
    match_values_type m_cur_match_values;
    column_filters_type m_column_filters;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
