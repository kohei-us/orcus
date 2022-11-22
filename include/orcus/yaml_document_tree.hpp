/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_YAML_DOCUMENT_TREE_HPP
#define INCLUDED_ORCUS_YAML_DOCUMENT_TREE_HPP

#include "env.hpp"
#include "exception.hpp"

#include <string>
#include <memory>
#include <vector>

namespace orcus {

namespace yaml {

class document_tree;

class ORCUS_DLLPUBLIC document_error : public general_error
{
public:
    document_error(const std::string& msg);
    virtual ~document_error();
};

enum class node_t : uint8_t
{
    unset,
    string,
    number,
    map,
    sequence,
    boolean_true,
    boolean_false,
    null
};

struct yaml_value;

class ORCUS_DLLPUBLIC const_node
{
    friend class ::orcus::yaml::document_tree;

    struct impl;
    std::unique_ptr<impl> mp_impl;

    const_node(const yaml_value* yv);

public:
    const_node() = delete;

    const_node(const const_node& other);
    const_node(const_node&& rhs);
    ~const_node();

    node_t type() const;

    size_t child_count() const;

    std::vector<const_node> keys() const;

    const_node key(size_t index) const;

    const_node child(size_t index) const;

    const_node child(const const_node& key) const;

    const_node parent() const;

    std::string_view string_value() const;
    double numeric_value() const;

    const_node& operator=(const const_node& other);

    uintptr_t identity() const;
};

class ORCUS_DLLPUBLIC document_tree
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    document_tree();
    document_tree(const document_tree&) = delete;
    document_tree(document_tree&& other);
    ~document_tree();

    void load(std::string_view s);

    size_t get_document_count() const;

    const_node get_document_root(size_t index) const;

    std::string dump_yaml() const;

    std::string dump_json() const;
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
