/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "session_context.hpp"

namespace orcus {

session_context::custom_data::~custom_data() {}

session_context::session_context(std::unique_ptr<custom_data> data) : cdata(std::move(data)) {}

std::string_view session_context::intern(const xml_token_attr_t& attr)
{
    // NB: always intern regardless of the transient flag since the string may
    // be used in another stream.
    return spool.intern(attr.value).first;
}

std::string_view session_context::intern(std::string_view s)
{
    return spool.intern(s).first;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
