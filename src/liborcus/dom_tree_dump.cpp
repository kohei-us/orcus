/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dom_tree_impl.hpp"
#include <orcus/xml_encode.hpp>

#include <sstream>
#include <cassert>

namespace orcus { namespace dom {

namespace {

void dump_element_name(std::ostream& os, const entity_name& name, const xmlns_context& cxt)
{
    if (name.ns)
    {
        std::size_t index = cxt.get_index(name.ns);
        if (index != INDEX_NOT_FOUND)
            os << "ns" << index << ':';
    }
    os << name.name;
}

void dump_element(std::ostream& os, const detail::element& elem, const xmlns_context& cxt)
{
    os << '<';
    dump_element_name(os, elem.name, cxt);

    for (const detail::attr& a : elem.attrs)
    {
        os << ' ';
        dump_element_name(os, a.name, cxt);
        os << "=\"";
        write_content_encoded(os, a.value, xml_encode_context_t::attr_double_quoted);
        os << '"';
    }

    if (elem.child_nodes.empty())
    {
        os << "/>";
        return;
    }

    os << '>';

    for (const auto& child : elem.child_nodes)
    {
        if (child->type == detail::node_type::content)
        {
            const auto* c = static_cast<const detail::content*>(child.get());
            write_content_encoded(os, c->value, xml_encode_context_t::text);
        }
        else
        {
            assert(child->type == detail::node_type::element);
            dump_element(os, static_cast<const detail::element&>(*child), cxt);
        }
    }

    os << "</";
    dump_element_name(os, elem.name, cxt);
    os << '>';
}

} // anonymous namespace

std::string document_tree::dump(std::size_t /*indent*/) const
{
    if (!mp_impl->m_root)
        return {};

    std::ostringstream os;

    // Emit XML declarations
    for (const auto& [name, decl] : mp_impl->m_decls)
    {
        os << "<?" << name;
        for (const detail::attr& a : decl.attrs)
        {
            os << ' ' << a.name.name << "=\"";
            write_content_encoded(os, a.value, xml_encode_context_t::attr_double_quoted);
            os << '"';
        }
        os << "?>";
    }

    dump_element(os, *mp_impl->m_root, mp_impl->m_ns_cxt);

    return os.str();
}

}} // namespace orcus::dom

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
