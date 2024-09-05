/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <orcus/spreadsheet/types.hpp>

#include <string>
#include <utility>

namespace orcus { namespace detail {

class xls_filter_value_parser
{
    std::string m_buf;

    const char* mp_char = nullptr;
    const char* mp_end = nullptr;
    bool m_regex = false;

public:
    struct result_type
    {
        spreadsheet::auto_filter_op_t op;
        std::string_view value;
        bool regex;
    };

    result_type parse(spreadsheet::auto_filter_op_t op, std::string_view value);

private:
    void parse_chars();

};

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
