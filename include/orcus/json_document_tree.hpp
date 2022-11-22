/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_JSON_DOCUMENT_TREE_HPP
#define INCLUDED_ORCUS_JSON_DOCUMENT_TREE_HPP

#include "env.hpp"
#include "exception.hpp"

#include <string>
#include <memory>
#include <vector>

namespace orcus {

struct json_config;

namespace json {

struct json_value;
struct document_resource;
class document_tree;

/**
 * Exception related to JSON document tree construction.
 */
class ORCUS_DLLPUBLIC document_error : public general_error
{
public:
    document_error(const std::string& msg);
    virtual ~document_error();
};

/**
 * Exception that gets thrown due to ambiguity when you specify a braced
 * list that can be interpreted either as a key-value pair inside an object
 * or as values of an array.
 */
class ORCUS_DLLPUBLIC key_value_error : public document_error
{
public:
    key_value_error(const std::string& msg);
    virtual ~key_value_error();
};

enum class node_t : uint8_t
{
    /** node type is not set.  */
    unset = 0,
    /** JSON string node.  A node of this type contains a string value. */
    string = 1,
    /** JSON number node. A node of this type contains a numeric value. */
    number = 2,
    /**
     * JSON object node.  A node of this type contains one or more key-value
     * pairs.
     */
    object = 3,
    /**
     * JSON array node.  A node of this type contains one or more child nodes.
     */
    array = 4,
    /**
     * JSON boolean node containing a value of 'true'.
     */
    boolean_true = 5,
    /**
     * JSON boolean node containing a value of 'false'.
     */
    boolean_false = 6,
    /**
     * JSON node containing a 'null' value.
     */
    null = 7,
};

namespace detail { namespace init { class node; }}

class const_node;
class document_tree;

class ORCUS_DLLPUBLIC const_node_iterator
{
    friend class const_node;

    struct impl;
    std::unique_ptr<impl> mp_impl;

    const_node_iterator(const document_tree* doc, const const_node& v, bool begin);

public:
    const_node_iterator();
    const_node_iterator(const const_node_iterator& other);
    ~const_node_iterator();

    const const_node& operator*() const;
    const const_node* operator->() const;

    const_node_iterator& operator++();
    const_node_iterator operator++(int);

    const_node_iterator& operator--();
    const_node_iterator operator--(int);

    bool operator== (const const_node_iterator& other) const;
    bool operator!= (const const_node_iterator& other) const;

    const_node_iterator& operator= (const const_node_iterator& other);
};

/**
 * Each node instance represents a JSON value stored in the document tree.
 * It's immutable.
 */
class ORCUS_DLLPUBLIC const_node
{
    friend class document_tree;
    friend class const_node_iterator;

protected:
    struct impl;
    std::unique_ptr<impl> mp_impl;

    const_node(const document_tree* doc, json_value* jv);
    const_node(std::unique_ptr<impl>&& p);
public:
    const_node() = delete;

    const_node(const const_node& other);
    const_node(const_node&& rhs);
    ~const_node();

    /**
     * Get the type of a node.
     *
     * @return node type.
     */
    node_t type() const;

    /**
     * Get the number of child nodes if any.
     *
     * @return number of child nodes.
     */
    size_t child_count() const;

    /**
     * Get a list of keys stored in a JSON object node.
     *
     * @exception orcus::json::document_error if the node is not of the object
     *                 type.
     * @return a list of keys.
     */
    std::vector<std::string_view> keys() const;

    /**
     * Get the key by index in a JSON object node.  This method works only
     * when the <b>preserve object order</b> option is set.
     *
     * @param index 0-based key index.
     *
     * @exception orcus::json::document_error if the node is not of the object
     *                 type.
     *
     * @exception std::out_of_range if the index is equal to or greater than
     *               the number of keys stored in the node.
     *
     * @return key value.
     */
    std::string_view key(size_t index) const;

    /**
     * Query whether or not a particular key exists in a JSON object node.
     *
     * @param key key value.
     *
     * @return true if this object node contains the specified key, otherwise
     *         false.  If this node is not of a JSON object type, false is
     *         returned.
     */
    bool has_key(std::string_view key) const;
    /**
     * Get a child node by index.
     *
     * @param index 0-based index of a child node.
     *
     * @exception orcus::json::document_error if the node is not one of the
     *                 object or array types.
     *
     * @exception std::out_of_range if the index is equal to or greater than
     *               the number of child nodes that the node has.
     *
     * @return child node instance.
     */
    const_node child(size_t index) const;

    /**
     * Get a child node by textural key value.
     *
     * @param key textural key value to get a child node by.
     *
     * @exception orcus::json::document_error if the node is not of the object
     *                 type, or the node doesn't have the specified key.
     *
     * @return child node instance.
     */
    const_node child(std::string_view key) const;

    /**
     * Get the parent node.
     *
     * @exception orcus::json::document_error if the node doesn't have a parent
     *                 node which implies that the node is a root node.
     *
     * @return parent node instance.
     */
    const_node parent() const;

    /**
     * Get the last child node.
     *
     * @exception orcus::json::document_error if the node is not of array type
     *                 or node has no children.
     *
     * @return last child node instance.
     */
    const_node back() const;

    /**
     * Get the string value of a JSON string node.
     *
     * @exception orcus::json::document_error if the node is not of the string
     *                 type.
     *
     * @return string value.
     */
    std::string_view string_value() const;

