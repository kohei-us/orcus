/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_IFACE_UTIL_HPP
#define INCLUDED_ORCUS_SPREADSHEET_IFACE_UTIL_HPP

#include "orcus/spreadsheet/types.hpp"

namespace orcus {

class pstring;
class range_formula_results;

namespace spreadsheet {

namespace iface {

class import_array_formula;

}

}

void push_array_formula(
    spreadsheet::iface::import_array_formula* xformula,
    const spreadsheet::range_t& range, std::string_view formula,
    spreadsheet::formula_grammar_t grammar, const range_formula_results& results);

} // namespace orcus

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
