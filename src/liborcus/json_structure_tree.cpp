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
using node_type = structure_tree::node_type;

/**
 * Represents a node inside a JSON structure tree.
 */
struct structure_node
{
    bool repeat = false;

    node_type type = node_type::unknown;

    node_children_type children;

    uint32_t child_count = 0;

    pstring name;

    structure_node(node_type _type) : type(_type) {}

    bool operator== (const structure_node& other) const
    {
        if (type != other.type)
            return false;

        if (type != node_type::object_key)
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

void print_scope(std::ostream& os, const scope& s)
{
    switch (s.node.type)
    {
        case node_type::array:
            os << "array";
            break;
        case node_type::object:
            os << "object";
            break;
        case node_type::object_key:
            os << "['" << s.node.name << "']";
            break;
        default:
            os << "???";
    }

    if (s.node.repeat)
        os << "(*)";

    if (s.node.type == node_type::array && s.node.child_count)
        os << '[' << s.node.child_count << ']';
}

void print_scopes(std::ostream& os, const scope_stack_type& scopes)
{
    auto it = scopes.cbegin();
    auto ite = scopes.cend();

    os << '$';
    print_scope(os, *it);

    for (++it; it != ite; ++it)
    {
        if (it->node.type != node_type::object_key)
            os << '.';
        print_scope(os, *it);
    }

    os << std::endl;
}

structure_tree::node_properties to_node_properties(const structure_node& sn)
{
    structure_tree::node_properties np;
    np.type = sn.type;
    np.repeat = sn.repeat;
    return np;
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
        push_stack(node_type::array);
    }

    void end_array()
    {
        pop_stack();
    }

    void begin_object()
    {
        push_stack(node_type::object);
    }

    void object_key(const char* p, size_t len, bool transient)
    {
        structure_node node(node_type::object_key);
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

                if (cur_node.type == node_type::value)
                {
                    assert(cur_node.children.empty());

                    // Print all its parent scopes.
                    print_scopes(os, scopes);
                    continue;
                }

                if (cur_node.children.empty())
                    continue;

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

        if (cur.type != node_type::array)
            return false;

        return node.type == node_type::array || node.type == node_type::object;
    }

    void push_stack(const structure_node& node)
    {
        if (!m_root)
        {
            // This is the very first node.
            assert(node.type != node_type::object_key);
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
        push_stack(node_type::value);
        pop_stack();
    }

    void pop_stack()
    {
        parse_scope& cur_scope = get_current_scope();
        structure_node& cur_node = cur_scope.node;
        if (cur_scope.child_count > cur_node.child_count)
            cur_node.child_count = cur_scope.child_count;

        m_stack.pop_back();

        if (!m_stack.empty() && get_current_scope().node.type == node_type::object_key)
            // Object key is a special non-leaf node that can only have one child.
            m_stack.pop_back();
    }
};

struct structure_tree::walker::impl
{
    using stack_type = std::vector<const structure_node*>;

    const structure_tree::impl* parent_impl;

    stack_type stack;

    impl() : parent_impl(nullptr) {}

    impl(const structure_tree::impl* _parent_impl) : parent_impl(_parent_impl) {}

    impl(const structure_tree::walker::impl& other) :
        parent_impl(other.parent_impl) {}

    void check_tree()
    {
        if (!parent_impl)
            throw json_structure_error(
                "This walker is not associated with any json_structure_tree instance.");

        if (!parent_impl->m_root)
            throw json_structure_error("Empty tree.");
    }
};

structure_tree::walker::walker() : mp_impl(orcus::make_unique<impl>()) {}
structure_tree::walker::walker(const walker& other) : mp_impl(orcus::make_unique<impl>(*other.mp_impl)) {}
structure_tree::walker::walker(const structure_tree::impl* parent_impl) : mp_impl(orcus::make_unique<impl>(parent_impl)) {}
structure_tree::walker::~walker() {}

void structure_tree::walker::root()
{
    mp_impl->check_tree();

    mp_impl->stack.clear();
    mp_impl->stack.push_back(mp_impl->parent_impl->m_root.get());
}

void structure_tree::walker::descend(size_t child_pos)
{
    mp_impl->check_tree();
    assert(!mp_impl->stack.empty());

    const structure_node* p = mp_impl->stack.back();
    assert(p);

    if (child_pos >= p->children.size())
    {
        std::ostringstream os;
        os << "Specified child position of " << child_pos << " exceeds the child count of " << p->children.size() << '.';
        throw json_structure_error(os.str());
    }

    p = &p->children[child_pos];
    assert(p);
    mp_impl->stack.push_back(p);
}

void structure_tree::walker::ascend()
{
    mp_impl->check_tree();
    assert(!mp_impl->stack.empty());

    if (mp_impl->stack.size() == 1u)
        throw json_structure_error("You cannot ascend from the root node.");

    mp_impl->stack.pop_back();
}

size_t structure_tree::walker::child_count() const
{
    mp_impl->check_tree();
    assert(!mp_impl->stack.empty());

    const structure_node* p = mp_impl->stack.back();
    return p->children.size();
}

structure_tree::node_properties structure_tree::walker::get_node() const
{
    mp_impl->check_tree();
    assert(!mp_impl->stack.empty());

    const structure_node* p = mp_impl->stack.back();
    assert(p);
    return to_node_properties(*p);
}

structure_tree::structure_tree() : mp_impl(orcus::make_unique<impl>()) {}
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

structure_tree::walker structure_tree::get_walker() const
{
    return walker(mp_impl.get());
}

std::ostream& operator<< (std::ostream& os, structure_tree::node_type nt)
{

    switch (nt)
    {
        case structure_tree::node_type::array:
            os << "structure_tree::node_type::array";
            break;
        case structure_tree::node_type::object:
            os << "structure_tree::node_type::object";
            break;
        case structure_tree::node_type::object_key:
            os << "structure_tree::node_type::object_key";
            break;
        case structure_tree::node_type::unknown:
            os << "structure_tree::node_type::unknown";
            break;
        case structure_tree::node_type::value:
            os << "structure_tree::node_type::value";
            break;
    }

    return os;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
