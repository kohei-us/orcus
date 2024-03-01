/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "debug_utils.hpp"

#include <orcus/config.hpp>

#include <iostream>

namespace orcus {

void warn(const config& conf, std::string_view msg)
{
    if (!conf.debug)
        return;

    std::cerr << "warning: " << msg << std::endl;
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
