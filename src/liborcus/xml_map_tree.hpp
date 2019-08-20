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
public:

    /**
     * Error indicating improper xpath syntax.
     */
    class xpath_error : public general_error
    {
    public:
        xpath_error(const std::string& msg);
    };

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
    typedef std::deque<std::unique_ptr<element>> element_store_type;
    typedef std::vector<element*> element_list_type;
    typedef std::vector<const element*> const_element_list_type;

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

    typedef std::map<cell_position, std::unique_ptr<range_reference>> range_ref_map_type;

    enum linkable_node_type { node_unknown, node_element, node_attribute };
    enum reference_type { reference_unknown, reference_cell, reference_range_field };
    enum element_type { element_unknown, element_linked, element_unlinked };

    struct linkable
    {
        xmlns_id_t ns;
        pstring name;
        linkable_node_type node_type;
        reference_type ref_type;

        union
        {
            cell_reference* cell_ref = nullptr;
            field_in_range* field_ref;
        };

        mutable pstring ns_alias; // namespace alias used in the content stream.

        linkable(const linkable&) = delete;
        linkable& operator=(const linkable&) = delete;

        linkable(xml_map_tree& parent, xmlns_id_t _ns, const pstring& _name, linkable_node_type _node_type, reference_type _ref_type);
    };

    struct attribute : public linkable
    {
        attribute(xml_map_tree& parent, xmlns_id_t _ns, const pstring& _name, reference_type _ref_type);
        ~attribute();
    };

    typedef std::vector<std::unique_ptr<attribute>> attribute_store_type;

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

        element(xml_map_tree& parent, xmlns_id_t _ns, const pstring& _name, element_type _elem_type, reference_type _ref_type);
        ~element();

        element* get_child(xmlns_id_t _ns, const pstring& _name);

        std::pair<element*, bool> get_or_create_child(
            xml_map_tree& parent, xmlns_id_t _ns, const pstring& _name);

        element* get_or_create_linked_child(
            xml_map_tree& parent, xmlns_id_t _ns, const pstring& _name, reference_type _ref_type);

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

    friend class linkable;

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
        element* push_element(xmlns_id_t ns, const pstring& name);
        element* pop_element(xmlns_id_t ns, const pstring& name);
    };

    xml_map_tree() = delete;
    xml_map_tree(const xml_map_tree&) = delete;
    xml_map_tree& operator=(const xml_map_tree&) = delete;

    xml_map_tree(xmlns_repository& xmlns_repo);
    ~xml_map_tree();

    void set_namespace_alias(const pstring& alias, const pstring& uri);
    xmlns_id_t get_namespace(const pstring& alias) const;

    void set_cell_link(const pstring& xpath, const cell_position& ref);

    void start_range();
    void append_range_field_link(const pstring& xpath, const cell_position& pos);
    void set_range_row_group(const pstring& xpath, const cell_position& pos);
    void commit_range();

    const linkable* get_link(const pstring& xpath) const;

    walker get_tree_walker() const;

    range_ref_map_type& get_range_references();

    pstring intern_string(const pstring& str) const;

private:
    range_reference* get_range_reference(const cell_position& pos);

    void create_ref_store(linkable& node);

    struct linked_node_type
    {
        element_list_type elem_stack;
        linkable* node;
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
    xmlns_context m_xmlns_cxt;

    /**
     * Element stack of current range parent element. This is used to
     * determine a common parent element for all field links of a current
     * range reference.
     */
    element_list_type m_cur_range_parent;

    range_reference* mp_cur_range_ref;

    /**
     * All range references present in the tree.  This container manages the
     * life cycles of stored range references.
     */
    range_ref_map_type m_field_refs;

    /** pool of element names. */
    mutable string_pool m_names;

    boost::object_pool<element_store_type> m_element_store_pool;
    boost::object_pool<cell_reference> m_cell_reference_pool;
    boost::object_pool<field_in_range> m_field_in_range_pool;

    std::unique_ptr<element> mp_root;
};

std::ostream& operator<< (std::ostream& os, const xml_map_tree::linkable& link);

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
