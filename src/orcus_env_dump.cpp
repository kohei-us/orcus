/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/global.hpp"
#include "cpu_features.hpp"

#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char** argv)
{
    cout << "CPU flags:" << endl;
    cout << "  SSE 4.2: " << orcus::detail::cpu::has_sse42() << endl;

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
