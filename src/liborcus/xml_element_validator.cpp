/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xml_element_validator.hpp"

namespace orcus {

xml_element_validator::xml_element_validator()
{
}

xml_element_validator::xml_element_validator(const rule* rules, std::size_t n_rules)
{
    init(rules, n_rules);
}

void xml_element_validator::init(const rule* rules, std::size_t n_rules)
{
    const rule* end_rules = rules + n_rules;

    for (; rules != end_rules; ++rules)
    {
        const xml_token_pair_t parent(rules->ns_parent, rules->name_parent);
        const xml_token_pair_t child(rules->ns_child, rules->name_child);

        auto it = m_rules.find(parent);
        if (it == m_rules.end())
        {
            auto res = m_rules.emplace(parent, xml_elem_set_t{});
            it = res.first;
        }

        xml_elem_set_t& children = it->second;
        children.insert(child);
    }
}

xml_element_validator::result xml_element_validator::validate(
    const xml_token_pair_t& parent, const xml_token_pair_t& child) const
{
    if (m_rules.empty())
        // No rules are defined. Allow everything.
        return result::child_valid;

    auto it = m_rules.find(parent);
    if (it == m_rules.end())
        // No rules for this parent.
        return result::parent_unknown;

    const xml_elem_set_t& rules = it->second;
    return rules.count(child) > 0 ? result::child_valid : result::child_invalid;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
