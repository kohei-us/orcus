/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xml_map_tree.hpp"
#include "xpath_parser.hpp"

#define ORCUS_DEBUG_XML_MAP_TREE 0

#if ORCUS_DEBUG_XML_MAP_TREE
#include <iostream>
#endif

#include <cassert>
#include <algorithm>
#include <iostream>
#include <sstream>

namespace orcus {

namespace {

template<typename T>
void print_element_stack(std::ostream& os, const T& elem_stack)
{
    for (const auto& elem : elem_stack)
        os << '/' << elem->name.name;
}

} // anonymous namespace

xml_map_tree::range_field_link::range_field_link(std::string_view _xpath, std::string_view _label) :
    xpath(_xpath), label(_label) {}

xml_map_tree::element_position::element_position() :
    open_begin(0), open_end(0), close_begin(0), close_end(0) {}

xml_map_tree::cell_reference::cell_reference() {}

xml_map_tree::range_reference::range_reference(const cell_position& _pos) :
    pos(_pos), row_position(0) {}

xml_map_tree::linkable::linkable(
    xml_map_tree& parent, const xml_name_t& _name, linkable_node_type _node_type, reference_type _ref_type) :
    name(_name), node_type(_node_type), ref_type(_ref_type)
{
    parent.create_ref_store(*this);
}

xml_map_tree::attribute::attribute(args_type args) :
    linkable(std::get<0>(args), xml_name_t(std::get<1>(args)), linkable_node_type::attribute, std::get<2>(args)) {}

xml_map_tree::attribute::~attribute() {}

xml_map_tree::element::element(args_type args) :
    linkable(std::get<0>(args), std::get<1>(args), linkable_node_type::element, std::get<3>(args)),
    elem_type(std::get<2>(args)),
    child_elements(nullptr),
    range_parent(nullptr),
    row_group(nullptr),
    row_group_position(0)
{
    xml_map_tree& parent = std::get<0>(args);

    if (elem_type == element_type::unlinked)
    {
        child_elements = parent.m_element_store_pool.construct();
        return;
    }

    assert(elem_type == element_type::linked);
}

xml_map_tree::element::~element() {}

xml_map_tree::element* xml_map_tree::element::get_child(const xml_name_t& _name)
{
    if (elem_type != element_type::unlinked)
        return nullptr;

    assert(child_elements);

    auto it = std::find_if(
        child_elements->begin(), child_elements->end(),
        [&_name](const element* p) -> bool
        {
            return p->name == _name;
        }
    );

    return it == child_elements->end() ? nullptr : *it;
}

xml_map_tree::element* xml_map_tree::element::get_or_create_child(
    xml_map_tree& parent, const xml_name_t& _name)
{
    auto it = std::find_if(
        child_elements->begin(), child_elements->end(),
        [&_name](const element* p) -> bool
        {
            return p->name == _name;
        }
    );

    if (it != child_elements->end())
        return *it;

    string_pool& sp = parent.m_names;

    // Insert a new element of this name.
    auto const nm = sp.intern(_name.name).first; // work around LLVM < 7 libc++ bug
    child_elements->push_back(
        parent.m_element_pool.construct(
            element::args_type(
                parent,
                xml_name_t(_name.ns, nm),
                element_type::unlinked,
                reference_type::unknown
            )
        )
    );

    return child_elements->back();
}

xml_map_tree::element* xml_map_tree::element::get_or_create_linked_child(
    xml_map_tree& parent, const xml_name_t& _name, reference_type _ref_type)
{
    if (!child_elements)
    {
        assert(elem_type == element_type::linked);
        std::ostringstream os;
        constexpr xml_name_t::to_string_type type = xml_name_t::use_alias;

        os << "You can't add a child element under an already linked element (this='"
            << name.to_string(parent.m_xmlns_cxt, type) << "'; child='"
            << _name.to_string(parent.m_xmlns_cxt, type) << "')";

        throw invalid_map_error(os.str());
    }

    auto it = std::find_if(
        child_elements->begin(), child_elements->end(),
        [&_name](const element* p) -> bool
        {
            return p->name == _name;
        }
    );

    if (it != child_elements->end())
    {
        // Specified child element already exists. Make sure it's unlinked.
        element* elem = *it;
        if (elem->ref_type != reference_type::unknown || elem->elem_type != element_type::unlinked)
            throw xpath_error("This element is already linked.  You can't link the same element twice.");

        elem->link_reference(parent, _ref_type);
        return elem;
    }

    string_pool& sp = parent.m_names;

    // Insert a new linked element of this name.
    auto const nm = sp.intern(_name.name).first; // work around LLVM < 7 libc++ bug
    child_elements->push_back(
        parent.m_element_pool.construct(
            element::args_type(
                parent,
                xml_name_t(_name.ns, nm),
                element_type::linked,
                _ref_type
            )
        )
    );

    return child_elements->back();
}

void xml_map_tree::element::link_reference(xml_map_tree& parent, reference_type _ref_type)
{
    if (elem_type == element_type::unlinked)
        parent.m_element_store_pool.destroy(child_elements);

    elem_type = element_type::linked;
    ref_type = _ref_type;
    parent.create_ref_store(*this);
}

bool xml_map_tree::element::unlinked_attribute_anchor() const
{
    return elem_type == element_type::unlinked && ref_type == reference_type::unknown && !attributes.empty();
}

xml_map_tree::walker::walker(const xml_map_tree& parent) :
    m_parent(parent) {}
xml_map_tree::walker::walker(const xml_map_tree::walker& r) :
    m_parent(r.m_parent), m_stack(r.m_stack), m_unlinked_stack(r.m_unlinked_stack) {}

void xml_map_tree::walker::reset()
{
    m_stack.clear();
    m_unlinked_stack.clear();
}

xml_map_tree::element* xml_map_tree::walker::push_element(const xml_name_t& name)
{
    if (!m_unlinked_stack.empty())
    {
        // We're still in the unlinked region.
        m_unlinked_stack.push_back(name);
        return nullptr;
    }

    if (m_stack.empty())
    {
        if (!m_parent.mp_root)
        {
            // Tree is empty.
            m_unlinked_stack.push_back(name);
            return nullptr;
        }

        element* p = m_parent.mp_root;
        if (p->name != name)
        {
            // Names differ.
            m_unlinked_stack.push_back(name);
            return nullptr;
        }

        m_stack.push_back(p);
        return p;
    }

    if (m_stack.back()->elem_type == element_type::unlinked)
    {
        // Check if the current element has a child of the same name.
        element* p = m_stack.back()->get_child(name);
        if (p)
        {
            m_stack.push_back(p);
            return p;
        }
    }

    m_unlinked_stack.push_back(name);
    return nullptr;
}

xml_map_tree::element* xml_map_tree::walker::pop_element(const xml_name_t& name)
{
    if (!m_unlinked_stack.empty())
    {
        // We're in the unlinked region.  Pop element from the unlinked stack.
        if (m_unlinked_stack.back() != name)
            throw general_error("Closing element has a different name than the opening element. (unlinked stack)");

        m_unlinked_stack.pop_back();

        if (!m_unlinked_stack.empty())
            // We are still in the unlinked region.
            return nullptr;

        return m_stack.empty() ? nullptr : m_stack.back();
    }

    if (m_stack.empty())
        throw general_error("Element was popped while the stack was empty.");

    if (m_stack.back()->name != name)
        throw general_error("Closing element has a different name than the opening element. (linked stack)");

    m_stack.pop_back();
    return m_stack.empty() ? nullptr : m_stack.back();
}

xml_map_tree::xml_map_tree(xmlns_repository& xmlns_repo) :
    m_xmlns_cxt(xmlns_repo.create_context()),
    mp_root(nullptr),
    m_default_ns(XMLNS_UNKNOWN_ID) {}

xml_map_tree::~xml_map_tree() {}

void xml_map_tree::set_namespace_alias(std::string_view alias, std::string_view uri, bool default_ns)
{
#if ORCUS_DEBUG_XML_MAP_TREE
    cout << "xml_map_tree::set_namespace_alias: alias='" << alias << "', uri='" << uri << "', default=" << default_ns << endl;
#endif
    // We need to turn the alias string persistent because the xmlns context
    // doesn't intern the alias strings.
    std::string_view alias_safe = m_names.intern(alias).first;
    xmlns_id_t ns = m_xmlns_cxt.push(alias_safe, uri);

    if (default_ns)
        m_default_ns = ns;
}

xmlns_id_t xml_map_tree::get_namespace(std::string_view alias) const
{
    return m_xmlns_cxt.get(alias);
}

void xml_map_tree::set_cell_link(std::string_view xpath, const cell_position& ref)
{
    if (xpath.empty())
        return;

#if ORCUS_DEBUG_XML_MAP_TREE
    cout << "xml_map_tree::set_cell_link: xpath='" << xpath << "' (ref=" << ref << ")" << endl;
#endif

    linked_node_type linked_node = get_linked_node(xpath, reference_type::cell);
    assert(linked_node.node);
    assert(!linked_node.elem_stack.empty());
    cell_reference* cell_ref = nullptr;
    switch (linked_node.node->node_type)
    {
        case linkable_node_type::element:
            assert(static_cast<element*>(linked_node.node)->cell_ref);
            cell_ref = static_cast<element*>(linked_node.node)->cell_ref;
            break;
        case linkable_node_type::attribute:
            assert(static_cast<attribute*>(linked_node.node)->cell_ref);
            cell_ref = static_cast<attribute*>(linked_node.node)->cell_ref;
            break;
        default:
            throw general_error("unknown node type returned from get_element_stack call in xml_map_tree::set_cell_link().");
    }

    cell_ref->pos = ref;
}

void xml_map_tree::start_range(const cell_position& pos)
{
    m_cur_range_field_links.clear();
    m_cur_range_pos = pos;
}

void xml_map_tree::append_range_field_link(std::string_view xpath, std::string_view label)
{
    if (xpath.empty())
        return;

    m_cur_range_field_links.emplace_back(xpath, label);
}

void xml_map_tree::insert_range_field_link(
    range_reference& range_ref, element_list_type& range_parent, const range_field_link& field)
{
    linked_node_type linked_node = get_linked_node(field.xpath, reference_type::range_field);
    if (linked_node.elem_stack.size() < 2)
        throw xpath_error("Path of a range field link must be at least 2 levels.");

    if (linked_node.node->node_type == linkable_node_type::unknown)
        throw xpath_error("Unrecognized node type");

    if (linked_node.anchor_elem)
        linked_node.anchor_elem->linked_range_fields.push_back(range_ref.field_nodes.size());

    if (!field.label.empty())
        linked_node.node->label = intern_string(field.label);

    switch (linked_node.node->node_type)
    {
        case linkable_node_type::element:
        {
            element* p = static_cast<element*>(linked_node.node);
            assert(p && p->ref_type == reference_type::range_field && p->field_ref);
            p->field_ref->ref = &range_ref;
            p->field_ref->column_pos = range_ref.field_nodes.size();

            range_ref.field_nodes.push_back(p);
            break;
        }
        case linkable_node_type::attribute:
        {
            attribute* p = static_cast<attribute*>(linked_node.node);
            assert(p && p->ref_type == reference_type::range_field && p->field_ref);
            p->field_ref->ref = &range_ref;
            p->field_ref->column_pos = range_ref.field_nodes.size();

            range_ref.field_nodes.push_back(p);
            break;
        }
        default:
            ;
    }

    // Determine the deepest common element for all field link elements in the
    // current range reference.
    if (range_parent.empty())
    {
        // First field link in this range. Find the first row-group element
        // going up from the linked node.
        auto rit = linked_node.elem_stack.rbegin();
        while (rit != linked_node.elem_stack.rend() && !(*rit)->row_group)
            ++rit;

        ++rit; // parent of the raw group
        auto it_end = rit.base();

        range_parent.assign(linked_node.elem_stack.begin(), it_end);
    }
    else
    {
        // Determine the deepest common element between the two.
        element_list_type::iterator it_elem = linked_node.elem_stack.begin(), it_elem_end = linked_node.elem_stack.end();
        element_list_type::iterator it_cur = range_parent.begin(), it_cur_end = range_parent.end();
        if (*it_elem != *it_cur)
            throw xpath_error("Two field links in the same range reference start with different root elements.");

        ++it_elem;
        ++it_cur;

        for (; it_elem != it_elem_end && it_cur != it_cur_end; ++it_elem, ++it_cur)
        {
            if (*it_elem == *it_cur)
                continue;

            // The two elements differ.  Take their parent element as the new common element.
            range_parent.assign(linked_node.elem_stack.begin(), it_elem); // current elemnt excluded.
            break;
        }

        if (range_parent.empty())
            throw xpath_error("Two field links in the same range reference must at least share the first level of their paths.");
    }

#if ORCUS_DEBUG_XML_MAP_TREE
        print_element_stack(std::cout, range_parent);
        std::cout << std::endl;
#endif
}

void xml_map_tree::set_range_row_group(std::string_view xpath)
{
    if (xpath.empty())
        return;

    range_reference* range_ref = get_range_reference(m_cur_range_pos);
    assert(range_ref);

    element* elem = get_element(xpath);
    assert(elem);
    elem->row_group = range_ref;
}

void xml_map_tree::commit_range()
{
    if (m_cur_range_field_links.empty())
        // Nothing to commit.
        return;

    range_reference* range_ref = get_range_reference(m_cur_range_pos);
    assert(range_ref);

    // commont parent element for this range.
    element_list_type range_parent;

    for (const range_field_link& field : m_cur_range_field_links)
        insert_range_field_link(*range_ref, range_parent, field);

#if ORCUS_DEBUG_XML_MAP_TREE
    cout << "parent element path for this range: ";
    for (const element* elem : range_parent)
        cout << "/" << elem->name.to_string(m_xmlns_cxt, xml_name_t::use_alias);
    cout << endl;
#endif

    assert(!range_parent.empty());
    // Mark the range parent element.
    range_parent.back()->range_parent = range_ref;

    // Set the current position invalid.
    m_cur_range_pos.row = -1;
    m_cur_range_pos.col = -1;
}

const xml_map_tree::linkable* xml_map_tree::get_link(std::string_view xpath) const
{
    if (!mp_root)
        return nullptr;

    if (xpath.empty())
        return nullptr;

#if ORCUS_DEBUG_XML_MAP_TREE
    cout << "xml_map_tree::get_link: xpath = '" << xpath << "'" << endl;
#endif
    const linkable* cur_node = mp_root;

    xpath_parser parser(m_xmlns_cxt, xpath.data(), xpath.size(), m_default_ns);

    // Check the root element first.
    xpath_parser::token token = parser.next();
    if (cur_node->name.ns != token.ns || cur_node->name.name != token.name)
        // Root element name doesn't match.
        return nullptr;

#if ORCUS_DEBUG_XML_MAP_TREE
    cout << "xml_map_tree::get_link: root = (ns=" << token.ns << ", name=" << token.name << ")" << endl;
#endif
    for (token = parser.next(); !token.name.empty(); token = parser.next())
    {
        if (token.attribute)
        {
            // The current node should be an element and should have an attribute of the same name.
            if (cur_node->node_type != linkable_node_type::element)
                return nullptr;

            const element* elem = static_cast<const element*>(cur_node);
            const attribute_store_type& attrs = elem->attributes;
            auto it = std::find_if(
                attrs.begin(), attrs.end(),
                [&token](const attribute* p) -> bool
                {
                    return p->name.ns == token.ns && p->name.name == token.name;
                }
            );

            if (it == attrs.end())
                // No such attribute exists.
                return nullptr;

            return *it;
        }

        // See if an element of this name exists below the current element.

        if (cur_node->node_type != linkable_node_type::element)
            return nullptr;

        const element* elem = static_cast<const element*>(cur_node);
        if (elem->elem_type != element_type::unlinked)
            return nullptr;

        if (!elem->child_elements)
            return nullptr;

        auto it = std::find_if(
            elem->child_elements->begin(), elem->child_elements->end(),
            [&token](const element* p) -> bool
            {
                return p->name.ns == token.ns && p->name.name == token.name;
            }
        );

        if (it == elem->child_elements->end())
            // No such child element exists.
            return nullptr;

        cur_node = *it;
    }

    if (cur_node->node_type != linkable_node_type::element || static_cast<const element*>(cur_node)->elem_type == element_type::unlinked)
        // Non-leaf elements are not links.
        return nullptr;

    return cur_node;
}

xml_map_tree::walker xml_map_tree::get_tree_walker() const
{
    return walker(*this);
}

xml_map_tree::range_ref_map_type& xml_map_tree::get_range_references()
{
    return m_field_refs;
}

std::string_view xml_map_tree::intern_string(std::string_view str) const
{
    return m_names.intern(str).first;
}

xml_map_tree::range_reference* xml_map_tree::get_range_reference(const cell_position& pos)
{
    range_ref_map_type::iterator it = m_field_refs.lower_bound(pos);
    if (it == m_field_refs.end() || m_field_refs.key_comp()(pos, it->first))
    {
        // This reference does not exist yet.  Insert a new one.

        // Make sure the sheet name string is persistent.
        cell_position pos_safe = pos;
        pos_safe.sheet = m_names.intern(pos.sheet).first;

        it = m_field_refs.insert(
            it, range_ref_map_type::value_type(
                pos_safe,
                m_range_reference_pool.construct(pos_safe)));
    }

    return it->second;
}

void xml_map_tree::create_ref_store(linkable& node)
{
    switch (node.ref_type)
    {
        case xml_map_tree::reference_type::cell:
            node.cell_ref = m_cell_reference_pool.construct();
            break;
        case xml_map_tree::reference_type::range_field:
            node.field_ref = m_field_in_range_pool.construct();
            break;
        case xml_map_tree::reference_type::unknown:
            break;
    }
}

xml_map_tree::linked_node_type xml_map_tree::get_linked_node(std::string_view xpath, reference_type ref_type)
{
    linked_node_type ret;

    assert(!xpath.empty());
    xpath_parser parser(m_xmlns_cxt, xpath.data(), xpath.size(), m_default_ns);

    // Get the root element first.
    xpath_parser::token token = parser.next();
    if (mp_root)
    {
        // Make sure the root element's names are the same.
        if (mp_root->name.ns != token.ns || mp_root->name.name != token.name)
            throw xpath_error("path begins with inconsistent root level name.");
    }
    else
    {
        // First time the root element is encountered.
        if (token.attribute)
            throw xpath_error("root element cannot be an attribute.");

        auto const nm = m_names.intern(token.name).first; // work around LLVM < 7 libc++ bug
        mp_root = m_element_pool.construct(
            element::args_type(
                *this,
                xml_name_t(token.ns, nm),
                element_type::unlinked,
                reference_type::unknown
            )
        );
    }

    ret.elem_stack.push_back(mp_root);
    element* cur_element = ret.elem_stack.back();
    assert(cur_element);
    assert(cur_element->child_elements);

    element* row_group_elem = nullptr;

    token = parser.next();
    for (xpath_parser::token token_next = parser.next(); !token_next.name.empty(); token_next = parser.next())
    {
        // Check if the current element contains a child element of the same name.
        if (token.attribute)
            throw xpath_error("attribute must always be at the end of the path.");

        cur_element = cur_element->get_or_create_child(*this, {token.ns, token.name});
        ret.elem_stack.push_back(cur_element);
        token = token_next;

        if (cur_element->row_group)
            row_group_elem = cur_element;
    }

    assert(cur_element);

    // Insert a leaf node.

    if (token.attribute)
    {
        // This is an attribute.  Insert it into the current element.
        attribute_store_type& attrs = cur_element->attributes;

        // Check if an attribute of the same name already exists.
        auto it = std::find_if(
            attrs.begin(), attrs.end(),
            [&token](const attribute* p) -> bool
            {
                return p->name.ns == token.ns && p->name.name == token.name;
            }
        );

        if (it != attrs.end())
            throw xpath_error("This attribute is already linked.  You can't link the same attribute twice.");

        auto const nm = m_names.intern(token.name).first; // work around LLVM < 7 libc++ bug
        attribute* p = m_attribute_pool.construct(
            attribute::args_type(
                *this,
                xml_name_t(token.ns, nm),
                ref_type
            )
        );

        attrs.push_back(p);
        ret.node = attrs.back();
    }
    else
    {
        element* elem = cur_element->get_or_create_linked_child(*this, {token.ns, token.name}, ref_type);
        ret.elem_stack.push_back(elem);
        ret.node = elem;

        if (elem->row_group)
            row_group_elem = elem;
    }

    ret.anchor_elem = row_group_elem;
    return ret;
}

xml_map_tree::element* xml_map_tree::get_element(std::string_view xpath)
{
    assert(!xpath.empty());
    xpath_parser parser(m_xmlns_cxt, xpath.data(), xpath.size(), m_default_ns);

    // Get the root element first.
    xpath_parser::token token = parser.next();
    if (mp_root)
    {
        // Make sure the root element's names are the same.
        if (mp_root->name.ns != token.ns || mp_root->name.name != token.name)
            throw xpath_error("path begins with inconsistent root level name.");
    }
    else
    {
        // First time the root element is encountered.
        if (token.attribute)
            throw xpath_error("root element cannot be an attribute.");

        auto const nm = m_names.intern(token.name).first; // work around LLVM < 7 libc++ bug
        mp_root = m_element_pool.construct(
            element::args_type(
                *this,
                xml_name_t(token.ns, nm),
                element_type::unlinked,
                reference_type::unknown
            )
        );
    }

    element* cur_element = mp_root;
    assert(cur_element->child_elements);

    for (token = parser.next(); !token.name.empty(); token = parser.next())
    {
        // Check if the current element contains a child element of the same name.
        if (token.attribute)
            throw xpath_error("attribute was not expected.");

        cur_element = cur_element->get_or_create_child(*this, {token.ns, token.name});
    }

    assert(cur_element);
    return cur_element;
}

std::ostream& operator<< (std::ostream& os, const xml_map_tree::linkable& link)
{
    if (!link.ns_alias.empty())
        os << link.ns_alias << ':';
    os << link.name.name;
    return os;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
