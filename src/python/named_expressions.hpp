/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_PYTHON_NAMED_EXPRESSIONS_HPP
#define INCLUDED_ORCUS_PYTHON_NAMED_EXPRESSIONS_HPP

#include "orcus/spreadsheet/types.hpp"

#include <Python.h>
#include <ixion/formula_tokens_fwd.hpp>

namespace ixion {

class named_expressions_iterator;

}

namespace orcus {

namespace spreadsheet {

class document;

}

namespace python {

PyObject* create_named_expressions_object(
    spreadsheet::sheet_t origin_sheet, const spreadsheet::document& doc, ixion::named_expressions_iterator iter);

PyTypeObject* get_named_exps_type();

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
