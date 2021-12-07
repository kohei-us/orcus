/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <orcus/types.hpp>

#include <unordered_map>

namespace orcus {

class xml_element_validator
{
    using rule_map_type = std::unordered_map<xml_token_pair_t, xml_elem_set_t, xml_token_pair_hash>;
    rule_map_type m_rules;

public:

    /** represents a single parent to child mapping rule. It must be a POD. */
    struct rule
    {
        const xmlns_id_t ns_parent;
        const xml_token_t name_parent;
        const xmlns_id_t ns_child;
        const xml_token_t name_child;
    };

    enum class result { unknown, valid, invalid };

    xml_element_validator(const rule* rules, std::size_t n_rules);

    result validate(const xml_token_pair_t& parent, const xml_token_pair_t& child) const;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
