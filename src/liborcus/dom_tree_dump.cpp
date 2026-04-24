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

class xml_dumper : public tree_walker
{
    // true when elem has at least one element child
    static bool has_element_children(const detail::element& elem)
    {
        return !elem.child_elem_positions.empty();
    }

    std::ostream& m_os;
    const xmlns_context& m_cxt;
    std::size_t m_indent;

    void write_indent(std::ostream& os, std::size_t depth)
    {
        for (std::size_t i = 0; i < depth * m_indent; ++i)
            os << ' ';
    }

public:
    xml_dumper(const detail::element& root, std::ostream& os, const xmlns_context& cxt, std::size_t indent) :
        tree_walker(root), m_os(os), m_cxt(cxt), m_indent(indent) {}

protected:
    void on_element_enter(const detail::element& elem, std::size_t depth) override
    {
        // indent only if parent element has at least one child element
        if (m_indent && elem.parent && has_element_children(*elem.parent))
            write_indent(m_os, depth);

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

        if (elem.child_nodes.empty())
        {
            // self-close leaf elements
            m_os << "/>";
            if (m_indent && elem.parent && has_element_children(*elem.parent))
                m_os << '\n';
        }
        else if (has_element_children(elem))
        {
            // block layout: each child element on its own line
            m_os << '>';
            if (m_indent)
                m_os << '\n';
        }
        else
        {
            // inline layout: content flows without extra whitespace
            m_os << '>';
        }
    }

    void on_element_exit(const detail::element& elem, std::size_t depth) override
    {
        if (elem.child_nodes.empty())
            return; // already closed with />

        if (m_indent && has_element_children(elem))
        {
            // close tag on its own indented line
            write_indent(m_os, depth);
            m_os << "</";
            detail::print(m_os, elem.name, m_cxt);
            m_os << ">\n";
        }
        else
        {
            // close tag inline, then newline if parent is in block layout
            m_os << "</";
            detail::print(m_os, elem.name, m_cxt);
            m_os << '>';
            if (m_indent && elem.parent && has_element_children(*elem.parent))
                m_os << '\n';
        }
    }

    void on_content(const detail::content& c, std::size_t depth) override
    {
        // indent content that sits alongside element siblings
        if (m_indent && c.parent && has_element_children(*c.parent))
        {
            write_indent(m_os, depth);
            write_content_encoded(m_os, c.value, xml_encode_context_t::text);
            m_os << '\n';
        }
        else
            write_content_encoded(m_os, c.value, xml_encode_context_t::text);
    }
};

} // anonymous namespace

std::string document_tree::dump(std::size_t indent) const
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
        if (indent)
            os << '\n';
    }

    xml_dumper walker(*mp_impl->m_root, os, mp_impl->m_ns_cxt, indent);
    walker.run();

    return os.str();
}

}} // namespace orcus::dom

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
