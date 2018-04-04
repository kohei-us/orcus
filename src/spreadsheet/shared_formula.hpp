/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_SHARED_FORMULA_HPP
#define INCLUDED_ORCUS_SPREADSHEET_SHARED_FORMULA_HPP

#include <ixion/formula_tokens.hpp>

#include <unordered_map>

namespace orcus { namespace spreadsheet {

class shared_formula_pool
{
    using store_type = std::unordered_map<size_t, ixion::formula_tokens_store_ptr_t>;

    store_type m_store;

public:
    shared_formula_pool();
    ~shared_formula_pool();

    void add(size_t sf_index, const ixion::formula_tokens_store_ptr_t& sf_tokens);

    ixion::formula_tokens_store_ptr_t get(size_t sf_index) const;
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
