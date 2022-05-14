/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <ostream>
#include <string_view>

namespace orcus { namespace detail {

class ostream_format_guard
{
    std::ostream& m_os;
    std::ios_base::fmtflags m_flags;
public:
    ostream_format_guard(std::ostream& os) :
        m_os(os)
    {
        m_flags = m_os.flags();
    }

    ~ostream_format_guard()
    {
        m_os.setf(m_flags);
    }
};

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
