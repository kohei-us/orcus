/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "orcus/env.hpp"

#include <string>

namespace orcus { namespace json {

/**
 * Properly escape an input string appropriate for json output.
 *
 * @param input string value to escape.
 *
 * @return escaped string value.
 */
ORCUS_PSR_DLLPUBLIC std::string escape_string(std::string_view input);

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
