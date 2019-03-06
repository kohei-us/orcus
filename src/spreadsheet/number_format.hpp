/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_NUMBER_FORMAT_HPP
#define INCLUDED_ORCUS_SPREADSHEET_NUMBER_FORMAT_HPP

#include <iosfwd>

namespace orcus { namespace spreadsheet { namespace detail {

/**
 * Format a numeric value to a lossless string representation appripriate
 * for file output.
 *
 * @param os output stream to add the string representation to.
 * @param v source numeric value to format.
 */
void format_to_file_output(std::ostream& os, double v);

}}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
