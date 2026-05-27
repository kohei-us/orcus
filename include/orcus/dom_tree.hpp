/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "types.hpp"

#include <vector>
#include <ostream>
#include <memory>

namespace orcus {

class xmlns_context;
class string_pool;

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
    processing_instruction,
};

struct ORCUS_DLLPUBLIC entity_name
{
    xmlns_id_t ns;
    std::string_view name;

    entity_name();
    entity_name(std::string_view _name);
    entity_name(xmlns_id_t _ns, std::string_view _name);

    bool operator== (const entity_name& other) const;
    std::strong_ordering operator<=> (const entity_name& other) const;
};

/**
 * Read-only handle into an XML DOM tree. Exposes accessors for inspecting a
 * single node (its type, name, attributes, children, and parent) without
 * allowing modification.
 *
 * @note A const_node is a lightweight value-type handle into storage owned by
 *       the enclosing document_tree. Do not use a const_node after its
 *       document_tree has been destroyed.
 */
class ORCUS_DLLPUBLIC const_node
{
    friend class document_tree;
    friend class node;

protected:
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
 * Mutable handle into an XML DOM tree. Inherits all read-only accessors from
 * const_node and adds methods to build and edit a tree in place.
 *
 * @note A node is a lightweight value-type handle into storage owned by the
 *       enclosing document_tree. Do not use a node after its document_tree has
 *       been destroyed.
 */
class ORCUS_DLLPUBLIC node : public const_node
{
    friend class document_tree;

    node(std::unique_ptr<impl>&& _impl, string_pool* pool);

public:
    node();
    node(const node& other);
    node(node&& other);

    ~node();

    node& operator= (const node& other);

    /**
     * Append a new child element to this element.
     *
     * @param name name of the new child element.
     *
     * @return mutable handle to the newly created child element.
     *
     * @throw std::invalid_argument if this node is not an element.
     */
    node append_element(entity_name name);

    /**
     * Append a text content node to this element.
     *
     * @param value text content.
     *
     * @throw std::invalid_argument if this node is not an element.
     */
    void append_content(std::string_view value);

    /**
     * Append a comment node to this element.
     *
     * @param value comment text (without the surrounding `<!--` `-->`).
     *
     * @throw std::invalid_argument if this node is not an element.
     */
    void append_comment(std::string_view value);

    /**
     * Set or update a namespaced attribute on this element. If an attribute
     * with the same namespace-name pair already exists, its value is replaced.
     *
     * @param name attribute name (with namespace).
     * @param value attribute value.
     *
     * @throw std::invalid_argument if this node is not an element.
     */
    void set_attribute(entity_name name, std::string_view value);

    /**
     * Set or update an attribute that has no explicit namespace. Use this for
     * the XML declaration, processing instructions, and element attributes
     * without a namespace prefix. If an attribute with the same name already
     * exists, its value is replaced.
     *
     * @param name attribute name.
     * @param value attribute value.
     *
     * @throw std::invalid_argument if this node is unset.
     */
    void set_attribute(std::string_view name, std::string_view value);

    /**
     * Change the name of this element.
     *
     * @throw std::invalid_argument if this node is not an element.
     */
    void set_name(entity_name name);

    /**
     * Declare a namespace prefix on this element. An empty alias declares a
     * default namespace.
     *
     * @param alias namespace prefix (or empty for the default namespace).
     * @param ns namespace identifier to bind to the prefix.
     *
     * @throw std::invalid_argument if this node is not an element.
     */
    void declare_namespace(std::string_view alias, xmlns_id_t ns);
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

    /**
     * Replace the current root element with a fresh empty element of the
     * given name. Any pre-existing root subtree is discarded. Other parts of
     * the document (XML declaration, processing instructions, prolog/epilog
     * comments, DOCTYPE) are preserved.
     *
     * @param name name of the new root element.
     *
     * @return mutable handle to the newly created root element.
     */
    dom::node set_root(entity_name name);

    dom::const_node declaration() const;

    /**
     * Create the XML declaration if it does not already exist, and return a
     * mutable handle to it. Use the returned handle's set_attribute(name,
     * value) overload to populate `version`, `encoding`, and `standalone`.
     */
    dom::node set_declaration();

    dom::const_node processing_instruction(std::string_view target) const;

    /**
     * Get or create a processing instruction with the given target, returning
     * a mutable handle. If a processing instruction with the same target
     * already exists, the existing one is returned unchanged.
     */
    dom::node add_processing_instruction(std::string_view target);

    /**
     * Append a comment node before the root element (in the document prolog).
     */
    void append_prolog_comment(std::string_view value);

    /**
     * Append a comment node after the root element (in the document epilog).
     */
    void append_epilog_comment(std::string_view value);

    [[deprecated("use processing_instruction(target), or declaration() for the XML declaration")]]
    dom::const_node declaration(std::string_view name) const;

    /**
     * Swap the content with another dom_tree instance.
     *
     * @param other the dom_tree instance to swap the content with.
     */
    void swap(document_tree& other);

    const sax::doctype_declaration* get_doctype() const;

    /**
     * Dump the XML document tree to string.
     *
     * @param indent Number of whitespace characters to use for one indent
     *               level.  Note that specifying the indent value of 0 will
     *               generate output without line breaks.
     *
     * @return A string representation of the XML document tree.
     */
    std::string dump(std::size_t indent) const;

    void dump_compact(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const entity_name& v);

} // namespace dom

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
