/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/json_structure_tree.hpp"
#include "orcus/json_parser.hpp"

namespace orcus {

struct json_structure_tree::impl
{
    impl() {}
    ~impl() {}

    void begin_parse() {}

    void end_parse() {}

    void begin_array() {}

    void end_array() {}

    void begin_object() {}

    void object_key(const char* p, size_t len, bool transient) {}

    void end_object() {}

    void boolean_true() {}

    void boolean_false() {}

    void null() {}

    void string(const char* p, size_t len, bool transient) {}

    void number(double val) {}
};

json_structure_tree::json_structure_tree() : mp_impl(std::make_unique<impl>()) {}
json_structure_tree::~json_structure_tree() {}

void json_structure_tree::parse(const char* p, size_t n)
{
    json_parser<impl> parser(p, n, *mp_impl);
    parser.parse();
}

void json_structure_tree::dump_compact(std::ostream& os) const {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
