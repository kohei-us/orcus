/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dom_tree_impl.hpp"
#include <orcus/xml_encode.hpp>

#include <sstream>

namespace orcus { namespace dom {

namespace {

struct xml_dumper : public tree_walker
{
    std::ostream& m_os;
    const xmlns_context& m_cxt;

    xml_dumper(const detail::element& root, std::ostream& os, const xmlns_context& cxt) :
        tree_walker(root), m_os(os), m_cxt(cxt) {}

protected:
    void on_element_enter(const detail::element& elem, std::size_t /*depth*/) override
    {
        m_os << '<';
        detail::print(m_os, elem.name, m_cxt);

        for (const detail::attr& a : elem.attrs)
        {
            m_os << ' ';
            detail::print(m_os, a.name, m_cxt);
            m_os << "=\"";
            write_content_encoded(m_os, a.value, xml_encode_context_t::attr_double_quoted);
            m_os << '"';
        }

        // self-close leaf elements
        m_os << (elem.child_nodes.empty() ? "/>" : ">");
    }

    void on_element_exit(const detail::element& elem, std::size_t /*depth*/) override
    {
        if (elem.child_nodes.empty())
            return; // already closed with />

        m_os << "</";
        detail::print(m_os, elem.name, m_cxt);
        m_os << '>';
    }

    void on_content(const detail::content& c, std::size_t /*depth*/) override
    {
        write_content_encoded(m_os, c.value, xml_encode_context_t::text);
    }
};

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

    xml_dumper walker(*mp_impl->m_root, os, mp_impl->m_ns_cxt);
    walker.run();

    return os.str();
}

}} // namespace orcus::dom

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
