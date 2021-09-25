/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_DOM_TREE_HPP
#define INCLUDED_ORCUS_DOM_TREE_HPP

#include "types.hpp"

#include <vector>
#include <ostream>
#include <memory>

namespace orcus {

class xmlns_context;

namespace sax {

struct doctype_declaration;

}

namespace dom {

class document_tree;

enum class node_t : uint8_t
{
    unset,
    declaration,
    element,
};

struct ORCUS_DLLPUBLIC entity_name
{
    xmlns_id_t ns;
    std::string_view name;

    entity_name();
    entity_name(std::string_view _name);
    entity_name(xmlns_id_t _ns, std::string_view _name);

    bool operator== (const entity_name& other) const;
    bool operator!= (const entity_name& other) const;
};

class ORCUS_DLLPUBLIC const_node
{
    friend class document_tree;

    struct impl;
    std::unique_ptr<impl> mp_impl;

    const_node(std::unique_ptr<impl>&& _impl);
public:
    const_node();
    const_node(const const_node& other);
    const_node(const_node&& other);

    ~const_node();

    node_t type() const;

    size_t child_count() const;

    const_node child(size_t index) const;

    entity_name name() const;

    std::string_view attribute(const entity_name& name) const;
    std::string_view attribute(std::string_view name) const;

    size_t attribute_count() const;

    const_node parent() const;

    void swap(const_node& other);

    const_node& operator= (const const_node& other);

    bool operator== (const const_node& other) const;
    bool operator!= (const const_node& other) const;
};

/**
 * Ordinary DOM tree representing the content of an XML document.
 */
class ORCUS_DLLPUBLIC document_tree
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    document_tree(const document_tree&) = delete;
    document_tree& operator= (const document_tree&) = delete;

    document_tree(xmlns_context& cxt);
    document_tree(document_tree&& other);
    ~document_tree();

    /**
     * Parse a given XML stream and build the content tree.
     *
     * @param strm XML stream.
     */
    void load(std::string_view strm);

    dom::const_node root() const;

    dom::const_node declaration(std::string_view name) const;

    /**
     * Swap the content with another dom_tree instance.
     *
     * @param other the dom_tree instance to swap the content with.
     */
    void swap(document_tree& other);

    const sax::doctype_declaration* get_doctype() const;

    void dump_compact(std::ostream& os) const;
};

} // namespace dom

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
