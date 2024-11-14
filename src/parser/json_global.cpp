/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/json_global.hpp"
#include "orcus/parser_global.hpp"

#include <sstream>

namespace orcus { namespace json {

namespace {

constexpr char backslash = '\\';

}

std::string escape_string(const std::string& input)
{
    std::ostringstream os;

    for (auto it = input.begin(), ite = input.end(); it != ite; ++it)
    {
        char c = *it;
        switch (c)
        {
            case '"':
                // Escape double quote, but not forward slash.
                os << backslash << c;
                break;
            case backslash:
                os << backslash << backslash;
                break;
            case '\b':
                os << "\\b";
                break;
            case '\f':
                os << "\\f";
                break;
            case '\n':
                os << "\\n";
                break;
            case '\r':
                os << "\\r";
                break;
            case '\t':
                os << "\\t";
                break;
            default:
                os << c;
        }
    }

    return os.str();
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
