/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_PARSER_UTF8_HPP
#define INCLUDED_ORCUS_PARSER_UTF8_HPP

#include <vector>
#include <cstdint>

namespace orcus {

const char* parse_utf8_xml_name_start_char(const char* p, const char* p_end);

const char* parse_utf8_xml_name_char(const char* p, const char* p_end);

std::vector<char> encode_utf8(uint32_t cp);

uint8_t calc_utf8_byte_length(uint8_t c1);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
