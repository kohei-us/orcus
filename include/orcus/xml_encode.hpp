/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "./types.hpp"
#include <ostream>
#include <string_view>

namespace orcus {

/**
 * Write a string value to an output stream with XML special characters
 * encoded according to the given encoding context.
 *
 * @param os  Output stream to write to.
 * @param val String value to encode.
 * @param cxt Encoding context that determines which characters are encoded.
 */
ORCUS_PSR_DLLPUBLIC void write_content_encoded(
    std::ostream& os, std::string_view val, xml_encode_context_t cxt);

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
