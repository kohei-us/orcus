/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ORCUS_BASE64_HPP__
#define __ORCUS_BASE64_HPP__

#include "env.hpp"
#include <vector>
#include <string>

namespace orcus {

/**
 * Decode a based64-encoded character sequence into a sequence of bytes.
 *
 * @param base64 encoded character sequence.
 * @return decoded byte sequence.
 */
ORCUS_PSR_DLLPUBLIC std::vector<uint8_t> decode_from_base64(std::string_view base64);

/**
 * Encode a sequence of bytes into base64-encoded characters.
 *
 * @param input sequence of bytes to encode.
 * @return base64-encoded character sequence representing the input bytes.
 */
ORCUS_PSR_DLLPUBLIC std::string encode_to_base64(const std::vector<uint8_t>& input);

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
