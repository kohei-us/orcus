/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <orcus/types.hpp>

namespace orcus {

/**
 * Holds a pair of XML namespace identifier and an element token.  Typically
 * used when managing the element stack inside element context classes.
 */
using xml_token_pair_t = std::pair<xmlns_id_t, xml_token_t>;

struct xml_token_pair_hash
{
    size_t operator()(const xml_token_pair_t& v) const;
};

using xml_elem_stack_t = std::vector<xml_token_pair_t>;
using xml_elem_set_t = std::unordered_set<xml_token_pair_t, xml_token_pair_hash>;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
