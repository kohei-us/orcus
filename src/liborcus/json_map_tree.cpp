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
        long array_pos = -1;

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
                return -1;
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

json_map_tree::json_map_tree() {}
json_map_tree::~json_map_tree() {}

void json_map_tree::set_cell_link(const pstring& path, const cell_position_t& pos)
{
    node* p = get_linked_node(path);
    if (!p)
        return;
}

void json_map_tree::start_range(const cell_position_t& pos) {}
void json_map_tree::append_field_link(const pstring& path)
{
    node* p = get_linked_node(path);
    if (!p)
        return;
}

void json_map_tree::set_range_row_group(const pstring& path)
{
    node* p = get_linked_node(path);
    if (!p)
        return;
}

void json_map_tree::commit_range() {}

json_map_tree::node* json_map_tree::get_linked_node(const pstring& path)
{
    std::cout << __FILE__ << ":" << __LINE__ << " (json_map_tree:get_linked_node): path='" << path << "'" << std::endl;

    if (path.empty() || path[0] != '$')
        // A valid path must begin with a '$'.
        return nullptr;

    json_path_parser parser(path);

    for (auto t = parser.next(); t.type != json_path_token_t::unknown; t = parser.next())
    {
        switch (t.type)
        {
            case json_path_token_t::array:
                std::cout << __FILE__ << ":" << __LINE__ << " (json_map_tree:get_linked_node): array" << std::endl;
                break;
            case json_path_token_t::array_pos:
                std::cout << __FILE__ << ":" << __LINE__ << " (json_map_tree:get_linked_node): array pos = " << t.array_pos << std::endl;
                break;
            case json_path_token_t::end:
                std::cout << __FILE__ << ":" << __LINE__ << " (json_map_tree:get_linked_node): end" << std::endl;
                return nullptr;
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
