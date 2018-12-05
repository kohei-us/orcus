/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_DETAIL_CPU_FEATURES_HPP
#define INCLUDED_ORCUS_DETAIL_CPU_FEATURES_HPP

#ifdef __ORCUS_CPU_FEATURES
#include <cpu_features/cpuinfo_x86.h>
#endif

namespace orcus { namespace detail { namespace cpu {

#ifdef __ORCUS_CPU_FEATURES

inline bool has_sse42()
{
    return !!cpu_features::GetX86Info().features.sse4_2;
}

#else

inline bool has_sse42() { return false; }

#endif

}}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
