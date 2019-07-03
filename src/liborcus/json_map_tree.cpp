/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "json_map_tree.hpp"
#include "orcus/global.hpp"

#include <iostream>

namespace orcus {

namespace {

enum class json_path_token_t { unknown, array };

class json_path_parser
{
    const char* mp_cur;
    const char* mp_end;
public:

    struct token
    {
        json_path_token_t type = json_path_token_t::unknown;

        token(json_path_token_t _type) : type(_type) {}
    };

    json_path_parser(const pstring& path) :
        mp_cur(path.data()), mp_end(mp_cur + path.size())
    {
        assert(!path.empty());
        assert(path[0] == '$');
        ++mp_cur; // skip the first '$'.
    }

    token next()
    {
        const char* p_head = mp_cur;

        for (; mp_cur != mp_end; ++mp_cur)
        {
            switch (*mp_cur)
            {
                case '[':
                    return to_token(p_head, mp_cur - p_head);
                default:
                    ;
            }
        }

        return json_path_token_t::unknown;
    }

private:
    token to_token(const char* p, size_t n) const
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
void json_map_tree::append_field_link(const pstring& path) {}
void json_map_tree::set_range_row_group(const pstring& path) {}
void json_map_tree::commit_range() {}

json_map_tree::node* json_map_tree::get_linked_node(const pstring& path)
{
    std::cout << __FILE__ << ":" << __LINE__ << " (json_map_tree:get_linked_node): path='" << path << "'" << std::endl;

    json_path_parser parser(path);
    for (auto t = parser.next(); t.type != json_path_token_t::unknown; t = parser.next())
    {
        switch (t.type)
        {
            case json_path_token_t::array:
                std::cout << __FILE__ << ":" << __LINE__ << " (json_map_tree:get_linked_node): array" << std::endl;
                break;
            case json_path_token_t::unknown:
            default:
                std::cout << __FILE__ << ":" << __LINE__ << " (json_map_tree:get_linked_node): unknown" << std::endl;
        }
    }

    return nullptr;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
