/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "session_context.hpp"

namespace orcus {

session_context::custom_data::~custom_data() {}

session_context::session_context() : mp_data(nullptr) {}
session_context::session_context(custom_data* data) : mp_data(data) {}

session_context::~session_context()
{
    mp_data.reset();
}

pstring session_context::intern(const xml_token_attr_t& attr)
{
    if (!attr.transient)
        return attr.value;

    return m_string_pool.intern(attr.value).first;
}

pstring session_context::intern(const pstring& s)
{
    return m_string_pool.intern(s).first;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
