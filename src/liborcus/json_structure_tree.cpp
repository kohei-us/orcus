/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/json_structure_tree.hpp"
#include "orcus/json_parser.hpp"
#include "orcus/global.hpp"
#include "orcus/string_pool.hpp"

#include <vector>
#include <deque>
#include <memory>
#include <algorithm>

namespace orcus { namespace json {

namespace {

struct structure_node;

using node_children_type = std::deque<structure_node>;

/**
 * Represents a node inside a JSON structure tree.
 */
struct structure_node
{
    enum node_type { unknown, array, object, object_key, value };

    bool repeat = false;

    node_type type = unknown;

    node_children_type children;

    uint32_t child_count = 0;

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

struct parse_scope
{
    structure_node& node;

    uint32_t child_count = 0;

    parse_scope(structure_node& _node) : node(_node) {}
};

using parse_scopes_type = std::vector<parse_scope>;

/**
 * Represents a scope during structure tree traversal.
 */
struct scope
{
    const structure_node& node;
    node_children_type::const_iterator current_pos;

    scope(const structure_node& _node) :
        node(_node),
        current_pos(node.children.begin()) {}
};

using scope_stack_type = std::vector<scope>;

void print_scopes(std::ostream& os, const scope_stack_type& scopes)
{
    os << '/';

    for (const scope& s : scopes)
    {
        switch (s.node.type)
        {
            case structure_node::array:
                os << "array";
                break;
            case structure_node::object:
                os << "object";
                break;
            case structure_node::object_key:
                os << "['" << s.node.name << "']";
                break;
            default:
                os << "???";
        }

        if (s.node.repeat)
        {
            if (s.node.type == structure_node::array && s.node.child_count)
                os << "(*|w=" << s.node.child_count;
            else
                os << "(*";

            os << ')';
        }

        os << '/';
    }
}

} // anonymous namespace

struct structure_tree::impl
{
    std::unique_ptr<structure_node> m_root;
    parse_scopes_type m_stack;
    string_pool m_pool;

    impl() {}
    ~impl() {}

    void begin_parse() {}

    void end_parse() {}

    void begin_array()
    {
        push_stack(structure_node::array);
    }

    void end_array()
    {
        pop_stack();
    }

    void begin_object()
    {
        push_stack(structure_node::object);
    }

    void object_key(const char* p, size_t len, bool transient)
    {
        structure_node node(structure_node::object_key);
        node.name = pstring(p, len);

        if (transient)
            node.name = m_pool.intern(node.name).first;

        push_stack(node);
    }

    void end_object()
    {
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

    void dump_compact(std::ostream& os) const
    {
        if (!m_root)
            return;

        scope_stack_type scopes;
        scopes.emplace_back(*m_root);

        while (!scopes.empty())
        {
            scope& cur_scope = scopes.back();

            bool new_scope = false;

            for (; cur_scope.current_pos != cur_scope.node.children.end(); ++cur_scope.current_pos)
            {
                const structure_node& cur_node = *cur_scope.current_pos;

                if (cur_node.type == structure_node::value)
                {
                    assert(cur_node.children.empty());

                    // Print this leaf node and all its parent scopes.
                    print_scopes(os, scopes);
                    os << "value" << std::endl;
                    continue;
                }

                assert(!cur_node.children.empty());

                // This node has child nodes. Push a new scope and trigger a new inner loop.

                ++cur_scope.current_pos; // Don't forget to move to the next sibling for when we return to this scope.
                scopes.emplace_back(cur_node);
                new_scope = true;
                break;
            }

            if (new_scope)
                continue;

            scopes.pop_back();
        }
    }

private:

    parse_scope& get_current_scope()
    {
        assert(!m_stack.empty());
        return m_stack.back();
    }

    bool is_node_repeatable(const structure_node& node) const
    {
        const structure_node& cur = m_stack.back().node;

        if (cur.type != structure_node::array)
            return false;

        return node.type == structure_node::array || node.type == structure_node::object;
    }

    void push_stack(const structure_node& node)
    {
        if (!m_root)
        {
            // This is the very first node.
            assert(node.type != structure_node::object_key);
            m_root = orcus::make_unique<structure_node>(node.type);
            m_stack.emplace_back(*m_root);
            return;
        }

        // See if the current node has a child node of the specified type.
        parse_scope& cur_scope = get_current_scope();
        ++cur_scope.child_count;

        structure_node& cur_node = cur_scope.node;

        auto it = std::find(cur_node.children.begin(), cur_node.children.end(), node);

        if (it == cur_node.children.end())
        {
            // current node doesn't have a child of specified type.  Add one.
            cur_node.children.emplace_back(node);
            m_stack.emplace_back(cur_node.children.back());
        }
        else
        {
            // current node does have a child of specified type.
            bool repeat = is_node_repeatable(node);
            structure_node& child = *it;
            child.repeat = repeat;
            m_stack.emplace_back(child);
        }
    }

    void push_value()
    {
        push_stack(structure_node::value);
        pop_stack();
    }

    void pop_stack()
    {
        parse_scope& cur_scope = get_current_scope();
        structure_node& cur_node = cur_scope.node;
        if (cur_scope.child_count > cur_node.child_count)
            cur_node.child_count = cur_scope.child_count;

        m_stack.pop_back();

        if (!m_stack.empty() && get_current_scope().node.type == structure_node::object_key)
            // Object key is a special non-leaf node that can only have one child.
            m_stack.pop_back();
    }
};

structure_tree::structure_tree() : mp_impl(std::make_unique<impl>()) {}
structure_tree::~structure_tree() {}

void structure_tree::parse(const char* p, size_t n)
{
    json_parser<impl> parser(p, n, *mp_impl);
    parser.parse();
}

void structure_tree::dump_compact(std::ostream& os) const
{
    mp_impl->dump_compact(os);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
