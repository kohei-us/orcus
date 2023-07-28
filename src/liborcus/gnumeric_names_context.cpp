/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gnumeric_names_context.hpp"
#include "gnumeric_token_constants.hpp"
#include "gnumeric_namespace_types.hpp"

#include <orcus/spreadsheet/import_interface.hpp>

#include <iostream>

namespace ss = orcus::spreadsheet;

namespace orcus {

gnumeric_names_context::gnumeric_names_context(
    session_context& session_cxt, const tokens& tokens,
    spreadsheet::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_gnumeric_gnm, XML_Names }, // root element
        { NS_gnumeric_gnm, XML_Names, NS_gnumeric_gnm, XML_Name },
        { NS_gnumeric_gnm, XML_Name, NS_gnumeric_gnm, XML_name },
        { NS_gnumeric_gnm, XML_Name, NS_gnumeric_gnm, XML_value },
        { NS_gnumeric_gnm, XML_Name, NS_gnumeric_gnm, XML_position },
    };

    init_element_validator(rules, std::size(rules));
}

gnumeric_names_context::~gnumeric_names_context() = default;

void gnumeric_names_context::start_element(
    xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& /*attrs*/)
{
    push_stack(ns, name);

    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_Name:
                m_current_name.reset();
                break;
        }
    }
}

bool gnumeric_names_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_Name:
                m_named_exps.push_back(m_current_name);
                break;
        }
    }
    return pop_stack(ns, name);
}

void gnumeric_names_context::characters(std::string_view str, bool transient)
{
    ss::iface::import_reference_resolver* resolver = mp_factory->get_reference_resolver(
        ss::formula_ref_context_t::global);

    const auto [ns, name] = get_current_element();

    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_name:
                m_current_name.name = transient ? intern(str) : str;
                break;
            case XML_value:
                m_current_name.value = transient ? intern(str) : str;
                break;
            case XML_position:
                m_current_name.position = resolver->resolve_address(str);
                break;
        }
    }
}

void gnumeric_names_context::reset()
{
    m_named_exps.clear();
}

const std::vector<gnumeric_named_exp>& gnumeric_names_context::get_names() const
{
    return m_named_exps;
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
