/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "spreadsheet_impl_types.hpp"
#include "orcus/string_pool.hpp"
#include "orcus/exception.hpp"

#include <boost/pool/object_pool.hpp>
#include <memory>
#include <map>

namespace orcus {

using spreadsheet::detail::cell_position_t;

class json_map_tree
{
public:
    static constexpr long node_child_default_position = -1;

    /**
     * Error indicating improper path.
     */
    class path_error : public general_error
    {
    public:
        path_error(const std::string& msg);
    };

    struct node;
    using node_children_type = std::map<long, node>;

    enum class node_type { unknown, array, object, cell_ref };

    struct cell_reference_type
    {
        cell_position_t pos;

        cell_reference_type(const cell_position_t& _pos);
    };

    struct node
    {
        node_type type = node_type::unknown;

        union
        {
            node_children_type* children = nullptr;
            cell_reference_type* cell_ref;

        } value;

        node(const node&) = delete;
        node& operator=(const node&) = delete;

        node();
        node(node&& other);

        node& get_or_create_child_node(long pos);
    };

    json_map_tree();
    ~json_map_tree();

    void set_cell_link(const pstring& path, const cell_position_t& pos);

    const node* get_link(const pstring& path) const;

    void start_range(const cell_position_t& pos);
    void append_field_link(const pstring& path);
    void set_range_row_group(const pstring& path);
    void commit_range();

private:
    const node* get_destination_node(const pstring& path) const;
    node* get_or_create_destination_node(const pstring& path);

private:
    boost::object_pool<node_children_type> m_node_children_pool;
    boost::object_pool<cell_reference_type> m_cell_ref_pool;
    string_pool m_str_pool;
    std::unique_ptr<node> m_root;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
