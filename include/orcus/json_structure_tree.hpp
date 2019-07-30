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
#include <vector>
#include <functional>

namespace orcus { namespace json {

struct ORCUS_DLLPUBLIC table_range_t
{
    std::vector<std::string> paths;
    std::vector<std::string> row_groups;
};

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

        /**
         * Move down to a child node at specified position.  Call
         * child_count() to get the number of child nodes the current node
         * has. A child node position is 0-based and must be less than the
         * child count.
         *
         * @param child_pos 0-based index of the child node to move down to.
         */
        void descend(size_t child_pos);

        /**
         * Move up to the parent node of the current node.
         */
        void ascend();

        /**
         * Return the number of child nodes the current node has.
         *
         * @return number of child nodes of the current node.
         */
        size_t child_count() const;

        /**
         * Get the properties of the current node.
         */
        node_properties get_node() const;

        /**
         * Build one or more field paths for the current value node.  For a
         * value node that is a child of an object, you'll always get one
         * path, whereas a value node that is a chlid of an array, you may get
         * more than one field paths.
         *
         * @return one or more field paths built for the current value node.
         */
        std::vector<std::string> build_field_paths() const;

        /**
         * Build a path for the parent of the current repeating node.  A row
         * group is an anchor to which repeating nodes get anchored to.  It is
         * used to determine when to increment row position during mapping.
         *
         * @return path for the row group of the current repeating node.
         */
        std::string build_row_group_path() const;
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

    using range_handler_type = std::function<void(table_range_t&&)>;

    void process_ranges(range_handler_type rh) const;
};

ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, structure_tree::node_type nt);

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
