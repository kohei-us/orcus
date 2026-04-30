/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dom_tree_impl.hpp"
#include <orcus/xml_encode.hpp>

#include <functional>
#include <sstream>
#include <unordered_map>

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
    std::vector<std::reference_wrapper<const detail::element>> m_alias_elems;

    void write_indent(std::ostream& os, std::size_t depth)
    {
        for (std::size_t i = 0; i < depth * m_indent; ++i)
            os << ' ';
    }

    void write_name(const entity_name& name)
    {
        if (name.ns)
        {
            auto alias = find_alias(name);
            if (!alias.empty())
                // not a default namespace
                m_os << alias << ':';
        }
        m_os << name.name;
    }

    std::string_view find_alias(const entity_name& name) const
    {
        // recursively search for the matching namespace from the inner element
        // and up
        auto it = m_alias_elems.rbegin(), it_end = m_alias_elems.rend();
        for (; it != it_end; ++it)
        {
            const detail::element& elem = *it;
            assert(!elem.ns_decls.empty());

            for (const auto& [alias, ns] : elem.ns_decls)
            {
                if (name.ns == ns)
                    return alias;
            }
        }

        // The xml prefix is predefined and never appears in element ns_decls.
        if (name.ns != XMLNS_UNKNOWN_ID && name.ns == m_cxt.get("xml"))
            return "xml";

        std::ostringstream os;
        os << "namespace alias for " << name << " not found but it should have";
        throw general_error(os.str());
    }

    void push_aliases(const detail::element& elem)
    {
        if (!elem.ns_decls.empty())
            m_alias_elems.push_back(elem);
    }

    void pop_aliases(const detail::element& elem)
    {
        if (!m_alias_elems.empty())
        {
            if (&m_alias_elems.back().get() == &elem)
                m_alias_elems.pop_back();
        }
    }

public:
    xml_dumper(const detail::element& root, std::ostream& os, const xmlns_context& cxt, std::size_t indent) :
        tree_walker(root), m_os(os), m_cxt(cxt), m_indent(indent) {}

protected:
    void on_element_enter(const detail::element& elem, std::size_t depth) override
    {
        // register this element's namespace aliases before printing its name
        push_aliases(elem);

        // indent only if parent element has at least one child element
        if (m_indent && elem.parent && has_element_children(*elem.parent))
            write_indent(m_os, depth);

        m_os << '<';
        write_name(elem.name);

        // emit namespace declarations recorded on this element
        for (const auto& [alias, ns_id] : elem.ns_decls)
        {
            if (alias.empty())
                m_os << " xmlns=\"";
            else
                m_os << " xmlns:" << alias << "=\"";
            write_content_encoded(m_os, ns_id, xml_encode_context_t::attr_double_quoted);
            m_os << '"';
        }

        for (const detail::attr& a : elem.attrs)
        {
            m_os << ' ';
            write_name(a.name);
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
        {
            // already closed with />; just clean up aliases
            pop_aliases(elem);
            return;
        }

        if (m_indent && has_element_children(elem))
        {
            // close tag on its own indented line
            write_indent(m_os, depth);
            m_os << "</";
            write_name(elem.name);
            m_os << ">\n";
        }
        else
        {
            // close tag inline, then newline if parent is in block layout
            m_os << "</";
            write_name(elem.name);
            m_os << '>';
            if (m_indent && elem.parent && has_element_children(*elem.parent))
                m_os << '\n';
        }

        // pop this element's aliases after writing the closing tag
        pop_aliases(elem);
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

    void on_document_exit() override
    {
        assert(m_alias_elems.empty());
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
