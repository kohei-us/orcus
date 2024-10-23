/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ods_database_ranges_context.hpp"
#include "odf_token_constants.hpp"
#include "odf_namespace_types.hpp"

namespace ss = orcus::spreadsheet;

namespace orcus {

ods_database_ranges_context::ods_database_ranges_context(
    session_context& session_cxt, const tokens& tokens, ss::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_odf_table, XML_database_ranges }, // root element
        { NS_odf_table, XML_database_range, NS_odf_table, XML_filter },
        { NS_odf_table, XML_database_ranges, NS_odf_table, XML_database_range },
        { NS_odf_table, XML_filter, NS_odf_table, XML_filter_and },
        { NS_odf_table, XML_filter, NS_odf_table, XML_filter_or },
        { NS_odf_table, XML_filter_and, NS_odf_table, XML_filter_condition },
        { NS_odf_table, XML_filter_and, NS_odf_table, XML_filter_or },
        { NS_odf_table, XML_filter_or, NS_odf_table, XML_filter_and },
        { NS_odf_table, XML_filter_or, NS_odf_table, XML_filter_condition },
    };

    init_element_validator(rules, std::size(rules));
}

ods_database_ranges_context::~ods_database_ranges_context() = default;

void ods_database_ranges_context::start_element(
    xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
{
    push_stack(ns, name);

    if (ns != NS_odf_table)
        warn_unexpected();

    switch (name)
    {
        case XML_database_ranges:
            break;
        case XML_database_range:
            break;
        case XML_filter:
            break;
        case XML_filter_or:
            break;
        case XML_filter_and:
            break;
        case XML_filter_condition:
            break;
        default:
            warn_unexpected();
    }
}

bool ods_database_ranges_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    return pop_stack(ns, name);
}

void ods_database_ranges_context::reset()
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

