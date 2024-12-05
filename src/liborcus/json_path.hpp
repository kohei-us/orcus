/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <variant>
#include <deque>
#include <string_view>

namespace orcus {

enum class json_path_t
{
    unknown,
    root,        // $
    array_index, // [0]
    array_all,   // [*]
    object_key   // .key
};

class json_path_part_t
{
    using value_type = std::variant<std::monostate, std::size_t, std::string_view>;

    json_path_t m_type;
    value_type m_value;

public:
    json_path_part_t(json_path_t type);
    json_path_part_t(std::size_t array_index);
    json_path_part_t(std::string_view object_key);

    json_path_t type() const;
    std::string_view object_key() const;
    std::size_t array_index() const;

    bool operator==(const json_path_part_t& other) const;
    bool operator!=(const json_path_part_t& other) const;
};

using json_path_parts_t = std::deque<json_path_part_t>;

class json_path_parser
{
    const char* mp = nullptr;
    const char* mp_end = nullptr;

    json_path_parts_t m_parts;

    void object_key();
    void object_key_in_brackets();
    void bracket();

public:
    void parse(std::string_view exp);

    [[nodiscard]] json_path_parts_t pop_parts();
};

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
