/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <orcus/dom_tree.hpp>
#include <orcus/xml_namespace.hpp>
#include <orcus/string_pool.hpp>
#include <orcus/sax_ns_parser.hpp>

#include <ostream>
#include <vector>
#include <unordered_map>
#include <memory>

namespace orcus { namespace dom {

namespace detail {

struct attr
{
    entity_name name;
    std::string_view value;

    attr(xmlns_id_t _ns, std::string_view _name, std::string_view _value);
};

struct entity_name_hash
{
    std::size_t operator()(const entity_name& v) const;
};

using attrs_type = std::vector<attr>;
using attr_map_type = std::unordered_map<entity_name, std::size_t, entity_name_hash>;

struct declaration
{
    attrs_type attrs;
    attr_map_type attr_map;
};

enum class node_type { element, content };

struct element;

struct node
{
    const element* parent;
    node_type type;

    node(node_type _type) : parent(nullptr), type(_type) {}
    virtual ~node() = 0;
};

using nodes_type = std::vector<std::unique_ptr<node>>;

struct element : public node
{
    entity_name name;
    attrs_type attrs;
    attr_map_type attr_map;
    nodes_type child_nodes;
    std::vector<size_t> child_elem_positions;

    element() = delete;
    element(xmlns_id_t _ns, std::string_view _name);
    virtual ~element();
};

struct content : public node
{
    std::string_view value;

    content(std::string_view _value);
    virtual ~content();
};

void print(std::ostream& os, const entity_name& name, const xmlns_context& cxt);
void print(std::ostream& os, const attr& at, const xmlns_context& cxt);
void print(std::ostream& os, const element& elem, const xmlns_context& cxt);
void print(std::ostream& os, const content& c, const xmlns_context& cxt);

/**
 * Escape certain characters with backslash (\).
 */
void escape(std::ostream& os, std::string_view val);

} // namespace detail

struct document_tree::impl
{
    using element_stack_type = std::vector<detail::element*>;
    using declarations_type = std::unordered_map<std::string_view, detail::declaration>;

    xmlns_context& m_ns_cxt;
    string_pool m_pool;

    std::unique_ptr<sax::doctype_declaration> m_doctype;

    std::string_view m_cur_decl_name;
    declarations_type m_decls;
    detail::attrs_type m_doc_attrs;
    detail::attrs_type m_cur_attrs;
    detail::attr_map_type m_cur_attr_map;
    element_stack_type m_elem_stack;
    std::unique_ptr<detail::element> m_root;

    impl(xmlns_context& cxt) : m_ns_cxt(cxt) {}

    void start_declaration(std::string_view name)
    {
        m_cur_decl_name = name;
    }

    void end_declaration(std::string_view name);
    void start_element(const sax_ns_parser_element& elem);
    void end_element(const sax_ns_parser_element& elem);
    void characters(std::string_view val, bool transient);
    void doctype(const sax::doctype_declaration& dtd);

    void attribute(std::string_view name, std::string_view val)
    {
        set_attribute(XMLNS_UNKNOWN_ID, name, val);
    }

    void attribute(const sax_ns_parser_attribute& attr)
    {
        set_attribute(attr.ns, attr.name, attr.value);
    }

    void set_attribute(xmlns_id_t ns, std::string_view name, std::string_view val);
};

}} // namespace orcus::dom

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
