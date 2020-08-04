/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_XML_MAP_TREE_HPP
#define INCLUDED_ORCUS_XML_MAP_TREE_HPP

#include "orcus/pstring.hpp"
#include "orcus/spreadsheet/types.hpp"
#include "orcus/exception.hpp"
#include "orcus/types.hpp"
#include "orcus/xml_namespace.hpp"
#include "orcus/string_pool.hpp"

#include "spreadsheet_impl_types.hpp"

#include <ostream>
#include <map>
#include <vector>
#include <deque>
#include <memory>

#include <boost/pool/object_pool.hpp>

namespace orcus {

class xmlns_repository;

/**
 * Tree structure representing XML-to-sheet mapping rules for mapped XML
 * import. This structure only contains linked elements and attributes and
 * their parent elements; it does not contain the entire structure of the
 * imported XML.
 */
class xml_map_tree
{
    struct range_field_link
    {
        pstring xpath;
        pstring label;

        range_field_link(const pstring& _xpath, const pstring& _label);
    };

public:
    /**
     * A single cell position.  Used both for single cell as well as range
     * links.  For a range link, this represents the upper-left cell of a
     * range.
     */
    using cell_position = spreadsheet::detail::cell_position_t;

    /**
     * Positions of opening and closing elements in xml stream.
     */
    struct element_position
    {
        std::ptrdiff_t open_begin;
        std::ptrdiff_t open_end;
        std::ptrdiff_t close_begin;
        std::ptrdiff_t close_end;

        element_position();
    };

    struct cell_reference
    {
        cell_position pos;

        cell_reference(const cell_reference&) = delete;
        cell_reference& operator=(const cell_reference&) = delete;

        cell_reference();
    };

    struct element;
    struct linkable;
    using element_store_type = std::deque<element*>;
    using element_list_type = std::vector<element*>;
    using const_element_list_type = std::vector<const element*>;

    struct range_reference
    {
        cell_position pos;

        /**
         * List of elements comprising the fields, in order of appearance from
         * left to right.
         */
        std::vector<const linkable*> field_nodes;

        /**
         * Total number of rows comprising data.  This does not include the
         * label row at the top.
         */
        spreadsheet::row_t row_position;

        range_reference(const range_reference&) = delete;
        range_reference& operator=(const range_reference&) = delete;

        range_reference(const cell_position& _pos);

        void reset();
    };

    struct field_in_range
    {
        range_reference* ref = nullptr;
        spreadsheet::col_t column_pos = -1;
    };

    typedef std::map<cell_position, range_reference*> range_ref_map_type;

    enum linkable_node_type { node_unknown, node_element, node_attribute };
    enum reference_type { reference_unknown, reference_cell, reference_range_field };
    enum element_type { element_unknown, element_linked, element_unlinked };

    struct linkable
    {
        xml_name_t name;
        linkable_node_type node_type;
        reference_type ref_type;

        union
        {
            cell_reference* cell_ref = nullptr;
            field_in_range* field_ref;
        };

        pstring label; // custom header label
        mutable pstring ns_alias; // namespace alias used in the content stream.

        linkable(const linkable&) = delete;
        linkable& operator=(const linkable&) = delete;

        linkable(xml_map_tree& parent, const xml_name_t& _name, linkable_node_type _node_type, reference_type _ref_type);
    };

    struct attribute : public linkable
    {
        using args_type = std::tuple<xml_map_tree&, const xml_name_t&, reference_type>;

        attribute(args_type args);
        ~attribute();
    };

    using attribute_store_type = std::deque<attribute*>;

    struct element : public linkable
    {
        element_type elem_type;
        element_store_type* child_elements;

        mutable element_position stream_pos; // position of this element in the content stream

        attribute_store_type attributes;

        /**
         * Points to a range reference instance of which this element is a
         * parent. nullptr if this element is not a parent element of any range
         * reference.
         */
        range_reference* range_parent;

        /**
         * The element is a row-group element (element that defines a row
         * boundary) if this value is not null.  If this is not null, it
         * points to the range_reference instance it belongs to.
         */
        range_reference* row_group;

