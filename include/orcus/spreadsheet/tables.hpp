/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "../env.hpp"
#include "types.hpp"

#include <memory>
#include <string_view>
#include <map>

namespace ixion {

class model_context;

}

namespace orcus {

class string_pool;

namespace spreadsheet {

namespace detail { struct document_impl; }

struct table_t;

class ORCUS_SPM_DLLPUBLIC tables
{
    friend struct detail::document_impl;

    tables(string_pool& sp, ixion::model_context& context);

public:
    tables() = delete;
    tables(const tables&) = delete;
    ~tables();

    tables& operator=(const tables&) = delete;

    /**
     * Insert a new table instance.
     *
     * @param p Table instance to insert.
     */
    void insert(std::unique_ptr<table_t> p);

    /**
     * Get a structure containing properties of a named table.
     *
     * @param name Name of the table.
     *
     * @return Weak pointer to the structure containing the properties of a
     *         named table, or an empty pointer if no such table exists for the
     *         given name.
     */
    std::weak_ptr<const table_t> get(std::string_view name) const;

    /**
     * Get all tables belonging to a certain sheet by sheet index.
     *
     * @param pos 0-based sheet index.
     *
     * @return Map containing pointers to all table instances belonging to
     *         specified sheet and their respective names as keys.
     */
    std::map<std::string_view, std::weak_ptr<const table_t>> get_by_sheet(sheet_t pos) const;

private:
    struct impl;
    std::unique_ptr<impl> mp_impl;
};

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
