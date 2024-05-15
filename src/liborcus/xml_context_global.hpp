/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_XML_CONTEXT_GLOBAL_HPP
#define ORCUS_XML_CONTEXT_GLOBAL_HPP

#include <orcus/types.hpp>

#include <functional>
#include <optional>

namespace orcus {

class string_pool;

std::string_view get_single_attr(
    const xml_token_attrs_t& attrs, xmlns_id_t ns, xml_token_t name, string_pool* pool = nullptr);

std::optional<long> get_single_long_attr(
    const xml_token_attrs_t& attrs, xmlns_id_t ns, xml_token_t name);

std::optional<double> get_single_double_attr(
    const xml_token_attrs_t& attrs, xmlns_id_t ns, xml_token_t name);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
