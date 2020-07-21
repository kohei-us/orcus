/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_XML_STRUCTURE_MAPPER_HPP
#define INCLUDED_XML_STRUCTURE_MAPPER_HPP

#include "orcus/xml_structure_tree.hpp"

namespace orcus { namespace detail {

class xml_structure_mapper
{
    xml_table_range_t m_current_range;

    xml_structure_tree::range_handler_type m_range_handler;
    xml_structure_tree::walker m_walker;
    xml_structure_tree::element m_cur_elem;
    size_t m_repeat_count;

    void reset();

    void traverse();

    void push_range();

public:
    xml_structure_mapper(xml_structure_tree::range_handler_type rh, const xml_structure_tree::walker& walker);

    void run();
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
