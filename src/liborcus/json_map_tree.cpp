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

        return next_name();
    }

    token next_name()
    {
        const char* p_head = mp_cur;

        for (; mp_cur != mp_end; ++mp_cur)
        {
            switch (*mp_cur)
            {
                case '[':
                    return name_to_token(p_head, mp_cur - p_head);
                default:
                    ;
            }
        }

        return json_path_token_t::unknown;
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

private:
    token name_to_token(const char* p, size_t n) const
    {
        pstring name(p, n);

        if (name == "array")
            return json_path_token_t::array;

        return json_path_token_t::unknown;
    }
};

} // anonymous namespace

json_map_tree::path_error::path_error(const std::string& msg) :
    general_error(msg) {}

json_map_tree::cell_reference_type::cell_reference_type(const cell_position_t& _pos) :
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

json_map_tree::json_map_tree() {}
json_map_tree::~json_map_tree() {}

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

void json_map_tree::start_range(const cell_position_t& pos) {}
void json_map_tree::append_field_link(const pstring& path)
{
    node* p = get_or_create_destination_node(path);
    if (!p)
        return;
}

void json_map_tree::set_range_row_group(const pstring& path)
{
    node* p = get_or_create_destination_node(path);
    if (!p)
        return;
}

void json_map_tree::commit_range() {}

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

                return &it->second;
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
                std::cout << __FILE__ << ":" << __LINE__ << " (json_map_tree:get_linked_node): array pos = " << t.array_pos << std::endl;
                break;
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
