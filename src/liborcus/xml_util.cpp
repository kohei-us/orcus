/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xml_util.hpp"

#include <orcus/xml_namespace.hpp>
#include <orcus/tokens.hpp>

#include <sstream>
#include <iostream>

namespace orcus {

xml_element_printer::xml_element_printer(const tokens& t) :
    m_tokens(t)
{
}

void xml_element_printer::set_ns_context(const xmlns_context* ns_cxt)
{
    mp_ns_cxt = ns_cxt;
}

void xml_element_printer::print_namespace(std::ostream& os, xmlns_id_t ns) const
{
    if (mp_ns_cxt)
    {
        std::string_view alias = mp_ns_cxt->get_alias(ns);
        if (!alias.empty())
            os << alias;
        else
            os << mp_ns_cxt->get_short_name(ns);
    }
    else
        os << ns;
}

void xml_element_printer::print_element(std::ostream& os, xmlns_id_t ns, xml_token_t name) const
{
    os << '<';

    std::ostringstream os_ns;
    print_namespace(os_ns, ns);
    std::string ns_str = os_ns.str();

    if (!ns_str.empty())
        os << ns_str << ':';

    os << m_tokens.get_token_name(name) << '>';
}

void print_attrs(const tokens& tokens, const xml_token_attrs_t& attrs)
{
    for (const auto& attr : attrs)
    {
        std::cout << "  ";
        if (attr.ns != XMLNS_UNKNOWN_ID)
            std::cout << attr.ns << ":";

        std::cout << tokens.get_token_name(attr.name) << " = \"" << attr.value << "\"" << std::endl;
    }
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
