/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "json_map_tree.hpp"
#include "orcus/global.hpp"
#include "orcus/measurement.hpp"

#include <iostream>
#include <sstream>

namespace orcus {

namespace {

void throw_path_error(const pstring& path)
{
    std::ostringstream os;
    os << "failed to link this path '" << path << "'";
    throw json_map_tree::path_error(os.str());
}

enum class json_path_token_t { unknown, array, array_pos, object, end };

class json_path_parser
{
    const char* mp_cur;
    const char* mp_end;

public:

    struct token
    {
        json_path_token_t type = json_path_token_t::unknown;
        long array_pos = json_map_tree::node_child_default_position;

        token(json_path_token_t _type) : type(_type) {}
        token(long _array_pos) : type(json_path_token_t::array_pos), array_pos(_array_pos) {}
    };

    json_path_parser(const pstring& path) :
        mp_cur(path.data()),
        mp_end(mp_cur + path.size())
    {
        assert(!path.empty());
        assert(path[0] == '$');
        ++mp_cur; // skip the first '$'.
    }

    token next()
    {
        if (mp_cur == mp_end)
            return json_path_token_t::end;

        switch (*mp_cur)
        {
            case '[':
                return next_array_pos();
            default:
                ;
        }

        throw json_map_tree::path_error("invalid path");
    }

