/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_ORCUS_XML_IMPL_HPP
#define INCLUDED_ORCUS_ORCUS_XML_IMPL_HPP

#include "orcus/orcus_xml.hpp"
#include "orcus/xml_namespace.hpp"

#include "xml_map_tree.hpp"

namespace orcus {

struct orcus_xml::impl
{
    spreadsheet::iface::import_factory* mp_import_factory;
    spreadsheet::iface::export_factory* mp_export_factory;

    /** xml namespace repository for the whole session. */
    xmlns_repository& m_ns_repo;

    /** xml namespace context  */
    xmlns_context m_ns_cxt_map;

    /** xml element tree that represents all mapped paths. */
    xml_map_tree m_map_tree;

    spreadsheet::sheet_t m_sheet_count;

    /**
     * Positions of all linked elements, single and range reference alike.
     * Stored link elements must be sorted in order of stream positions, and
     * as such, no linked elements should be nested; there should never be a
     * linked element inside the substructure of another linked element.
     */
    xml_map_tree::const_element_list_type m_link_positions;

    xml_map_tree::cell_position m_cur_range_ref;

    explicit impl(xmlns_repository& ns_repo);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
