/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <algorithm>
#include <cstddef>

namespace orcus { namespace detail {

// Reserve space for n elements, but cap how much is allocated up front so a
// count taken from the input cannot ask for a huge block. The container still
// grows as elements are added.
template<typename ContT>
void reserve_bounded(ContT& cont, std::size_t n)
{
    constexpr std::size_t max_bytes = 16u * 1024 * 1024;
    constexpr std::size_t max_n = max_bytes / sizeof(typename ContT::value_type);
    cont.reserve(std::min(n, max_n));
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
