/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gnumeric_types.hpp"

namespace orcus {

void gnumeric_named_exp::reset()
{
    name = std::string_view{};
    value = std::string_view{};
    position = {0, 0, 0};
}

bool gnumeric_style::valid() const
{
    if (sheet < 0)
        return false;

    if (region.first.column < 0 || region.first.row < 0 || region.last.column < 0 || region.last.row < 0)
        return false;

    return true;
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
