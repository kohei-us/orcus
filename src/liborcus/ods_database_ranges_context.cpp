/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ods_database_ranges_context.hpp"
#include "odf_token_constants.hpp"
#include "odf_namespace_types.hpp"

namespace orcus {

ods_database_ranges_context::ods_database_ranges_context(session_context& session_cxt, const tokens& tokens) :
    xml_context_base(session_cxt, tokens)
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

