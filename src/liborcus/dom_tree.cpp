/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dom_tree_impl.hpp"
#include <orcus/exception.hpp>

#include <sstream>
#include <cassert>
#include <algorithm>
#include <deque>

namespace orcus {

namespace dom {

entity_name::entity_name() : ns(XMLNS_UNKNOWN_ID) {}

entity_name::entity_name(std::string_view _name) :
    ns(XMLNS_UNKNOWN_ID), name(_name) {}

entity_name::entity_name(xmlns_id_t _ns, std::string_view _name) :
    ns(_ns), name(_name) {}

bool entity_name::operator== (const entity_name& other) const
{
    return ns == other.ns && name == other.name;
}

bool entity_name::operator!= (const entity_name& other) const
{
    return !operator==(other);
}

struct const_node::impl
{
    node_t type;

    union
    {
        const detail::declaration* decl;
        const detail::element* elem;

        struct
        {
            const char* p;
            size_t n;

        } str;

    } value;

    impl() : type(node_t::unset) {}

    impl(const impl& other) : type(other.type)
    {
        switch (type)
        {
            case node_t::declaration:
                value.decl = other.value.decl;
                break;
            case node_t::element:
                value.elem = other.value.elem;
                break;
            case node_t::unset:
            default:
                ;
        }
    }

    impl(const detail::element* _elem) : type(node_t::element)
    {
        value.elem = _elem;
    }

