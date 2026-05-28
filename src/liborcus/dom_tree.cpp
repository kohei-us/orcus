/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dom_tree_impl.hpp"
#include <orcus/exception.hpp>

#include <cassert>
#include <format>
#include <algorithm>
#include <stdexcept>
#include <variant>

namespace orcus {

namespace dom {

entity_name::entity_name() : ns(XMLNS_UNKNOWN_ID) {}

entity_name::entity_name(std::string_view _name) :
    ns(XMLNS_UNKNOWN_ID), name(_name) {}

entity_name::entity_name(xmlns_id_t _ns, std::string_view _name) :
    ns(_ns), name(_name) {}

bool entity_name::operator== (const entity_name& other) const = default;

std::strong_ordering entity_name::operator<=> (const entity_name& other) const
{
    if (auto cmp = std::compare_three_way{}(ns, other.ns); cmp != 0)
        return cmp;

    return name <=> other.name;
}

struct const_node::impl
{
    using value_type = std::variant<
        std::monostate,
        detail::processing_instruction*,
        detail::element*>;

    node_t type;
    value_type value;
    string_pool* pool; //< non-null only for handles obtained via the mutable API
    detail::namespace_set* ns_set; //< non-null only for handles obtained via the mutable API

    impl() : type(node_t::unset), pool(nullptr), ns_set(nullptr) {}

    impl(detail::element* _elem) :
        type(node_t::element), value(_elem), pool(nullptr), ns_set(nullptr) {}

    impl(detail::processing_instruction* _pi, node_t _type) :
        type(_type), value(_pi), pool(nullptr), ns_set(nullptr) {}

