/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "filesystem_env.hpp"

#include <orcus/spreadsheet/pivot.hpp>

namespace orcus { namespace spreadsheet { namespace detail {

class debug_state_dumper_pivot_cache
{
    const pivot_cache::impl& m_store;

public:
    debug_state_dumper_pivot_cache(const pivot_cache::impl& store);

    void dump(const fs::path& outdir) const;
};

class debug_state_dumper_pivot_table
{
    const pivot_table::impl& m_store;

public:
    debug_state_dumper_pivot_table(const pivot_table::impl& store);

    void dump(const fs::path& outdir) const;
};

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
