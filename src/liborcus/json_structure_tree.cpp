/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/json_structure_tree.hpp"

namespace orcus {

struct json_structure_tree::impl
{
};

json_structure_tree::json_structure_tree() : mp_impl(std::make_unique<impl>()) {}
json_structure_tree::~json_structure_tree() {}

void json_structure_tree::parse(const char* p, size_t n) {}

void json_structure_tree::dump_compact(std::ostream& os) const {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
