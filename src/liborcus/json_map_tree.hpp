/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>

#include "spreadsheet_impl_types.hpp"

namespace orcus {

using spreadsheet::detail::cell_position_t;

class json_map_tree
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    json_map_tree();
    ~json_map_tree();

    void set_cell_link(const pstring& path, const cell_position_t& pos);

    void start_range(const cell_position_t& pos);
    void append_field_link(const pstring& path);
    void set_range_row_group(const pstring& path);
    void commit_range();
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
