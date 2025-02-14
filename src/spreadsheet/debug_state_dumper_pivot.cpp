/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "debug_state_dumper_pivot.hpp"
#include "pivot_impl.hpp"

#include <sstream>
#include <fstream>

namespace orcus { namespace spreadsheet { namespace detail {

debug_state_dumper_pivot_cache::debug_state_dumper_pivot_cache(const pivot_cache::impl& store) :
    m_store(store)
{
}

void debug_state_dumper_pivot_cache::dump(const fs::path& outdir) const
{
    fs::create_directories(outdir);

    std::ostringstream os;
    os << "cache-" << m_store.m_cache_id << ".yaml";
    fs::path outpath = outdir / os.str();
    std::ofstream of{outpath.native()};

    of << "id: " << m_store.m_cache_id << std::endl;
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
