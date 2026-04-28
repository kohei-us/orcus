/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dom_tree_impl.hpp"

#include <cassert>
#include <deque>

namespace orcus { namespace dom { namespace detail {

attr::attr(xmlns_id_t _ns, std::string_view _name, std::string_view _value) :
    name(_ns, _name), value(_value) {}

std::size_t entity_name_hash::operator()(const entity_name& v) const
{
    return std::hash<std::string_view>{}(v.name) ^ reinterpret_cast<std::size_t>(v.ns);
}

ns_declaration::ns_declaration(std::string_view _alias, xmlns_id_t _name) :
    alias(_alias), name(_name) {}

void print(std::ostream& os, const entity_name& name, const xmlns_context& cxt)
{
    if (name.ns)
    {
        std::size_t index = cxt.get_index(name.ns);
        if (index != INDEX_NOT_FOUND)
            os << "ns" << index << ':';
    }
    os << name.name;
}

void print(std::ostream& os, const attr& at, const xmlns_context& cxt)
{
    print(os, at.name, cxt);
    os << "=\"";
    escape(os, at.value);
    os << '"';
}

void escape(std::ostream& os, std::string_view val)
{
    if (val.empty())
        return;

    const char* p = val.data();
    const char* p_end = p + val.size();
    for (; p != p_end; ++p)
    {
        switch (*p)
        {
            case '"':
                os << "\\\"";
                break;
            case '\\':
                os << "\\\\";
                break;
            case '\b':
                os << "\\b";
                break;
            case '\f':
                os << "\\f";
                break;
            case '\n':
                os << "\\n";
                break;
            case '\r':
                os << "\\r";
                break;
            case '\t':
                os << "\\t";
                break;
            default:
                os << *p;
        }
    }
}

node::~node() = default;

element::element(xmlns_id_t _ns, std::string_view _name) :
    node(node_type::element), name(_ns, _name) {}

element::~element() = default;

content::content(std::string_view _value) : node(node_type::content), value(_value) {}

content::~content() = default;

void print(std::ostream& os, const content& c, const xmlns_context& /*cxt*/)
{
    os << '"';
    escape(os, c.value);
    os << '"';
}

}}} // namespace orcus::dom::detail

