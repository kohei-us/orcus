/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "number_format.hpp"

#include <ostream>
#include <iomanip>
#include <limits>

namespace orcus { namespace spreadsheet { namespace detail {

void format_to_file_output(std::ostream& os, double v)
{
    std::ios_base::fmtflags origflags = os.flags();
    os << std::setprecision(std::numeric_limits<double>::digits10 + 1) << v;
    os.setf(origflags);
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
