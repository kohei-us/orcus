/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_ORCUS_XML_HPP
#define INCLUDED_ORCUS_ORCUS_XML_HPP

#include "env.hpp"
#include "spreadsheet/types.hpp"

#include <ostream>
#include <memory>

namespace orcus {

class pstring;
class xmlns_repository;
struct orcus_xml_impl;

namespace spreadsheet { namespace iface {
    class import_factory;
    class export_factory;
}}

class ORCUS_DLLPUBLIC orcus_xml
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

    void read_impl(const pstring& strm);

public:
    orcus_xml(const orcus_xml&) = delete;
    orcus_xml& operator= (const orcus_xml&) = delete;

    orcus_xml(xmlns_repository& ns_repo, spreadsheet::iface::import_factory* im_fact, spreadsheet::iface::export_factory* ex_fact);
    ~orcus_xml();

    void set_namespace_alias(const pstring& alias, const pstring& uri);

    void set_cell_link(const pstring& xpath, const pstring& sheet, spreadsheet::row_t row, spreadsheet::col_t col);

    void start_range(const pstring& sheet, spreadsheet::row_t row, spreadsheet::col_t col);
    void append_field_link(const pstring& xpath);
    void set_range_row_group(const pstring& xpath);
    void commit_range();

    void append_sheet(const pstring& name);

    void read_stream(const char* p, size_t n);

    /**
     * Read an XML string that contains an entire set of mapping rules.
     *
     * This method also inserts all necessary sheets into the document model.
     *
     * @param p pointer to the in-memory buffer that contains the XML string.
     * @param n size of the buffer.
     */
    void read_map_definition(const char* p, size_t n);

    void detect_map_definition(const char* p, size_t n);

    void write(const char* p_in, size_t n_in, std::ostream& out) const;
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
