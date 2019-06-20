/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/json_structure_tree.hpp"
#include "orcus/json_parser.hpp"
#include "orcus/global.hpp"

#include <iostream>
#include <vector>
#include <deque>
#include <memory>
#include <algorithm>

namespace orcus { namespace json {

namespace {

struct structure_node
{
    enum node_type { unknown, array, object, object_key, value };

    node_type type = unknown;

    std::deque<structure_node> children;

    pstring name;

    structure_node(node_type _type) : type(_type) {}

    bool operator== (const structure_node& other) const
    {
        if (type != other.type)
            return false;

        if (type != object_key)
            return true;

        return name == other.name;
    }
};

} // anonymous namespace

struct structure_tree::impl
{
    using stack_type = std::vector<structure_node*>;

    std::unique_ptr<structure_node> m_root;
    stack_type m_stack;

    impl() {}
    ~impl() {}

    void begin_parse() {}

    void end_parse() {}

    void begin_array()
    {
        std::cout << __FILE__ << ":" << __LINE__ << " (json_structure_tree:impl:begin_array): " << std::endl;
        push_stack(structure_node::array);
    }

    void end_array()
    {
        std::cout << __FILE__ << ":" << __LINE__ << " (json_structure_tree:impl:end_array): " << std::endl;
        pop_stack();
    }

    void begin_object()
    {
        std::cout << __FILE__ << ":" << __LINE__ << " (json_structure_tree:impl:begin_object): " << std::endl;
        push_stack(structure_node::object);
    }

    void object_key(const char* p, size_t len, bool transient)
    {
        structure_node node(structure_node::object_key);
        node.name = pstring(p, len);
        push_stack(node);
    }

    void end_object()
    {
        std::cout << __FILE__ << ":" << __LINE__ << " (json_structure_tree:impl:end_object): " << std::endl;
        pop_stack();
    }

    void boolean_true()
    {
        push_value();
    }

    void boolean_false()
    {
        push_value();
    }

    void null()
    {
        push_value();
    }

    void string(const char* p, size_t len, bool transient)
    {
        push_value();
    }

    void number(double val)
    {
        push_value();
    }

private:

    structure_node& get_current_node()
    {
        assert(!m_stack.empty());
        return *m_stack.back();
    }

    void push_stack(const structure_node& node)
    {
        if (!m_root)
        {
            // This is the very first node.
            assert(node.type != structure_node::object_key);
            m_root = orcus::make_unique<structure_node>(node.type);
            m_stack.push_back(m_root.get());
            return;
        }

        // See if the current node has a child node of the specified type.
        structure_node& cur_node = get_current_node();
        auto it = std::find(cur_node.children.begin(), cur_node.children.end(), node);

        if (it == cur_node.children.end())
        {
            // current node doesn't have an array child.  Add one.
            cur_node.children.emplace_back(node);
            m_stack.push_back(&cur_node.children.back());
        }
        else
        {
            // current node does have an array child.
            structure_node& child = *it;
            m_stack.push_back(&child);
            std::cout << __FILE__ << ":" << __LINE__ << " (structure_tree:impl:push_stack): repeat" << std::endl;
        }
    }

    void push_value()
    {
        std::cout << __FILE__ << ":" << __LINE__ << " (structure_tree:impl:push_value): " << std::endl;
        push_stack(structure_node::value);
        pop_stack();
    }

    void pop_stack()
    {
        assert(!m_stack.empty());
        m_stack.pop_back();
    }
};

structure_tree::structure_tree() : mp_impl(std::make_unique<impl>()) {}
structure_tree::~structure_tree() {}

void structure_tree::parse(const char* p, size_t n)
{
    json_parser<impl> parser(p, n, *mp_impl);
    parser.parse();

    std::cout << __FILE__ << ":" << __LINE__ << " (structure_tree:parse): stack size = " << mp_impl->m_stack.size() << std::endl;
}

void structure_tree::dump_compact(std::ostream& os) const
{
    std::cout << __FILE__ << ":" << __LINE__ << " (structure_tree:dump_compact): " << std::endl;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