namespace orcus { namespace dom {

void document_tree::impl::namespace_declaration(std::string_view alias, xmlns_id_t ns_id)
{
    std::string_view alias_safe = m_pool.intern(alias).first;
    m_cur_ns_decls.emplace_back(alias_safe, ns_id);
}

void document_tree::impl::end_declaration(std::string_view name)
{
    assert(m_cur_decl_name == name);

    detail::declaration decl;
    decl.attrs.swap(m_cur_attrs);
    decl.attr_map.swap(m_cur_attr_map);

    declarations_type::iterator it = m_decls.find(name);
    if (it == m_decls.end())
    {
        // Insert a new entry for this name.
        std::pair<declarations_type::iterator,bool> r =
            m_decls.insert(
                declarations_type::value_type(
                    m_pool.intern(name).first, std::move(decl)));

        if (!r.second)
            // Insertion failed.
            throw general_error("dom_tree::end_declaration: failed to insert a new declaration entry.");
    }
    else
        it->second = std::move(decl);
}

void document_tree::impl::start_element(const sax_ns_parser_element& elem)
{
    xmlns_id_t ns = elem.ns;
    std::string_view name = elem.name;

    // These strings must be persistent.
    std::string_view name_safe = m_pool.intern(name).first;

    detail::element* p = nullptr;
    if (!m_root)
    {
        // This must be the root element!
        m_root = std::make_unique<detail::element>(ns, name_safe);
        m_elem_stack.push_back(m_root.get());
        p = m_elem_stack.back();
        p->attrs.swap(m_cur_attrs);
        p->attr_map.swap(m_cur_attr_map);
        p->ns_decls.swap(m_cur_ns_decls);
        return;
    }

    // Append new element as a child element of the current element.
    p = m_elem_stack.back();

    size_t elem_pos = p->child_nodes.size();
    p->child_elem_positions.push_back(elem_pos);

    p->child_nodes.push_back(std::make_unique<detail::element>(ns, name_safe));
    const detail::element* parent = p;
    p = static_cast<detail::element*>(p->child_nodes.back().get());
    p->parent = parent;
    p->attrs.swap(m_cur_attrs);
    p->attr_map.swap(m_cur_attr_map);
    p->ns_decls.swap(m_cur_ns_decls);

    m_elem_stack.push_back(p);
}

void document_tree::impl::end_element(const sax_ns_parser_element& elem)
{
    xmlns_id_t ns = elem.ns;
    std::string_view name = elem.name;

    const detail::element* p = m_elem_stack.back();
    if (p->name.ns != ns || p->name.name != name)
        throw general_error("non-matching end element.");

    m_elem_stack.pop_back();
}

void document_tree::impl::characters(std::string_view val, bool /*transient*/)
{
    if (m_elem_stack.empty())
        // No root element has been encountered.  Ignore this.
        return;

    std::string_view val2 = trim(val);
    if (val2.empty())
        return;

    detail::element* p = m_elem_stack.back();
    val2 = m_pool.intern(val2).first; // Make sure the string is persistent.
    auto child = std::make_unique<detail::content>(val2);
    child->parent = p;
    p->child_nodes.push_back(std::move(child));
}

void document_tree::impl::doctype(const sax::doctype_declaration& dtd)
{
    m_doctype = std::make_unique<sax::doctype_declaration>(dtd);  // make a copy.

    sax::doctype_declaration& this_dtd = *m_doctype;
    string_pool& pool = m_pool;

    // Intern the strings.
    this_dtd.root_element = pool.intern(dtd.root_element).first;
    this_dtd.fpi = pool.intern(dtd.fpi).first;
    this_dtd.uri = pool.intern(dtd.uri).first;
}

void document_tree::impl::set_attribute(xmlns_id_t ns, std::string_view name, std::string_view val)
{
    // These strings must be persistent.
    std::string_view name2 = m_pool.intern(name).first;
    std::string_view val2 = m_pool.intern(val).first;

    size_t pos = m_cur_attrs.size();
    m_cur_attrs.push_back(detail::attr(ns, name2, val2));
    m_cur_attr_map.insert({entity_name(ns, name2), pos});
}

tree_walker::scope::scope(const detail::element* _owner, std::size_t _depth) :
    current_pos(nodes.begin()), owner(_owner), depth(_depth) {}

tree_walker::tree_walker(const detail::element& root) : m_root(root) {}

void tree_walker::run()
{
    std::deque<scope> scopes;
    scopes.emplace_back(nullptr, 0u);
    scopes.back().nodes.push_back(&m_root);
    scopes.back().current_pos = scopes.back().nodes.begin();

    while (!scopes.empty())
    {
        scope& cur_scope = scopes.back();

        if (cur_scope.current_pos == cur_scope.nodes.end())
        {
            // current scope has no more nodes to process - end the scope
            if (cur_scope.owner)
                on_element_exit(*cur_scope.owner, cur_scope.depth - 1);
            scopes.pop_back();
            continue;
        }

        // process the current node in the current scope
        const detail::node* this_node = *cur_scope.current_pos;
        ++cur_scope.current_pos;
        assert(this_node);

        if (this_node->type == detail::node_type::content)
        {
            const auto* cnt = static_cast<const detail::content*>(this_node);
            on_content(*cnt, cur_scope.depth);
            continue;
        }

        const auto* elem = static_cast<const detail::element*>(this_node);
        on_element_enter(*elem, cur_scope.depth);

        if (elem->child_nodes.empty())
        {
            // this element is a leaf element
            on_element_exit(*elem, cur_scope.depth);
            continue;
        }

        // push a new scope with the child elements of this element
        nodes_type child_nodes;
        for (const auto& p : elem->child_nodes)
            child_nodes.push_back(p.get());

        scopes.emplace_back(elem, cur_scope.depth + 1);
        scope& child_scope = scopes.back();
        child_scope.nodes.swap(child_nodes);
        child_scope.current_pos = child_scope.nodes.begin();
    }

    on_document_exit();
}

}} // namespace orcus::dom

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
