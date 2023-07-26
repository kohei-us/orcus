/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string_view>
#include <optional>
#include <cstdint>

namespace orcus {

std::optional<std::uint16_t> hex_to_uint16(std::string_view s);

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
