/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/json_global.hpp>
#include <orcus/parser_global.hpp>

#include "utf8.hpp"

#include <sstream>
#include <iomanip>
#include <cassert>

namespace orcus { namespace json {

namespace {

constexpr char backslash = '\\';

}

std::string escape_string(std::string_view input)
{
    std::ostringstream os;

    const char* p = input.data();
    const char* p_end = p + input.size();

    while (p < p_end)
    {
        char c = *p;
        auto n = calc_utf8_byte_length(c);
        if (n > 1)
        {
            // utf-8 character
            if (std::next(p, n) > p_end)
            {
                std::ostringstream err;
                err << __FILE__ << ':' << __LINE__ << ": utf-8 bytes of length "
                    << n << " was expected, but the string does not have enough bytes left";
                throw std::runtime_error(err.str());
            }

            std::string_view sub{p, n};
            os << sub;
            p += n;
            continue;
        }

        assert(n == 1);

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
            {
                if (std::iscntrl(c))
                {
                    os << "\\u" << std::setw(4) << std::setfill('0') << std::hex << short(c);
                    break;
                }

                os << c;
            }
        }
        ++p;
    }

    return os.str();
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