    impl(const detail::declaration* _decl) : type(node_t::declaration)
    {
        value.decl = _decl;
    }
};

const_node::const_node(std::unique_ptr<impl>&& _impl) : mp_impl(std::move(_impl)) {}
const_node::const_node() : mp_impl(std::make_unique<impl>()) {}
const_node::const_node(const const_node& other) : mp_impl(std::make_unique<impl>(*other.mp_impl)) {}
const_node::const_node(const_node&& other) : mp_impl(std::move(other.mp_impl)) {}
const_node::~const_node() {}

node_t const_node::type() const
{
    return mp_impl->type;
}

size_t const_node::child_count() const
{
    switch (mp_impl->type)
    {
        case node_t::element:
        {
            const detail::element* p = mp_impl->value.elem;
            return p->child_elem_positions.size();
        }
        default:
            ;
    }

    return 0;
}

const_node const_node::child(size_t index) const
{
    switch (mp_impl->type)
    {
        case node_t::element:
        {
            const detail::element* p = mp_impl->value.elem;

            size_t elem_pos = p->child_elem_positions.at(index);
            assert(elem_pos < p->child_nodes.size());

            const detail::node* child_node = p->child_nodes[elem_pos].get();
            assert(child_node->type == detail::node_type::element);

            auto v = std::make_unique<impl>(static_cast<const detail::element*>(child_node));
            return const_node(std::move(v));
        }
        default:
            ;
    }
    return const_node();
}

entity_name const_node::name() const
{
    switch (mp_impl->type)
    {
        case node_t::element:
        {
            const detail::element* p = mp_impl->value.elem;
            return p->name;
        }
        default:
            ;
    }

    return entity_name();
}

std::string_view const_node::attribute(const entity_name& name) const
{
    switch (mp_impl->type)
    {
        case node_t::element:
        {
            const detail::element* p = mp_impl->value.elem;
            auto it = p->attr_map.find(name);
            if (it == p->attr_map.end())
                break;

            size_t pos = it->second;
            assert(pos < p->attrs.size());
            return p->attrs[pos].value;
        }
        default:
            ;
    }

    return std::string_view();
}

std::string_view const_node::attribute(std::string_view name) const
{
    switch (mp_impl->type)
    {
        case node_t::declaration:
        {
            const detail::declaration* p = mp_impl->value.decl;
            auto it = p->attr_map.find(name);
            if (it == p->attr_map.end())
                return std::string_view();

            size_t pos = it->second;
            assert(pos < p->attrs.size());
            return p->attrs[pos].value;
        }
        default:
            ;
    }

    return attribute(entity_name(name));
}

size_t const_node::attribute_count() const
{
    switch (mp_impl->type)
    {
        case node_t::declaration:
        {
            const detail::declaration* p = mp_impl->value.decl;
            return p->attrs.size();
        }
        case node_t::element:
        {
            const detail::element* p = mp_impl->value.elem;
            return p->attrs.size();
        }
        default:
            ;
    }
    return 0;
}

const_node const_node::parent() const
{
    if (mp_impl->type != node_t::element)
        return const_node();

    const detail::element* p = mp_impl->value.elem->parent;
    if (!p)
        return const_node();

    auto v = std::make_unique<impl>(p);
    return const_node(std::move(v));
}

void const_node::swap(const_node& other)
{
    mp_impl.swap(other.mp_impl);
}

const_node& const_node::operator= (const const_node& other)
{
    const_node tmp(other);
    swap(tmp);
    return *this;
}

bool const_node::operator== (const const_node& other) const
{
    if (mp_impl->type != other.mp_impl->type)
        return false;

    switch (mp_impl->type)
    {
        case node_t::unset:
            return true;
        case node_t::declaration:
            return mp_impl->value.decl == other.mp_impl->value.decl;
        case node_t::element:
            return mp_impl->value.elem == other.mp_impl->value.elem;
        default:
            ;
    }

    return false;
}

bool const_node::operator!= (const const_node& other) const
{
    return !operator==(other);
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

void document_tree::impl::set_attribute(xmlns_id_t ns, std::string_view name, std::string_view val)
{
    // These strings must be persistent.
    std::string_view name2 = m_pool.intern(name).first;
    std::string_view val2 = m_pool.intern(val).first;

    size_t pos = m_cur_attrs.size();
    m_cur_attrs.push_back(detail::attr(ns, name2, val2));
    m_cur_attr_map.insert({entity_name(ns, name2), pos});
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

document_tree::document_tree(xmlns_context& cxt) :
    mp_impl(std::make_unique<impl>(cxt)) {}

document_tree::document_tree(document_tree&& other) :
    mp_impl(std::move(other.mp_impl))
{
    other.mp_impl = std::make_unique<impl>(mp_impl->m_ns_cxt);
}

document_tree::~document_tree() {}

void document_tree::load(std::string_view strm)
{
    sax_ns_parser<impl> parser(strm, mp_impl->m_ns_cxt, *mp_impl);
    parser.parse();
}

dom::const_node document_tree::root() const
{
    const detail::element* p = mp_impl->m_root.get();
    auto v = std::make_unique<const_node::impl>(p);
    return dom::const_node(std::move(v));
}

dom::const_node document_tree::declaration(std::string_view name) const
{
    impl::declarations_type::const_iterator it = mp_impl->m_decls.find(name);
    if (it == mp_impl->m_decls.end())
        return dom::const_node();

    const detail::declaration* decl = &it->second;
    auto v = std::make_unique<dom::const_node::impl>(decl);
    return dom::const_node(std::move(v));
}

void document_tree::swap(document_tree& other)
{
    mp_impl.swap(other.mp_impl);
}

const sax::doctype_declaration* document_tree::get_doctype() const
{
    return mp_impl->m_doctype.get();
}

namespace {

struct scope
{
    typedef std::vector<const detail::node*> nodes_type;
    std::string name;
    nodes_type nodes;
    nodes_type::const_iterator current_pos;

    scope(const scope&) = delete;
    scope& operator=(const scope&) = delete;

    scope(const std::string& _name, detail::node* _node) :
        name(_name)
    {
        nodes.push_back(_node);
        current_pos = nodes.begin();
    }

    scope(const std::string& _name) : name(_name) {}
};

typedef std::deque<scope> scopes_type;

void print_scope(std::ostream& os, const scopes_type& scopes)
{
    if (scopes.empty())
        throw general_error("scope stack shouldn't be empty while dumping tree.");

    // Skip the first scope which is root.
    scopes_type::const_iterator it = scopes.begin(), it_end = scopes.end();
    for (++it; it != it_end; ++it)
        os << "/" << it->name;
}

}

void document_tree::dump_compact(std::ostream& os) const
{
    if (!mp_impl->m_root)
        return;

    // Dump namespaces first.
    mp_impl->m_ns_cxt.dump(os);

    scopes_type scopes;

    scopes.emplace_back(std::string(), mp_impl->m_root.get());
    while (!scopes.empty())
    {
        bool new_scope = false;

        // Iterate through all elements in the current scope.
        scope& cur_scope = scopes.back();
        for (; cur_scope.current_pos != cur_scope.nodes.end(); ++cur_scope.current_pos)
        {
            const detail::node* this_node = *cur_scope.current_pos;
            assert(this_node);
            print_scope(os, scopes);
            if (this_node->type == detail::node_type::content)
            {
                // This is a text content.
                this_node->print(os, mp_impl->m_ns_cxt);
                os << std::endl;
                continue;
            }

            assert(this_node->type == detail::node_type::element);
            const detail::element* elem = static_cast<const detail::element*>(this_node);
            os << "/";
            elem->print(os, mp_impl->m_ns_cxt);
            os << std::endl;

            {
                // Dump attributes.
                detail::attrs_type attrs = elem->attrs;
                std::sort(attrs.begin(), attrs.end(),
                      [](const detail::attr& left, const detail::attr& right) -> bool
                      {
                          return left.name.name < right.name.name;
                      }
                );

                for (const detail::attr& a : attrs)
                {
                    print_scope(os, scopes);
                    os << "/";
                    elem->print(os, mp_impl->m_ns_cxt);
                    os << "@";
                    detail::print(os, a, mp_impl->m_ns_cxt);
                    os << std::endl;
                }
            }

            if (elem->child_nodes.empty())
                continue;

            // This element has child nodes.  Push a new scope and populate it
            // with all child elements, but skip content nodes.
            scope::nodes_type nodes;
            for (const std::unique_ptr<detail::node>& p : elem->child_nodes)
                nodes.push_back(p.get());

            assert(!nodes.empty());

            // Push a new scope, and restart the loop with the new scope.
            ++cur_scope.current_pos;
            std::ostringstream elem_name;
            elem->print(elem_name, mp_impl->m_ns_cxt);
            scopes.emplace_back(elem_name.str());
            scope& child_scope = scopes.back();
            child_scope.nodes.swap(nodes);
            child_scope.current_pos = child_scope.nodes.begin();

            new_scope = true;
            break;
        }

        if (new_scope)
            continue;

        scopes.pop_back();
    }
}

} // namespace dom

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
