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
#include <iosfwd>

namespace orcus {

using spreadsheet::detail::cell_position_t;

class json_map_tree
{
public:
    using child_position_type = std::uintptr_t;

    static constexpr child_position_type node_child_default_position = -1;

    /**
     * Error indicating improper path.
     */
    class path_error : public general_error
    {
    public:
        path_error(const std::string& msg);
    };

    struct node;
    struct range_reference_type;
    using node_children_type = std::map<child_position_type, node>;
    using range_ref_store_type = std::map<cell_position_t, range_reference_type>;

    /** Types of nodes in the json input tree. */
    enum class input_node_type { unknown = 0x00, array = 0x01, object = 0x02, value = 0x04 };

    /**
     * Types of nodes in the map tree.  The lower 4-bits specify the input
     * node type which are kept in sync with the input_node_type values. The
     * next 4-bits specify the link type.
     */
    enum class map_node_type { unknown = 0x00, array = 0x01, object = 0x02, cell_ref = 0x14, range_field_ref = 0x24 };

    struct cell_reference_type
    {
        cell_position_t pos;

        cell_reference_type(const cell_position_t& _pos);
    };

    struct range_reference_type
    {
        cell_position_t pos;
        std::vector<const node*> fields;
        spreadsheet::row_t row_position;
        bool row_header;

        range_reference_type(const cell_position_t& _pos);
    };

    /** Represents a field within a range reference. */
    struct range_field_reference_type
    {
        range_reference_type* ref;
        spreadsheet::col_t column_pos;
        pstring label;
    };

    struct node
    {
        map_node_type type = map_node_type::unknown;

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

        std::vector<node*> anchored_fields;

        node(const node&) = delete;
        node& operator=(const node&) = delete;

        node();
        node(node&& other);

        node& get_or_create_child_node(child_position_type pos);
    };

    class walker
    {
        friend class json_map_tree;

        struct scope
        {
            node* p;
            child_position_type array_position;

            scope(node* _p);
        };

        using stack_type = std::vector<scope>;
        using unlinked_stack_type = std::vector<input_node_type>;

        const json_map_tree& m_parent;
        stack_type m_stack;
        unlinked_stack_type m_unlinked_stack;

        walker(const json_map_tree& parent);
    public:

        node* push_node(input_node_type nt);
        node* pop_node(input_node_type nt);

        void set_object_key(const char* p, size_t n);
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

    range_ref_store_type& get_range_references();

private:
    range_reference_type& get_range_reference(const cell_position_t& pos);

    const node* get_destination_node(const pstring& path) const;
    std::vector<node*> get_or_create_destination_node(const pstring& path);

    child_position_type to_key_position(const char* p, size_t n) const;

    static bool is_equivalent(input_node_type input_node, map_node_type map_node);

private:
    boost::object_pool<node_children_type> m_node_children_pool;
    boost::object_pool<cell_reference_type> m_cell_ref_pool;
    boost::object_pool<range_field_reference_type> m_range_field_ref_pool;

    mutable string_pool m_str_pool;

    std::unique_ptr<node> m_root;

    range_ref_store_type m_range_refs;

    struct
    {
        cell_position_t pos;
        std::vector<pstring> field_paths;
        std::vector<pstring> row_groups;

    } m_current_range;
};

std::ostream& operator<< (std::ostream& os, json_map_tree::input_node_type nt);
std::ostream& operator<< (std::ostream& os, json_map_tree::map_node_type nt);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