    token next_array_pos()
    {
        assert(*mp_cur == '[');
        ++mp_cur;
        const char* p_head = mp_cur;

        for (; mp_cur != mp_end; ++mp_cur)
        {
            if (*mp_cur != ']')
                continue;

            if (p_head == mp_cur)
            {
                // empty brackets.
                ++mp_cur;
                return json_map_tree::node_child_default_position;
            }

            const char* p_parse_ended = nullptr;
            long pos = to_long(p_head, mp_cur, &p_parse_ended);

            if (p_parse_ended != mp_cur)
                // Parsing failed.
                break;

            if (pos < 0)
                // array position cannot be negative.
                break;

            ++mp_cur; // skip the ']'.
            return pos;
        }

        return json_path_token_t::unknown;
    }
};

} // anonymous namespace

json_map_tree::path_error::path_error(const std::string& msg) :
    general_error(msg) {}

json_map_tree::cell_reference_type::cell_reference_type(const cell_position_t& _pos) :
    pos(_pos) {}

json_map_tree::range_reference_type::range_reference_type(const cell_position_t& _pos) :
    pos(_pos) {}

json_map_tree::node::node() {}
json_map_tree::node::node(node&& other) :
    type(other.type)
{
    value.children = nullptr;

    switch (type)
    {
        case node_type::array:
            value.children = other.value.children;
            other.value.children = nullptr;
            break;
        case node_type::cell_ref:
            value.cell_ref = other.value.cell_ref;
            other.value.cell_ref = nullptr;
            break;
        case node_type::range_field_ref:
            value.range_field_ref = other.value.range_field_ref;
            other.value.range_field_ref = nullptr;
        default:
            ;
    }

    other.type = node_type::unknown;
}

json_map_tree::node& json_map_tree::node::get_or_create_child_node(long pos)
{
    node_children_type& children = *value.children;

    auto it = children.lower_bound(pos); // get the first position where pos <= k is true.

    if (it == children.end() || children.key_comp()(pos, it->first))
    {
        // Insert a new array child node of unspecified type at the specified position.
        it = children.insert(
            it, node_children_type::value_type(pos, node()));
    }

    assert(it->first == pos);
    return it->second;
}

json_map_tree::walker::scope::scope(const node* _p) : p(_p), array_position(0) {}

json_map_tree::walker::walker(const json_map_tree& parent) : m_parent(parent) {}

const json_map_tree::node* json_map_tree::walker::push_node(node_type nt)
{
    if (!m_unlinked_stack.empty())
    {
        // We're still in the unlinked region.
        m_unlinked_stack.push_back(nt);
        return nullptr;
    }

    if (m_stack.empty())
    {
        if (!m_parent.m_root)
        {
            // Tree is empty.
            m_unlinked_stack.push_back(nt);
            return nullptr;
        }

        const node* p = m_parent.m_root.get();

        if (p->type != nt)
        {
            // Different node type.
            m_unlinked_stack.push_back(nt);
            return nullptr;
        }

        m_stack.push_back(p);
        return p;
    }

    scope& cur_scope = m_stack.back();

    switch (cur_scope.p->type)
    {
        case json_map_tree::node_type::array:
        {
            const node_children_type& node_children = *cur_scope.p->value.children;

            auto it = node_children.find(cur_scope.array_position);
            if (it == node_children.end())
                it = node_children.find(json_map_tree::node_child_default_position);

            if (it == node_children.end())
                throw std::logic_error("empty array should never happen!");

            const node* p = &it->second;
            m_stack.push_back(p);
            return p;
        }
        case json_map_tree::node_type::object:
            throw std::runtime_error("WIP: handle this");
        default:
            ;
    }

    m_unlinked_stack.push_back(nt);
    return nullptr;
}

const json_map_tree::node* json_map_tree::walker::pop_node(node_type nt)
{
    throw std::runtime_error("WIP: node popping has yet to be implemented.");
    return nullptr;
}

json_map_tree::json_map_tree() {}
json_map_tree::~json_map_tree() {}

json_map_tree::walker json_map_tree::get_tree_walker() const
{
    return walker(*this);
}

void json_map_tree::set_cell_link(const pstring& path, const cell_position_t& pos)
{
    node* p = get_or_create_destination_node(path);
    if (!p)
        return;

    if (p->type != node_type::unknown)
    {
        std::ostringstream os;
        os << "this path is not linkable: '" << path << '\'';
        throw path_error(os.str());
    }

    p->type = node_type::cell_ref;
    p->value.cell_ref = m_cell_ref_pool.construct(pos);

    // Ensure that this tree owns the instance of the string.
    p->value.cell_ref->pos.sheet = m_str_pool.intern(p->value.cell_ref->pos.sheet).first;

    std::cout << __FILE__ << ":" << __LINE__ << " (json_map_tree:set_cell_link): cell link set" << std::endl;
}

const json_map_tree::node* json_map_tree::get_link(const pstring& path) const
{
    return get_destination_node(path);
}

void json_map_tree::start_range(const cell_position_t& pos)
{
    m_current_range.pos = pos;
    m_current_range.field_paths.clear();
    m_current_range.row_groups.clear();
}

void json_map_tree::append_field_link(const pstring& path)
{
    m_current_range.field_paths.push_back(path);
}

void json_map_tree::set_range_row_group(const pstring& path)
{
    m_current_range.row_groups.push_back(path);
}

void json_map_tree::commit_range()
{
    auto it = m_range_refs.lower_bound(m_current_range.pos);
    if (it == m_range_refs.end() || m_range_refs.key_comp()(m_current_range.pos, it->first))
    {
        // Ensure that we own the sheet name instance before storing it.
        m_current_range.pos.sheet = m_str_pool.intern(m_current_range.pos.sheet).first;

        it = m_range_refs.insert(
            it, range_ref_store_type::value_type(
                m_current_range.pos, range_reference_type(m_current_range.pos)));
    }

    range_reference_type* ref = &it->second;
    spreadsheet::col_t column_pos = 0;

    for (const pstring& path : m_current_range.field_paths)
    {
        node* p = get_or_create_destination_node(path);
        if (!p || p->type != node_type::unknown)
            throw_path_error(path);

        p->type = node_type::range_field_ref;
        p->value.range_field_ref = m_range_field_ref_pool.construct();
        p->value.range_field_ref->column_pos = column_pos++;
        p->value.range_field_ref->ref = ref;

        ref->fields.push_back(p);
    }

    for (const pstring& path : m_current_range.row_groups)
    {
        node* p = get_or_create_destination_node(path);
        if (!p)
            throw_path_error(path);

        p->row_group = ref;
    }

    std::cout << __FILE__ << "#" << __LINE__ << " (json_map_tree:commit_range): all good!" << std::endl;
}

const json_map_tree::node* json_map_tree::get_destination_node(const pstring& path) const
{
    if (!m_root)
        // The tree is empty.
        return nullptr;

    if (path.empty() || path[0] != '$')
        // A valid path must begin with a '$'.
        return nullptr;

    json_path_parser parser(path);
    const node* cur_node = m_root.get();

    for (json_path_parser::token t = parser.next(); t.type != json_path_token_t::unknown; t = parser.next())
    {
        switch (t.type)
        {
            case json_path_token_t::array_pos:
            {
                if (cur_node->type != node_type::array)
                    return nullptr;

                std::cerr << __FILE__ << "#" << __LINE__ << " (json_map_tree:get_destination_node): array pos = " << t.array_pos << std::endl;
                auto it = cur_node->value.children->find(t.array_pos);
                if (it == cur_node->value.children->end())
                {
                    std::cerr << __FILE__ << "#" << __LINE__ << " (json_map_tree:get_destination_node): no node at the array pos." << std::endl;
                    return nullptr;
                }

                cur_node = &it->second;
                break;
            }
            case json_path_token_t::end:
                std::cerr << __FILE__ << "#" << __LINE__ << " (json_map_tree:get_destination_node): end" << std::endl;
                return cur_node;
            case json_path_token_t::unknown:
            default:
                // Something has gone wrong. Bail out.
                break;
        }
    }

    // If this code path reaches here, something has gone wrong.
    return nullptr;
}

json_map_tree::node* json_map_tree::get_or_create_destination_node(const pstring& path)
{
    std::cout << __FILE__ << ":" << __LINE__ << " (json_map_tree:get_linked_node): path='" << path << "'" << std::endl;

    if (path.empty() || path[0] != '$')
        // A valid path must begin with a '$'.
        return nullptr;

    json_path_parser parser(path);

    node* cur_node = nullptr;

    json_path_parser::token t = parser.next();

    switch (t.type)
    {
        case json_path_token_t::array_pos:
        {
            // Insert or re-use an array node and its child at specified position.

            std::cout << __FILE__ << ":" << __LINE__ << " (json_map_tree:get_linked_node): array pos = " << t.array_pos << std::endl;

            if (m_root)
            {
                if (m_root->type != node_type::array)
                    throw path_error("root node was expected to be of type array, but is not.");

                assert(m_root->value.children);
            }
            else
            {
                m_root = orcus::make_unique<node>();
                m_root->type = node_type::array;
                m_root->value.children = m_node_children_pool.construct();
            }

            cur_node = m_root.get();
            cur_node = &cur_node->get_or_create_child_node(t.array_pos);
            break;
        }
        default:
            // Something has gone wrong. Bail out.
            return nullptr;
    }

    for (t = parser.next(); t.type != json_path_token_t::unknown; t = parser.next())
    {
        switch (t.type)
        {
            case json_path_token_t::array_pos:
            {
                std::cout << __FILE__ << ":" << __LINE__ << " (json_map_tree:get_linked_node): array pos = " << t.array_pos << std::endl;
                switch (cur_node->type)
                {
                    case node_type::array:
                        // Do nothing.
                        break;
                    case node_type::unknown:
                        // Turn this node into an array node.
                        cur_node->type = node_type::array;
                        cur_node->value.children = m_node_children_pool.construct();
                        break;
                    default:
                        throw_path_error(path);
                }
                cur_node = &cur_node->get_or_create_child_node(t.array_pos);
                break;
            }
            case json_path_token_t::end:
                std::cout << __FILE__ << ":" << __LINE__ << " (json_map_tree:get_linked_node): end" << std::endl;
                return cur_node;
            case json_path_token_t::unknown:
            default:
                // Something has gone wrong. Bail out.
                break;
        }
    }

    // If this code path reaches here, something has gone wrong.
    return nullptr;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
