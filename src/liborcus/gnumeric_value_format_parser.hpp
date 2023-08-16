/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string_view>
#include <vector>

#include "gnumeric_types.hpp"

namespace orcus {

class gnumeric_value_format_parser
{
    const char* m_head = nullptr;
    const char* m_cur = nullptr;
    const char* m_end = nullptr;

    std::vector<gnumeric_value_format_segment> m_segments;

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
    gnumeric_value_format_parser(std::string_view format);

    void parse();

    std::vector<gnumeric_value_format_segment> pop_segments();
};

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
