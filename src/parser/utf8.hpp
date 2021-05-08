/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_PARSER_UTF8_HPP
#define INCLUDED_ORCUS_PARSER_UTF8_HPP

namespace orcus {

const char* parse_utf8_xml_name_start_char(const char* p, const char* p_end);

const char* parse_utf8_xml_name_char(const char* p, const char* p_end);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
