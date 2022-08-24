/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "cpu_features.hpp"

#include <iostream>

using std::cout;
using std::endl;

int main()
{
    cout << "CPU flags:" << endl;
    cout << "  SSE 4.2: " << orcus::detail::cpu::has_sse42() << endl;
    cout << "  AVX2: " << orcus::detail::cpu::has_avx2() << endl;

#if defined(_MSC_VER)
    cout << "MSVC macros:" << endl;
    #ifdef _M_IX86_FP
    cout << "  _M_IX86_FP: " << _M_IX86_FP << endl;
    #else
    cout << "  _M_IX86_FP: not defined" << endl;
    #endif
#endif

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
