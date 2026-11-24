/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cstddef>
#include <sstream>
#include <string>
#include <string_view>

namespace orcus { namespace detail {

/**
 * Temporary replacement for std::format, until the <format> header is usable
 * in all build environments.
 */
template<typename... Args>
std::string format(std::string_view fmt, const Args&... args)
{
    std::ostringstream os;

    auto write_next = [&os, &fmt](const auto& value)
    {
        std::size_t pos = fmt.find("{}");
        if (pos == fmt.npos)
            // no more placeholders
            return;

        os << fmt.substr(0, pos) << value;
        fmt.remove_prefix(pos + 2);
    };

    (write_next(args), ...);
    os << fmt;
    return os.str();
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