        spreadsheet::row_t row_group_position;

        std::vector<spreadsheet::col_t> linked_range_fields;

        using args_type = std::tuple<xml_map_tree&, const xml_name_t&, element_type, reference_type>;

        element(args_type args);
        ~element();

        element* get_child(const xml_name_t& _name);

        element* get_or_create_child(
            xml_map_tree& parent, const xml_name_t& _name);

        element* get_or_create_linked_child(
            xml_map_tree& parent, const xml_name_t& _name, reference_type _ref_type);

        void link_reference(xml_map_tree& parent, reference_type _ref_type);

        /**
         * Unlinked attribute anchor is an element that's not linked but has
         * one or more attributes that are linked.
         *
         * @return true if the element is an unlinked attribute anchor, false
         *         otherwise.
         */
        bool unlinked_attribute_anchor() const;
    };

    friend struct linkable;

public:

    /**
     * Wrapper class to allow walking through the element tree.
     */
    class walker
    {
        typedef std::vector<element*> ref_element_stack_type;
        typedef std::vector<xml_name_t> name_stack_type;
        const xml_map_tree& m_parent;
        ref_element_stack_type m_stack;
        name_stack_type m_unlinked_stack;
    public:
        walker(const xml_map_tree& parent);
        walker(const walker& r);

        void reset();
        element* push_element(const xml_name_t& name);
        element* pop_element(const xml_name_t& name);
    };

    xml_map_tree() = delete;
    xml_map_tree(const xml_map_tree&) = delete;
    xml_map_tree& operator=(const xml_map_tree&) = delete;

    xml_map_tree(xmlns_repository& xmlns_repo);
    ~xml_map_tree();

    void set_namespace_alias(const pstring& alias, const pstring& uri, bool default_ns);
    xmlns_id_t get_namespace(const pstring& alias) const;

    void set_cell_link(const pstring& xpath, const cell_position& ref);

    void start_range(const cell_position& pos);
    void append_range_field_link(const pstring& xpath, const pstring& label);
    void set_range_row_group(const pstring& xpath);
    void commit_range();

    const linkable* get_link(const pstring& xpath) const;

    walker get_tree_walker() const;

    range_ref_map_type& get_range_references();

    pstring intern_string(const pstring& str) const;

private:
    void insert_range_field_link(
        range_reference& range_ref, element_list_type& range_parent, const range_field_link& field);

    range_reference* get_range_reference(const cell_position& pos);

    void create_ref_store(linkable& node);

    struct linked_node_type
    {
        element_list_type elem_stack;
        linkable* node = nullptr;
        element* anchor_elem = nullptr;
    };

    /**
     * Get a linked node (element or attribute) referenced by the specified
     * xpath.
     *
     * @param xpath path to the linked node.
     * @param type type of reference, either a cell or a range field.
     */
    linked_node_type get_linked_node(const pstring& xpath, reference_type type);

    element* get_element(const pstring& xpath);

private:

    using range_field_links = std::vector<range_field_link>;

    xmlns_context m_xmlns_cxt;

    /**
     * Stores field links to insert into the current range reference.
     */
    range_field_links m_cur_range_field_links;

    cell_position m_cur_range_pos;

    /**
     * All range references present in the tree.  This container manages the
     * life cycles of stored range references.
     */
    range_ref_map_type m_field_refs;

    /** pool of element names. */
    mutable string_pool m_names;

    boost::object_pool<element_store_type> m_element_store_pool;
    boost::object_pool<cell_reference> m_cell_reference_pool;
    boost::object_pool<range_reference> m_range_reference_pool;
    boost::object_pool<field_in_range> m_field_in_range_pool;
    boost::object_pool<attribute> m_attribute_pool;
    boost::object_pool<element> m_element_pool;

    element* mp_root;

    xmlns_id_t m_default_ns;
};

std::ostream& operator<< (std::ostream& os, const xml_map_tree::linkable& link);

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
