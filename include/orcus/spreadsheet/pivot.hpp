/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_PIVOT_HPP
#define INCLUDED_ORCUS_SPREADSHEET_PIVOT_HPP

#include "orcus/env.hpp"

#include <memory>

namespace ixion {

struct abs_range_t;

}

namespace orcus {

class string_pool;
class pstring;

namespace spreadsheet {

class document;

class ORCUS_SPM_DLLPUBLIC pivot_cache
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    pivot_cache(string_pool& sp);
    ~pivot_cache();
};

class ORCUS_SPM_DLLPUBLIC pivot_collection
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    pivot_collection(document& doc);
    ~pivot_collection();

    /**
     * Insert a new pivot cache associated with a worksheet source.
     *
     * @param sheet_name name of the sheet where the source data is.
     * @param range range of the source data.  Note that the sheet indices are
     *              not used.
     */
    void insert_worksheet_cache(
        const pstring& sheet_name, const ixion::abs_range_t& range, std::unique_ptr<pivot_cache>&& cache);
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
