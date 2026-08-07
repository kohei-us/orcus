/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "../env.hpp"
#include "types.hpp"

#include <memory>
#include <string_view>
#include <map>

#include <ixion/table.hpp>

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
     * Insert a new table.  The core properties of the table, which formula
     * expressions may reference, are stored in the ixion model context,
     * while the presentation properties are stored in this store under the
     * same table name.
     *
     * @param core Core properties of the table to store in the ixion model
     *             context.
     * @param pres Presentation properties of the table to store in this
     *             store.
     *
     * @throw std::invalid_argument When the table name is empty, or the
     *        table range is invalid or spans multiple sheets.
     * @throw ixion::model_context_error When a table by the same name
     *        already exists.
     */
    void insert(ixion::table_t core, table_t pres);

    /**
     * Get the presentation properties of a named table.  The rest of the
     * table properties are stored in the ixion model context, accessible
     * through document::get_model_context().
     *
     * @param name Name of the table.
     *
     * @return Pointer to the structure containing the presentation
     *         properties of the table, or nullptr if no such table exists
     *         for the given name.  The returned pointer remains valid for
     *         the lifetime of this store.
     */
    const table_t* get(std::string_view name) const;

    /**
     * Get all tables belonging to a certain sheet by sheet index.
     *
     * @param pos 0-based sheet index.
     *
     * @return Map containing pointers to all table instances belonging to
     *         specified sheet and their respective names as keys.
     */
    std::map<std::string_view, const table_t*> get_by_sheet(sheet_t pos) const;

private:
    struct impl;
    std::unique_ptr<impl> mp_impl;
};

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
