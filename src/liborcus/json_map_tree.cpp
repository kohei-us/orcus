/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "json_map_tree.hpp"
#include "orcus/global.hpp"

namespace orcus {

struct json_map_tree::impl
{
};

json_map_tree::json_map_tree() : mp_impl(orcus::make_unique<impl>()) {}
json_map_tree::~json_map_tree() {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
