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

void structure_mapper::range_type::sort()
{
    std::sort(paths.begin(), paths.end());
    std::sort(row_groups.begin(), row_groups.end());
}

void structure_mapper::range_type::clear()
{
    paths.clear();
    row_groups.clear();
}

structure_mapper::structure_mapper(range_handler_type rh, const json::structure_tree::walker& walker) :
    m_walker(walker),
    m_repeat_count(0),
    m_range_count(0),
    m_sheet_name_prefix("range-"),
    m_sort_before_push(false),
    m_range_handler(std::move(rh)) {}

void structure_mapper::run()
{
    reset();
    traverse(0);
}

void structure_mapper::reset()
{
    m_walker.root();
    m_current_range.clear();
    m_repeat_count = 0;
}

void structure_mapper::push_range()
{
    m_range_handler(m_current_range);
    m_current_range.clear();
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
