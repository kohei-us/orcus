/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "filesystem_env.hpp"
#include "pivot_impl.hpp"

namespace orcus { namespace spreadsheet { namespace detail {

class debug_state_context;

class debug_state_dumper_pivot_cache
{
    const pivot_cache::impl& m_store;

public:
    debug_state_dumper_pivot_cache(const pivot_cache::impl& store);

    void dump(const fs::path& outdir) const;
};

class debug_state_dumper_pivot_table
{
    const debug_state_context& m_cxt;
    const pivot_table::impl& m_store;

public:
    debug_state_dumper_pivot_table(const debug_state_context& cxt, const pivot_table::impl& store);

    void dump(const fs::path& outpath, const detail::caches_type& caches) const;

private:
    void dump_rc_items(std::ofstream& of, const pivot_ref_rc_items_t& rc_items) const;

    const pivot_cache::impl* get_cache_store(
        const detail::caches_type& caches, pivot_cache_id_t cache_id) const;
};

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
