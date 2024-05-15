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

single_long_attr_getter::single_long_attr_getter(xmlns_id_t ns, xml_token_t name) :
    m_value(-1), m_ns(ns), m_name(name) {}

void single_long_attr_getter::operator() (const xml_token_attr_t& attr)
{
    if (attr.name != m_name)
        return;

    if (attr.ns && attr.ns != m_ns)
        return;

    m_value = to_long(attr.value);
}

long single_long_attr_getter::get_value() const
{
    return m_value;
}

long single_long_attr_getter::get(const std::vector<xml_token_attr_t>& attrs, xmlns_id_t ns, xml_token_t name)
{
    single_long_attr_getter func(ns, name);
    return std::for_each(attrs.begin(), attrs.end(), func).get_value();
}

single_double_attr_getter::single_double_attr_getter(xmlns_id_t ns, xml_token_t name) :
    m_value(-1.0), m_ns(ns), m_name(name) {}

void single_double_attr_getter::operator() (const xml_token_attr_t& attr)
{
    if (attr.name != m_name)
        return;

    if (attr.ns && attr.ns != m_ns)
        return;

    m_value = to_double(attr.value);
}

double single_double_attr_getter::get_value() const
{
    return m_value;
}

double single_double_attr_getter::get(const std::vector<xml_token_attr_t>& attrs, xmlns_id_t ns, xml_token_t name)
{
    single_double_attr_getter func(ns, name);
    return std::for_each(attrs.begin(), attrs.end(), func).get_value();
}

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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
