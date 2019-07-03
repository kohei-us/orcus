/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "spreadsheet_impl_types.hpp"
#include "orcus/string_pool.hpp"

#include <boost/pool/object_pool.hpp>
#include <memory>
#include <map>

namespace orcus {

using spreadsheet::detail::cell_position_t;

class json_map_tree
{
public:

    struct node;
    using node_children_type = std::map<uint16_t, node>;

    enum class node_type { unknown, array, object, value };

    struct node
    {
        node_type type = node_type::unknown;

        node_children_type* children = nullptr;
        node* default_child = nullptr; /// used for non-mapped array children.
        bool linked = false;

        node(const node&) = delete;
        node& operator=(const node&) = delete;
    };

    json_map_tree();
    ~json_map_tree();

    void set_cell_link(const pstring& path, const cell_position_t& pos);

    void start_range(const cell_position_t& pos);
    void append_field_link(const pstring& path);
    void set_range_row_group(const pstring& path);
    void commit_range();

private:
    node* get_linked_node(const pstring& path);

private:
    boost::object_pool<node_children_type> m_node_children_pool;
    string_pool m_str_pool;
    std::unique_ptr<node> m_root;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
