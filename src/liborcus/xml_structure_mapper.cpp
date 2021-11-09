/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xml_structure_mapper.hpp"

namespace orcus { namespace detail {

xml_structure_mapper::xml_structure_mapper(
    xml_structure_tree::range_handler_type rh, const xml_structure_tree::walker& walker) :
        m_range_handler(std::move(rh)),
        m_walker(walker),
        m_repeat_count(0)
{
}

void xml_structure_mapper::run()
{
    reset();
    traverse();
}

void xml_structure_mapper::reset()
{
    m_cur_elem = m_walker.root();
    m_repeat_count = 0;
}

void xml_structure_mapper::traverse()
{
    auto elem = m_cur_elem;
    const bool row_group = elem.repeat;

    if (row_group)
    {
        ++m_repeat_count;
        m_current_range.row_groups.push_back(m_walker.get_path());
    }

    xml_structure_tree::entity_names_type children = m_walker.get_children();

    if (m_repeat_count)
    {
        std::string path = m_walker.get_path();

        xml_structure_tree::entity_names_type attr_names = m_walker.get_attributes();
        for (const auto& attr_name : attr_names)
        {
            std::string attr_path = path + "/@" + m_walker.to_string(attr_name);
            m_current_range.paths.push_back(attr_path);
        }

        if (children.empty() && elem.has_content)
            // Only add leaf elements to the range, and only those with contents.
            m_current_range.paths.push_back(path);
    }

    for (const auto& child_name : children)
    {
        m_cur_elem = m_walker.descend(child_name);
        traverse();
        m_cur_elem = m_walker.ascend();
    }

    if (row_group)
    {
        --m_repeat_count;

        if (!m_repeat_count)
            push_range();
    }
}

void xml_structure_mapper::push_range()
{
    m_range_handler(std::move(m_current_range));
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
