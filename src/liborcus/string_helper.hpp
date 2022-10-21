/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vector>
#include <string_view>

namespace orcus {

class string_helper
{
public:
    static std::vector<std::string_view> split_string(std::string_view str, const char sep);
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
