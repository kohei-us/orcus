/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/dom_tree.hpp"
#include "orcus/exception.hpp"
#include "orcus/xml_namespace.hpp"
#include "orcus/global.hpp"
#include "orcus/sax_ns_parser.hpp"

#include "orcus/string_pool.hpp"

#include <iostream>
#include <sstream>
#include <cassert>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <deque>

using namespace std;

namespace orcus {

namespace dom {

namespace {

/**
 * Escape certain characters with backslash (\).
 */
void escape(ostream& os, const pstring& val)
{
    if (val.empty())
        return;

    const char* p = val.data();
    const char* p_end = p + val.size();
    for (; p != p_end; ++p)
    {
        if (*p == '"')
            os << "\\\"";
        else if (*p == '\\')
            os << "\\\\";
        else
            os << *p;
    }
}

struct attr
{
    dom::entity_name name;
    pstring value;

    attr(xmlns_id_t _ns, const pstring& _name, const pstring& _value);
};

typedef std::vector<attr> attrs_type;

enum class node_type { element, content };

struct node
{
    node_type type;

    node(node_type _type) : type(_type) {}

    virtual ~node() = 0;
    virtual void print(std::ostream& os, const xmlns_context& cxt) const = 0;
};

typedef std::vector<std::unique_ptr<node>> nodes_type;

struct element : public node
{
    entity_name name;
    attrs_type attrs;
    nodes_type child_nodes;

    element(xmlns_id_t _ns, const pstring& _name);
    virtual void print(std::ostream& os, const xmlns_context& cxt) const;
    virtual ~element();
};

struct content : public node
{
    pstring value;

    content(const pstring& _value);
    virtual void print(std::ostream& os, const xmlns_context& cxt) const;
    virtual ~content();
};

void print(std::ostream& os, const entity_name& name, const xmlns_context& cxt)
{
    if (name.ns)
    {
        size_t index = cxt.get_index(name.ns);
        if (index != index_not_found)
            os << "ns" << index << ':';
    }
    os << name.name;
}

void print(std::ostream& os, const attr& at, const xmlns_context& cxt)
{
    dom::print(os, at.name, cxt);
    os << "=\"";
    escape(os, at.value);
    os << '"';
}

attr::attr(xmlns_id_t _ns, const pstring& _name, const pstring& _value) :
    name(_ns, _name), value(_value) {}

node::~node() {}

element::element(xmlns_id_t _ns, const pstring& _name) : node(node_type::element), name(_ns, _name) {}

void element::print(ostream& os, const xmlns_context& cxt) const
{
    dom::print(os, name, cxt);
}

element::~element() {}

content::content(const pstring& _value) : node(node_type::content), value(_value) {}

void content::print(ostream& os, const xmlns_context& /*cxt*/) const
{
    os << '"';
    escape(os, value);
    os << '"';
}

content::~content() {}

} // anonymous namespace

entity_name::entity_name() : ns(XMLNS_UNKNOWN_ID) {}

entity_name::entity_name(xmlns_id_t _ns, const pstring& _name) :
    ns(_ns), name(_name) {}

struct const_node::impl
{
    node_t type;

    union
    {
        const dom::attrs_type* attrs;
        const dom::attr* attr;
        const dom::element* elem;

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
            case node_t::attribute:
                value.attr = other.value.attr;
                break;
            case node_t::content:
                break;
            case node_t::declaration:
                value.attrs = other.value.attrs;
                break;
            case node_t::element:
                break;
            case node_t::unset:
            default:
                ;
        }
    }

    impl(const dom::element* _elem) : type(node_t::element)
    {
        value.elem = _elem;
    }

    impl(const dom::attr* _attr) : type(node_t::attribute)
    {
        value.attr = _attr;
    }

    impl(const dom::attrs_type* _attrs) : type(node_t::declaration)
    {
        value.attrs = _attrs;
    }
};

const_node::const_node(std::unique_ptr<impl>&& _impl) : mp_impl(std::move(_impl)) {}
const_node::const_node() : mp_impl(orcus::make_unique<impl>()) {}
const_node::const_node(const const_node& other) : mp_impl(orcus::make_unique<impl>(*other.mp_impl)) {}
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
        case node_t::declaration:
            return mp_impl->value.attrs->size();
        default:
            ;
    }

    return 0;
}

