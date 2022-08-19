/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <orcus/types.hpp>

#include <ostream>

namespace orcus {

class tokens;
class xmlns_context;

class xml_element_printer
{
    const tokens& m_tokens;
    const xmlns_context* mp_ns_cxt = nullptr;

public:
    xml_element_printer(const tokens& t);

    void set_ns_context(const xmlns_context* ns_cxt);

    void print_namespace(std::ostream& os, xmlns_id_t ns) const;

    void print_element(std::ostream& os, xmlns_id_t ns, xml_token_t name) const;
};

void print_element(std::ostream& os, const tokens& t, xmlns_id_t ns, xml_token_t name);

/**
 * Print attributes to stdout for debugging purposes.
 */
void print_attrs(const tokens& tokens, const xml_attrs_t& attrs);

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
