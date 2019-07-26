/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_JSON_STRUCTURE_TREE_HPP
#define INCLUDED_ORCUS_JSON_STRUCTURE_TREE_HPP

#include "orcus/env.hpp"
#include "orcus/types.hpp"

#include <ostream>
#include <memory>

namespace orcus { namespace json {

class ORCUS_DLLPUBLIC structure_tree
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:

    enum class node_type : short { unknown = 0, array = 1, object = 2, object_key = 3, value = 4 };

    struct node_properties
    {
        node_type type;
        bool repeat;
    };

    class ORCUS_DLLPUBLIC walker
    {
        friend class structure_tree;

        struct impl;
        std::unique_ptr<impl> mp_impl;

        walker(const structure_tree::impl* parent_impl);
    public:
        walker();
        walker(const walker& other);
        ~walker();

        /**
         * Set the current position to the root node, and return its
         * properties.
         */
        void root();

        void descend(size_t child_pos);

        void ascend();

        size_t child_count() const;

        node_properties get_node() const;

        std::vector<std::string> build_field_paths() const;

        std::string build_path_to_parent() const;
    };

    structure_tree(const structure_tree&) = delete;
    structure_tree& operator= (const structure_tree&) = delete;

    structure_tree();
    ~structure_tree();

    void parse(const char* p, size_t n);

    /**
     * For now, normalizing a tree just means sorting child nodes.  We may add
     * other normalization stuff later.
     */
    void normalize_tree();

    void dump_compact(std::ostream& os) const;

    walker get_walker() const;
};

ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, structure_tree::node_type nt);

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