const_node const_node::child(size_t index) const
{
    switch (mp_impl->type)
    {
        case node_t::declaration:
        {
            assert(index < mp_impl->value.attrs->size());
            const dom::attrs_type& attrs = *mp_impl->value.attrs;
            auto v = orcus::make_unique<impl>(&attrs[index]);
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
        case node_t::attribute:
        {
            const dom::attr* v = mp_impl->value.attr;
            return entity_name(v->name.ns, v->name.name);
        }
        default:
            ;
    }

    return entity_name();
}

pstring const_node::value() const
{
    switch (mp_impl->type)
    {
        case node_t::attribute:
        {
            const dom::attr* v = mp_impl->value.attr;
            return v->value;
        }
        default:
            ;
    }
    return pstring();
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

} // namespace dom

struct dom_tree::impl
{
    typedef std::vector<dom::element*> element_stack_type;
    typedef std::unordered_map<pstring, dom::attrs_type, pstring::hash> declarations_type;

    xmlns_context& m_ns_cxt;
    string_pool m_pool;

    std::unique_ptr<sax::doctype_declaration> m_doctype;

    pstring m_cur_decl_name;
    declarations_type m_decls;
    dom::attrs_type m_doc_attrs;
    dom::attrs_type m_cur_attrs;
    element_stack_type m_elem_stack;
    std::unique_ptr<dom::element> m_root;

    impl(xmlns_context& cxt) : m_ns_cxt(cxt) {}

    void start_declaration(const pstring& name)
    {
        m_cur_decl_name = name;
    }

    void end_declaration(const pstring& name);
    void start_element(const sax_ns_parser_element& elem);
    void end_element(const sax_ns_parser_element& elem);
    void characters(const pstring& val, bool transient);
    void doctype(const sax::doctype_declaration& dtd);

    void attribute(const pstring& name, const pstring& val)
    {
        set_attribute(XMLNS_UNKNOWN_ID, name, val);
    }

    void attribute(const sax_ns_parser_attribute& attr)
    {
        set_attribute(attr.ns, attr.name, attr.value);
    }

    void set_attribute(xmlns_id_t ns, const pstring& name, const pstring& val);
};

void dom_tree::impl::end_declaration(const pstring& name)
{
    assert(m_cur_decl_name == name);
    declarations_type& decls = m_decls;
    declarations_type::iterator it = decls.find(name);
    if (it == decls.end())
    {
        // Insert a new entry for this name.
        std::pair<declarations_type::iterator,bool> r =
            decls.insert(
                declarations_type::value_type(
                    m_pool.intern(name).first, m_cur_attrs));

        if (!r.second)
            // Insertion failed.
            throw general_error("dom_tree::end_declaration: failed to insert a new declaration entry.");
    }
    else
        it->second = m_cur_attrs;

    m_cur_attrs.clear();
}

void dom_tree::impl::start_element(const sax_ns_parser_element& elem)
{
    xmlns_id_t ns = elem.ns;
    const pstring& name = elem.name;

    // These strings must be persistent.
    pstring name_safe = m_pool.intern(name).first;

    dom::element* p = nullptr;
    if (!m_root)
    {
        // This must be the root element!
        m_root = orcus::make_unique<dom::element>(ns, name_safe);
        m_elem_stack.push_back(m_root.get());
        p = m_elem_stack.back();
        p->attrs.swap(m_cur_attrs);
        return;
    }

    // Append new element as a child element of the current element.
    p = m_elem_stack.back();
    p->child_nodes.push_back(orcus::make_unique<dom::element>(ns, name_safe));
    p = static_cast<dom::element*>(p->child_nodes.back().get());
    p->attrs.swap(m_cur_attrs);
    m_elem_stack.push_back(p);
}

void dom_tree::impl::end_element(const sax_ns_parser_element& elem)
{
    xmlns_id_t ns = elem.ns;
    const pstring& name = elem.name;

    const dom::element* p = m_elem_stack.back();
    if (p->name.ns != ns || p->name.name != name)
        throw general_error("non-matching end element.");

    m_elem_stack.pop_back();
}

void dom_tree::impl::characters(const pstring& val, bool transient)
{
    if (m_elem_stack.empty())
        // No root element has been encountered.  Ignore this.
        return;

    pstring val2 = val.trim();
    if (val2.empty())
        return;

    dom::element* p = m_elem_stack.back();
    val2 = m_pool.intern(val2).first; // Make sure the string is persistent.
    p->child_nodes.push_back(orcus::make_unique<dom::content>(val2));
}

void dom_tree::impl::set_attribute(xmlns_id_t ns, const pstring& name, const pstring& val)
{
    // These strings must be persistent.
    pstring name2 = m_pool.intern(name).first;
    pstring val2 = m_pool.intern(val).first;

    m_cur_attrs.push_back(dom::attr(ns, name2, val2));
}

void dom_tree::impl::doctype(const sax::doctype_declaration& dtd)
{
    m_doctype = orcus::make_unique<sax::doctype_declaration>(dtd);  // make a copy.

    sax::doctype_declaration& this_dtd = *m_doctype;
    string_pool& pool = m_pool;

    // Intern the strings.
    this_dtd.root_element = pool.intern(dtd.root_element).first;
    this_dtd.fpi = pool.intern(dtd.fpi).first;
    this_dtd.uri = pool.intern(dtd.uri).first;
}

dom_tree::dom_tree(xmlns_context& cxt) :
    mp_impl(orcus::make_unique<impl>(cxt)) {}

dom_tree::dom_tree(dom_tree&& other) :
    mp_impl(std::move(other.mp_impl))
{
    other.mp_impl = orcus::make_unique<impl>(mp_impl->m_ns_cxt);
}

dom_tree::~dom_tree() {}

void dom_tree::load(const std::string& strm)
{
    sax_ns_parser<impl> parser(
        strm.c_str(), strm.size(), mp_impl->m_ns_cxt, *mp_impl);
    parser.parse();
}

dom::const_node dom_tree::root() const
{
    const dom::element* p = mp_impl->m_root.get();
    auto v = orcus::make_unique<dom::const_node::impl>(p);
    return dom::const_node(std::move(v));
}

dom::const_node dom_tree::declaration(const pstring& name) const
{
    impl::declarations_type::const_iterator it = mp_impl->m_decls.find(name);
    if (it == mp_impl->m_decls.end())
        return dom::const_node();

    const dom::attrs_type* attrs = &it->second;
    auto v = orcus::make_unique<dom::const_node::impl>(attrs);
    return dom::const_node(std::move(v));
}

void dom_tree::swap(dom_tree& other)
{
    mp_impl.swap(other.mp_impl);
}

const sax::doctype_declaration* dom_tree::get_doctype() const
{
    return mp_impl->m_doctype.get();
}

namespace {

struct scope
{
    typedef std::vector<const dom::node*> nodes_type;
    string name;
    nodes_type nodes;
    nodes_type::const_iterator current_pos;

    scope(const scope&) = delete;
    scope& operator=(const scope&) = delete;

    scope(const string& _name, dom::node* _node) :
        name(_name)
    {
        nodes.push_back(_node);
        current_pos = nodes.begin();
    }

    scope(const string& _name) : name(_name) {}
};

typedef std::deque<scope> scopes_type;

void print_scope(ostream& os, const scopes_type& scopes)
{
    if (scopes.empty())
        throw general_error("scope stack shouldn't be empty while dumping tree.");

    // Skip the first scope which is root.
    scopes_type::const_iterator it = scopes.begin(), it_end = scopes.end();
    for (++it; it != it_end; ++it)
        os << "/" << it->name;
}

}

void dom_tree::dump_compact(ostream& os) const
{
    if (!mp_impl->m_root)
        return;

    // Dump namespaces first.
    mp_impl->m_ns_cxt.dump(os);

    scopes_type scopes;

    scopes.emplace_back(string(), mp_impl->m_root.get());
    while (!scopes.empty())
    {
        bool new_scope = false;

        // Iterate through all elements in the current scope.
        scope& cur_scope = scopes.back();
        for (; cur_scope.current_pos != cur_scope.nodes.end(); ++cur_scope.current_pos)
        {
            const dom::node* this_node = *cur_scope.current_pos;
            assert(this_node);
            print_scope(os, scopes);
            if (this_node->type == dom::node_type::content)
            {
                // This is a text content.
                this_node->print(os, mp_impl->m_ns_cxt);
                os << endl;
                continue;
            }

            assert(this_node->type == dom::node_type::element);
            const dom::element* elem = static_cast<const dom::element*>(this_node);
            os << "/";
            elem->print(os, mp_impl->m_ns_cxt);
            os << endl;

            {
                // Dump attributes.
                dom::attrs_type attrs = elem->attrs;
                std::sort(attrs.begin(), attrs.end(),
                      [](const dom::attr& left, const dom::attr& right) -> bool
                      {
                          return left.name.name < right.name.name;
                      }
                );

                for (const dom::attr& a : attrs)
                {
                    print_scope(os, scopes);
                    os << "/";
                    elem->print(os, mp_impl->m_ns_cxt);
                    os << "@";
                    dom::print(os, a, mp_impl->m_ns_cxt);
                    os << endl;
                }
            }

            if (elem->child_nodes.empty())
                continue;

            // This element has child nodes.  Push a new scope and populate it
            // with all child elements, but skip content nodes.
            scope::nodes_type nodes;
            for (const std::unique_ptr<dom::node>& p : elem->child_nodes)
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

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
