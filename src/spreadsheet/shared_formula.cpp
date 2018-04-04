/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "shared_formula.hpp"

namespace orcus { namespace spreadsheet {

shared_formula_pool::shared_formula_pool() {}
shared_formula_pool::~shared_formula_pool() {}

void shared_formula_pool::add(
    size_t sf_index, const ixion::formula_tokens_store_ptr_t& sf_tokens)
{
    m_store.emplace(sf_index, sf_tokens);
}

ixion::formula_tokens_store_ptr_t shared_formula_pool::get(size_t sf_index) const
{
    auto it = m_store.find(sf_index);
    if (it == m_store.end())
        return ixion::formula_tokens_store_ptr_t();

    return it->second;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
