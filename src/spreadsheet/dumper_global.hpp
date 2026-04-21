/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <ixion/model_context.hpp>
#include <ixion/model_iterator.hpp>

#include <ostream>
#include <functional>
#include <string>

namespace ixion {
    class formula_cell;
    class formula_name_resolver;
}

namespace orcus { namespace spreadsheet { namespace detail {

using func_str_handler = std::function<void(std::ostream&, const std::string&)>;
using func_empty_handler = std::function<void(std::ostream&)>;

/**
 * Dump a formula cell's expression to an output stream, wrapping it in
 * curly braces if the cell belongs to a grouped (array) formula.
 *
 * @param os Output stream to write the formula expression to.
 * @param cxt Model context used for resolving formula tokens.
 * @param pos Position of the formula cell.
 * @param resolver Name resolver used to print cell references; if null,
 *                 the expression is written as @c "???".
 * @param cell Formula cell whose expression is to be written.
 */
void dump_formula_expression(
    std::ostream& os,
    const ixion::model_context& cxt,
    ixion::abs_address_t pos,
    const ixion::formula_name_resolver* resolver,
    const ixion::formula_cell& cell);

void dump_cell_value(
    std::ostream& os, const ixion::model_context& cxt, const ixion::model_iterator::cell& cell,
    func_str_handler str_handler,
    func_empty_handler empty_handler);

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
