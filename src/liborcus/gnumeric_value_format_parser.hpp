/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string_view>

#include "gnumeric_types.hpp"

namespace orcus {

struct config;

class gnumeric_value_format_parser
{
    const config& m_config;

    const char* m_head = nullptr;
    const char* m_cur = nullptr;
    const char* m_end = nullptr;

    value_format_segments_type m_segments;

private:
    std::size_t get_pos() const;

    void segment();

public:
    /**
     * Constructor.
     *
     * @param format Format string containing one or more format segments. Make
     *               sure the source of this string is persisent!
     */
    gnumeric_value_format_parser(const config& conf, std::string_view format);

    void parse();

    value_format_segments_type pop_segments();
};

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
