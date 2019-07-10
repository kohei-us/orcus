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
#include <vector>

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

    enum class node_type { unknown, array, object, cell_ref, range_field_ref };

    struct cell_reference_type
    {
        cell_position_t pos;

        cell_reference_type(const cell_position_t& _pos);
    };

    struct range_reference_type
    {
        cell_position_t pos;
        std::vector<const node*> fields;

        range_reference_type(const cell_position_t& _pos);
    };

    /** Represents a field within a range reference. */
    struct range_field_reference_type
    {
        range_reference_type* ref;
        spreadsheet::col_t column_pos;
    };

    struct node
    {
        node_type type = node_type::unknown;

        union
        {
            node_children_type* children = nullptr;
            cell_reference_type* cell_ref;
            range_field_reference_type* range_field_ref;

        } value;

        /**
         * The node is a row-group node (node that defines a row boundary)
         * if this value is set to a non-null value.  If this is not null, it
         * points to the range_reference instance it belongs to.
         */
        range_reference_type* row_group = nullptr;

        node(const node&) = delete;
        node& operator=(const node&) = delete;

        node();
        node(node&& other);

        node& get_or_create_child_node(long pos);
        node* get_child_node(long pos);
    };

    class walker
    {
        friend class json_map_tree;

        struct scope
        {
            const node* p;
            long array_position;

            scope(const node* _p);
        };

        using stack_type = std::vector<scope>;
        using unlinked_stack_type = std::vector<node_type>;

        const json_map_tree& m_parent;
        stack_type m_stack;
        unlinked_stack_type m_unlinked_stack;

        walker(const json_map_tree& parent);
    public:

        const node* push_node(node_type nt);
        const node* pop_node(node_type nt);
    };

    json_map_tree();
    ~json_map_tree();

    walker get_tree_walker() const;

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
    using range_ref_store_type = std::map<cell_position_t, range_reference_type>;

    boost::object_pool<node_children_type> m_node_children_pool;
    boost::object_pool<cell_reference_type> m_cell_ref_pool;
    boost::object_pool<range_field_reference_type> m_range_field_ref_pool;

    string_pool m_str_pool;

    std::unique_ptr<node> m_root;

    range_ref_store_type m_range_refs;

    struct
    {
        cell_position_t pos;
        std::vector<pstring> field_paths;
        std::vector<pstring> row_groups;

    } m_current_range;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
