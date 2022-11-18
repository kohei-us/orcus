/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_TOKENS_HPP
#define INCLUDED_ORCUS_TOKENS_HPP

#include "types.hpp"

#include <algorithm>
#include <unordered_map>

namespace orcus {

/**
 * XML token store that provides mapping of integral token indentifiers and
 * their original names.  Instances of this class are typically used as global
 * constants.
 *
 * @note The string values for the original token names should be static
 *       values whose values and memory addresses remain unchanged during the
 *       life cycle of the instance that references them.
 *
 * @note This class is not copy-constructible.
 */
class ORCUS_PSR_DLLPUBLIC tokens
{
public:
    tokens() = delete;
    tokens(const tokens&) = delete;
    tokens(const char** token_names, size_t token_name_count);
    ~tokens();

    /**
     * Check if a token returned from get_token() method is valid.
     *
     * @return true if valid, false otherwise.
     */
    bool is_valid_token(xml_token_t token) const;

    /**
     * Get token from a specified name.
     *
     * @param name textural token name
     *
     * @return token value representing the given textural token.
     */
    xml_token_t get_token(std::string_view name) const;

    /**
     * Get textural token name from a token value.
     *
     * @param token numeric token value
     *
     * @return textural token name, or empty string in case the given token is
     *         not valid.
     */
    std::string_view get_token_name(xml_token_t token) const;

private:
    using token_map_type = std::unordered_map<std::string_view, xml_token_t>;

    token_map_type m_tokens;
    const char** m_token_names;
    size_t m_token_name_count;
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
