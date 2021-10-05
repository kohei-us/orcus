/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_ORCUS_JSON_HPP
#define INCLUDED_ORCUS_ORCUS_JSON_HPP

#include "env.hpp"
#include "./spreadsheet/types.hpp"

#include <memory>
#include <string_view>

namespace orcus {

namespace spreadsheet { namespace iface {

class import_factory;

}}

class ORCUS_DLLPUBLIC orcus_json
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:

    orcus_json(const orcus_json&) = delete;
    orcus_json& operator= (const orcus_json&) = delete;

    orcus_json(spreadsheet::iface::import_factory* im_fact);
    ~orcus_json();

    void set_cell_link(std::string_view path, std::string_view sheet, spreadsheet::row_t row, spreadsheet::col_t col);

    void start_range(
        std::string_view sheet, spreadsheet::row_t row, spreadsheet::col_t col, bool row_header);

    void append_field_link(std::string_view path, std::string_view label);
    void set_range_row_group(std::string_view path);
    void commit_range();

    void append_sheet(std::string_view name);

    void read_stream(std::string_view stream);

    /**
     * Read a JSON string that contains an entire set of mapping rules.
     *
     * This method also inserts all necessary sheets into the document model.
     *
     * @param stream JSON string.
     */
    void read_map_definition(std::string_view stream);

    /**
     * Read a JSON string, and detect and define mapping rules for one or more
     * ranges.
     *
     * @param stream JSON string.
     */
    void detect_map_definition(std::string_view stream);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