    /**
     * Get the numeric value of a JSON number node.
     *
     * @exception orcus::json::document_error if the node is not of the number
     *                 type.
     *
     * @return numeric value.
     */
    double numeric_value() const;

    const_node& operator=(const const_node& other);
    const_node& operator=(const_node&& other);

    /**
     * Return an indentifier of the JSON value object that the node
     * represents.  The identifier is derived directly from the memory address
     * of the value object.
     *
     * @return identifier of the JSON value object.
     */
    uintptr_t identity() const;

    const_node_iterator begin() const;
    const_node_iterator end() const;
};

/**
 * Each node instance represents a JSON value stored in the document tree.
 * This class allows mutable operations.
 */
class ORCUS_DLLPUBLIC node : public const_node
{
    friend class document_tree;

    node(const document_tree* doc, json_value* jv);
    node(const_node&& rhs);

public:
    node() = delete;

    node(const node& other);
    node(node&& rhs);
    ~node();

    node& operator=(const node& other);
    node& operator=(const detail::init::node& v);
    node operator[](std::string_view key);

    /**
     * Get a child node by index.
     *
     * @param index 0-based index of a child node.
     *
     * @exception orcus::json::document_error if the node is not one of the
     *                 object or array types.
     *
     * @exception std::out_of_range if the index is equal to or greater than
     *               the number of child nodes that the node has.
     *
     * @return child node instance.
     */
    node child(size_t index);

    /**
     * Get a child node by textural key value.
     *
     * @param key textural key value to get a child node by.
     *
     * @exception orcus::json::document_error if the node is not of the object
     *                 type, or the node doesn't have the specified key.
     *
     * @return child node instance.
     */
    node child(std::string_view key);

    /**
     * Get the parent node.
     *
     * @exception orcus::json::document_error if the node doesn't have a parent
     *                 node which implies that the node is a root node.
     *
     * @return parent node instance.
     */
    node parent();

    /**
     * Get the last child node.
     *
     * @exception orcus::json::document_error if the node is not of array type
     *                 or node has no children.
     *
     * @return last child node instance.
     */
    node back();

    /**
     * Append a new node value to the end of the array.
     *
     * @exception orcus::json::document_error if the node is not of array
     *                 type.
     * @param v new node value to append to the end of the array.
     */
    void push_back(const detail::init::node& v);
};

/**
 * This class represents a JSON array, to be used to explicitly create an
 * array instance during initialization.
 */
class ORCUS_DLLPUBLIC array
{
    friend class detail::init::node;
    friend class document_tree;

    std::vector<detail::init::node> m_vs;
public:
    array();
    array(const array&) = delete;
    array(array&& other);
    array(std::initializer_list<detail::init::node> vs);
    ~array();
};

/**
 * This class represents a JSON object, primarily to be used to create an
 * empty object instance.
 */
class ORCUS_DLLPUBLIC object
{
public:
    object();
    object(const object&) = delete;
    object(object&& other);
    ~object();
};

namespace detail { namespace init {

/**
 * Node to store an initial value during document tree initialization.  It's
 * not meant to be instantiated explicitly.  A value passed from the braced
 * initialization list is implicitly converted to an instance of this class.
 */
class ORCUS_DLLPUBLIC node
{
    friend class ::orcus::json::document_tree;
    friend class ::orcus::json::node;

    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    node(double v);
    node(int v);
    node(bool b);
    node(std::nullptr_t);
    node(const char* p);
    node(const std::string& s);
    node(std::initializer_list<detail::init::node> vs);
    node(json::array array);
    node(json::object obj);

    node(const node& other) = delete;
    node(node&& other);
    ~node();

    node& operator= (node other) = delete;

private:
    node_t type() const;
    json_value* to_json_value(document_resource& res) const;
    void store_to_node(document_resource& res, json_value* parent) const;
};

}}

/**
 * This class stores a parsed JSON document tree structure.
 */
class ORCUS_DLLPUBLIC document_tree
{
    friend class const_node;
    friend class node;

    struct impl;
    std::unique_ptr<impl> mp_impl;

    const document_resource& get_resource() const;

public:
    document_tree();
    document_tree(const document_tree&) = delete;
    document_tree(document_tree&& other);
    document_tree(document_resource& res);
    document_tree(std::initializer_list<detail::init::node> vs);
    document_tree(array vs);
    document_tree(object obj);
    ~document_tree();

    document_tree& operator= (std::initializer_list<detail::init::node> vs);
    document_tree& operator= (array vs);
    document_tree& operator= (object obj);

    /**
     * Load raw string stream containing a JSON structure to populate the
     * document tree.
     *
     * @param stream stream containing a JSON structure.
     * @param config configuration object.
     */
    void load(std::string_view stream, const json_config& config);

    /**
     * Get the root node of the document.
     *
     * @return root node of the document.
     */
    json::const_node get_document_root() const;

    /**
     * Get the root node of the document.
     *
     * @return root node of the document.
     */
    json::node get_document_root();

    /**
     * Dump the JSON document tree to string.
     *
     * @return a string representation of the JSON document tree.
     */
    std::string dump() const;

    /**
     * Dump the JSON document tree to an XML structure.
     *
     * @return a string containing an XML structure representing the JSON
     *         content.
     */
    std::string dump_xml() const;

    /**
     * Dump the JSON document tree as YAML output.
     *
     * @return string containing a YAML output representing the JSON document
     *         tree structure.
     */
    std::string dump_yaml() const;

    /**
     * Swap the content of the document with another document instance.
     *
     * @param other document instance to swap the content with.
     */
    void swap(document_tree& other);
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