    /**
     * Ensure this node is an element and return a reference to it as an element
     * type.
     *
     * @param op Name of function where it is needed.
     */
    detail::element& require_element(std::string_view op)
    {
        if (type != node_t::element)
            throw std::invalid_argument(std::format("{}: node is not an element", op));

        return *std::get<detail::element*>(value);
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
            const auto* p = std::get<detail::element*>(mp_impl->value);
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
            auto* p = std::get<detail::element*>(mp_impl->value);

            size_t elem_pos = p->child_elem_positions.at(index);
            assert(elem_pos < p->child_nodes.size());

            detail::node* child_node = p->child_nodes[elem_pos].get();
            assert(child_node->type == detail::node_type::element);

            auto v = std::make_unique<impl>(static_cast<detail::element*>(child_node));
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
            const auto* p = std::get<detail::element*>(mp_impl->value);
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
            const auto* p = std::get<detail::element*>(mp_impl->value);
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
        case node_t::processing_instruction:
        {
            const auto* p = std::get<detail::processing_instruction*>(mp_impl->value);
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
        case node_t::processing_instruction:
        {
            const auto* p = std::get<detail::processing_instruction*>(mp_impl->value);
            return p->attrs.size();
        }
        case node_t::element:
        {
            const auto* p = std::get<detail::element*>(mp_impl->value);
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

    auto* p = std::get<detail::element*>(mp_impl->value)->parent;
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
    return mp_impl->type == other.mp_impl->type && mp_impl->value == other.mp_impl->value;
}

bool const_node::operator!= (const const_node& other) const
{
    return !operator==(other);
}

node::node(std::unique_ptr<impl>&& _impl, string_pool* pool, detail::namespace_set* ns_set) :
    const_node(std::move(_impl))
{
    mp_impl->pool = pool;
    mp_impl->ns_set = ns_set;
}

node::node() = default;
node::node(const node& other) = default;
node::node(node&& other) = default;
node::~node() = default;

node& node::operator= (const node& other) = default;

namespace {

void set_attr_on(detail::attrs_type& attrs, detail::attr_map_type& attr_map,
    entity_name name, std::string_view value)
{
    auto it = attr_map.find(name);
    if (it == attr_map.end())
    {
        std::size_t pos = attrs.size();
        attrs.emplace_back(name.ns, name.name, value);
        attr_map.emplace(entity_name(name.ns, name.name), pos);
    }
    else
    {
        attrs[it->second].value = value;
    }
}

} // anonymous namespace

node node::append_element(entity_name name)
{
    auto& parent_elem = mp_impl->require_element("dom::node::append_element");

    string_pool* pool = mp_impl->pool;
    name.name = pool->intern(name.name).first;

    std::size_t pos = parent_elem.child_nodes.size();
    parent_elem.child_elem_positions.push_back(pos);
    parent_elem.child_nodes.push_back(
        std::make_unique<detail::element>(name.ns, name.name));

    auto* child = static_cast<detail::element*>(parent_elem.child_nodes.back().get());
    child->parent = &parent_elem;

    auto v = std::make_unique<const_node::impl>(child);
    return node(std::move(v), pool, mp_impl->ns_set);
}

void node::append_content(std::string_view value)
{
    auto& parent_elem = mp_impl->require_element("dom::node::append_content");

    value = mp_impl->pool->intern(value).first;
    auto child = std::make_unique<detail::content>(value);
    child->parent = &parent_elem;
    parent_elem.child_nodes.push_back(std::move(child));
}

void node::append_comment(std::string_view value)
{
    auto& parent_elem = mp_impl->require_element("dom::node::append_comment");

    value = mp_impl->pool->intern(value).first;
    auto child = std::make_unique<detail::comment>(value);
    child->parent = &parent_elem;
    parent_elem.child_nodes.push_back(std::move(child));
}

void node::set_attribute(entity_name name, std::string_view value)
{
    auto& elem = mp_impl->require_element("dom::node::set_attribute");

    name.name = mp_impl->pool->intern(name.name).first;
    value = mp_impl->pool->intern(value).first;
    set_attr_on(elem.attrs, elem.attr_map, name, value);
}

void node::set_attribute(std::string_view name, std::string_view value)
{
    name = mp_impl->pool->intern(name).first;
    value = mp_impl->pool->intern(value).first;

    switch (mp_impl->type)
    {
        case node_t::element:
        {
            auto& elem = *std::get<detail::element*>(mp_impl->value);
            set_attr_on(elem.attrs, elem.attr_map,
                entity_name(XMLNS_UNKNOWN_ID, name), value);

            break;
        }
        case node_t::declaration:
        case node_t::processing_instruction:
        {
            auto& pi = *std::get<detail::processing_instruction*>(mp_impl->value);
            set_attr_on(pi.attrs, pi.attr_map,
                entity_name(XMLNS_UNKNOWN_ID, name), value);

            break;
        }
        default:
            throw std::invalid_argument(
                "dom::node::set_attribute: node is unset");
    }
}

void node::set_name(entity_name name)
{
    auto& elem = mp_impl->require_element("dom::node::set_name");
    name.name = mp_impl->pool->intern(name.name).first;
    elem.name = name;
}

void node::declare_namespace(std::string_view alias, xmlns_id_t ns)
{
    auto& elem = mp_impl->require_element("dom::node::declare_namespace");
    alias = mp_impl->pool->intern(alias).first;
    elem.ns_decls.emplace_back(alias, ns);
    mp_impl->ns_set->add(ns);
}

document_tree::document_tree(xmlns_repository& repo) :
    mp_impl(std::make_unique<impl>(repo)) {}

document_tree::document_tree(document_tree&& other) :
    mp_impl(std::move(other.mp_impl))
{
    other.mp_impl = std::make_unique<impl>(mp_impl->m_repo);
}

document_tree::~document_tree() {}

void document_tree::load(std::string_view strm)
{
    // discard any prior tree state so the parser builds a fresh document
    mp_impl->m_doctype.reset();
    mp_impl->m_cur_pi_target = {};
    mp_impl->m_xml_decl.reset();
    mp_impl->m_pis.clear();
    mp_impl->m_doc_attrs.clear();
    mp_impl->m_cur_attrs.clear();
    mp_impl->m_cur_attr_map.clear();
    mp_impl->m_cur_ns_decls.clear();
    mp_impl->m_elem_stack.clear();
    mp_impl->m_root.reset();
    mp_impl->m_prolog_comments.clear();
    mp_impl->m_epilog_comments.clear();
    mp_impl->m_namespaces.clear();

    // the parser context lives only for the duration of parsing; afterwards we
    // copy out the namespaces it observed and discard the context
    xmlns_context ns_cxt = mp_impl->m_repo.create_context();
    sax_ns_parser<impl> parser(strm, ns_cxt, *mp_impl);
    parser.parse();

    for (xmlns_id_t ns : ns_cxt.get_all_namespaces())
        mp_impl->m_namespaces.add(ns);
}

dom::const_node document_tree::root() const
{
    detail::element* p = mp_impl->m_root.get();
    auto v = std::make_unique<const_node::impl>(p);
    return dom::const_node(std::move(v));
}

dom::node document_tree::set_root(entity_name name)
{
    name.name = mp_impl->m_pool.intern(name.name).first;
    mp_impl->m_root = std::make_unique<detail::element>(name.ns, name.name);
    mp_impl->m_elem_stack.clear();

    auto v = std::make_unique<const_node::impl>(mp_impl->m_root.get());
    return dom::node(std::move(v), &mp_impl->m_pool, &mp_impl->m_namespaces);
}

dom::const_node document_tree::declaration() const
{
    if (!mp_impl->m_xml_decl)
        return dom::const_node();

    auto v = std::make_unique<dom::const_node::impl>(&*mp_impl->m_xml_decl, node_t::declaration);
    return dom::const_node(std::move(v));
}

dom::node document_tree::set_declaration()
{
    if (!mp_impl->m_xml_decl)
        mp_impl->m_xml_decl.emplace();

    auto v = std::make_unique<dom::const_node::impl>(
        &*mp_impl->m_xml_decl, node_t::declaration);

    return dom::node(std::move(v), &mp_impl->m_pool, &mp_impl->m_namespaces);
}

dom::const_node document_tree::processing_instruction(std::string_view target) const
{
    auto it = mp_impl->m_pis.find(target);
    if (it == mp_impl->m_pis.end())
        return dom::const_node();

    auto v = std::make_unique<dom::const_node::impl>(&it->second, node_t::processing_instruction);
    return dom::const_node(std::move(v));
}

dom::node document_tree::add_processing_instruction(std::string_view target)
{
    target = mp_impl->m_pool.intern(target).first;
    auto [it, inserted] = mp_impl->m_pis.try_emplace(target);
    (void)inserted;

    auto v = std::make_unique<dom::const_node::impl>(
        &it->second, node_t::processing_instruction);

    return dom::node(std::move(v), &mp_impl->m_pool, &mp_impl->m_namespaces);
}

void document_tree::append_prolog_comment(std::string_view value)
{
    value = mp_impl->m_pool.intern(value).first;
    mp_impl->m_prolog_comments.emplace_back(value);
}

void document_tree::append_epilog_comment(std::string_view value)
{
    value = mp_impl->m_pool.intern(value).first;
    mp_impl->m_epilog_comments.emplace_back(value);
}

dom::const_node document_tree::declaration(std::string_view name) const
{
    if (name == "xml")
        return declaration();

    auto it = mp_impl->m_pis.find(name);
    if (it == mp_impl->m_pis.end())
        return dom::const_node();

    auto v = std::make_unique<dom::const_node::impl>(&it->second, node_t::declaration);
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

class compact_dumper : public tree_walker
{
    std::ostream& m_os;
    const xmlns_repository& m_repo;

    void print_path(const detail::element& elem)
    {
        std::vector<const detail::element*> path;
        for (const detail::element* p = &elem; p; p = p->parent)
            path.push_back(p);

        for (auto it = path.rbegin(); it != path.rend(); ++it)
        {
            m_os << "/";
            detail::print(m_os, (*it)->name, m_repo);
        }
    }

public:
    compact_dumper(const detail::element& root, std::ostream& os, const xmlns_repository& repo) :
        tree_walker(root), m_os(os), m_repo(repo) {}

protected:
    void on_element_enter(const detail::element& elem, std::size_t /*depth*/) override
    {
        print_path(elem);
        m_os << "\n";

        // dump attributes sorted by name
        detail::attrs_type attrs = elem.attrs;
        std::sort(attrs.begin(), attrs.end(),
            [](const detail::attr& left, const detail::attr& right) {
                return left.name.name < right.name.name;
            });

        for (const detail::attr& a : attrs)
        {
            // print path, element then the attribute
            print_path(elem);
            m_os << "@";
            detail::print(m_os, a, m_repo);
            m_os << "\n";
        }
    }

    void on_content(const detail::content& c, std::size_t /*depth*/) override
    {
        // print the value of this content node
        assert(c.parent);
        print_path(*c.parent);
        detail::print(m_os, c, m_repo);
        m_os << "\n";
    }
};

}

void document_tree::dump_compact(std::ostream& os) const
{
    if (!mp_impl->m_root)
        return;

    for (xmlns_id_t ns : mp_impl->m_namespaces.all)
    {
        std::size_t index = mp_impl->m_repo.get_index(ns);
        if (index == INDEX_NOT_FOUND)
            continue;

        os << "ns" << index << "=\"" << ns << '"' << std::endl;
    }

    compact_dumper walker(*mp_impl->m_root, os, mp_impl->m_repo);
    walker.run();
}

std::ostream& operator<<(std::ostream& os, const entity_name& v)
{
    os << "entity-name(ns=";

    if (!v.ns)
        os << "none";
    else
        os << "'" << v.ns << "'";

    os << "; name='" << v.name << "')";
    return os;
}

} // namespace dom

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
