/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#ifdef __ORCUS_CPU_FEATURES
#include <immintrin.h>
#include <cstddef>
#include <cstring>
#endif

namespace orcus { namespace detail { namespace cpu {

#ifdef __ORCUS_CPU_FEATURES

constexpr bool has_sse42()
{
#if defined(__SSE4_2__) || defined(__AVX2__)
    return true;
#else
    return false;
#endif
}

constexpr bool has_avx2()
{
#ifdef __AVX2__
    return true;
#else
    return false;
#endif
}

#if defined(__SSE4_2__) || defined(__AVX2__)

/**
 * Load the 16 bytes at p, of which only the first n can be read.  The bytes
 * past n are loaded as zeros.
 */
inline __m128i load_m128(const char* p, std::size_t n)
{
    if (n >= 16)
        return _mm_loadu_si128(reinterpret_cast<const __m128i*>(p));

    alignas(16) char buf[16] = {};
    std::memcpy(buf, p, n);
    return _mm_load_si128(reinterpret_cast<const __m128i*>(buf));
}

#endif

#ifdef __AVX2__

/**
 * Load the 32 bytes at p, of which only the first n can be read.  The bytes
 * past n are loaded as zeros.
 */
inline __m256i load_m256(const char* p, std::size_t n)
{
    if (n >= 32)
        return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(p));

    alignas(32) char buf[32] = {};
    std::memcpy(buf, p, n);
    return _mm256_load_si256(reinterpret_cast<const __m256i*>(buf));
}

#endif

#else

constexpr bool has_sse42() { return false; }
constexpr bool has_avx2() { return false; }

#endif

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
