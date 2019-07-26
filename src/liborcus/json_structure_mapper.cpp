/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "json_structure_mapper.hpp"

#include <algorithm>
#include <sstream>

namespace orcus { namespace json { namespace detail {

structure_mapper::structure_mapper(structure_tree::range_handler_type rh, const json::structure_tree::walker& walker) :
    m_walker(walker),
    m_range_handler(std::move(rh)),
    m_repeat_count(0) {}

void structure_mapper::run()
{
    reset();
    traverse(0);
}

void structure_mapper::reset()
{
    m_walker.root();
    m_current_range.paths.clear();
    m_current_range.row_groups.clear();
    m_repeat_count = 0;
}

void structure_mapper::push_range()
{
    m_range_handler(m_current_range);
    m_current_range.paths.clear();
    m_current_range.row_groups.clear();
}

void structure_mapper::traverse(size_t pos)
{
    json::structure_tree::node_properties node = m_walker.get_node();

    if (node.repeat)
    {
        ++m_repeat_count;
        m_current_range.row_groups.push_back(m_walker.build_path_to_parent());
    }

    if (m_repeat_count && node.type == json::structure_tree::node_type::value)
    {
        for (std::string path : m_walker.build_field_paths())
            m_current_range.paths.push_back(std::move(path));
    }

    for (size_t i = 0, n = m_walker.child_count(); i < n; ++i)
    {
        m_walker.descend(i);
        traverse(i);
        m_walker.ascend();
    }

    if (node.repeat)
    {
        --m_repeat_count;

        if (!m_repeat_count)
            push_range();
    }
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
