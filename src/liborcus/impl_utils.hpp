/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#define ENSURE_INTERFACE(PTR, NAME) \
    do \
    { \
        if (!PTR) \
            throw orcus::interface_error( \
                "implementer must provide a concrete instance of " #NAME "."); \
    } while (false)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
