/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string_view>

namespace orcus {

struct config;

void warn(const config& conf, std::string_view msg);

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
