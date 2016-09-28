/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/spreadsheet/pivot.hpp"
#include "orcus/global.hpp"
#include "orcus/string_pool.hpp"

namespace orcus { namespace spreadsheet {

struct pivot_cache::impl
{
    string_pool& m_string_pool;

    pstring m_src_sheet_name;

    impl(string_pool& sp) : m_string_pool(sp) {}
};

pivot_cache::pivot_cache(string_pool& sp) :
    mp_impl(orcus::make_unique<impl>(sp)) {}

pivot_cache::~pivot_cache() {}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
