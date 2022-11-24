/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xml_element_types.hpp"

namespace orcus {

size_t xml_token_pair_hash::operator()(const xml_token_pair_t& v) const
{
    return std::hash<const char*>()(v.first) ^ std::hash<size_t>()(v.second);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
