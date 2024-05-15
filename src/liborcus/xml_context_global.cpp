/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xml_context_global.hpp"
#include "orcus/string_pool.hpp"
#include "orcus/measurement.hpp"

#include <algorithm>

namespace orcus {

std::string_view get_single_attr(
    const xml_token_attrs_t& attrs, xmlns_id_t ns, xml_token_t name, string_pool* pool)
{
    std::string_view value;

    for (const auto& attr : attrs)
    {
        if (attr.name != name)
            continue;

        if (attr.ns && attr.ns != ns)
            continue;

        value = attr.value;
        if (attr.transient && pool)
            value = pool->intern(value).first;
    }

    return value;
}

std::optional<long> get_single_long_attr(const xml_token_attrs_t& attrs, xmlns_id_t ns, xml_token_t name)
{
    auto s = get_single_attr(attrs, ns, name);
    if (s.empty())
        return {};

    const char* p_end = nullptr;
    long v = to_long(s, &p_end);
    if (s.data() == p_end)
        return {}; // parsing failed

    return v;
}

std::optional<double> get_single_double_attr(const xml_token_attrs_t& attrs, xmlns_id_t ns, xml_token_t name)
{
    auto s = get_single_attr(attrs, ns, name);
    if (s.empty())
        return {};

    const char* p_end = nullptr;
    double v = to_double(s, &p_end);
    if (s.data() == p_end)
        return {}; // parsing failed

    return v;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
