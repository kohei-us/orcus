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

void print(std::ostream& os, const element& elem, const xmlns_context& cxt)
{
    print(os, elem.name, cxt);
}

void print(std::ostream& os, const content& c, const xmlns_context& /*cxt*/)
{
    os << '"';
    escape(os, c.value);
    os << '"';
}

}}} // namespace orcus::dom::detail

namespace orcus { namespace dom {

tree_walker::scope::scope() = default;

tree_walker::scope::scope(const detail::node* node)
{
    nodes.push_back(node);
    current_pos = nodes.begin();
}

tree_walker::tree_walker(const detail::element& root) : m_root(root) {}

void tree_walker::run()
{
    std::deque<scope> scopes;
    scopes.emplace_back(&m_root);

    while (!scopes.empty())
    {
        scope& cur_scope = scopes.back();

        if (cur_scope.current_pos == cur_scope.nodes.end())
        {
            scopes.pop_back();
            continue;
        }

        const detail::node* this_node = *cur_scope.current_pos;
        ++cur_scope.current_pos;
        assert(this_node);

        if (this_node->type == detail::node_type::content)
            continue;

        const auto* elem = static_cast<const detail::element*>(this_node);

        if (elem->child_nodes.empty())
            continue;

        nodes_type child_nodes;
        for (const auto& p : elem->child_nodes)
            child_nodes.push_back(p.get());

        scopes.emplace_back();
        scope& child_scope = scopes.back();
        child_scope.nodes.swap(child_nodes);
        child_scope.current_pos = child_scope.nodes.begin();
    }
}

}} // namespace orcus::dom

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
