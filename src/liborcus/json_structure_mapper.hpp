/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/json_structure_tree.hpp"

#include <vector>
#include <functional>

namespace orcus { namespace json { namespace detail {

class structure_mapper
{
public:
    structure_mapper(json::structure_tree::range_handler_type rh, const json::structure_tree::walker& walker);

    void run();

private:
    void reset();
    void push_range();
    void traverse(size_t pos);

private:
    json::structure_tree::walker m_walker;
    json::structure_tree::range_handler_type m_range_handler;
    size_t m_repeat_count;
    json::table_range_t m_current_range;

};

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
