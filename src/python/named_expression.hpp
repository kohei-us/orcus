/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_NAMED_EXPRESSION_HPP
#define INCLUDED_NAMED_EXPRESSION_HPP

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

/** non-python part. */
struct named_exp_data
{
    named_exp_data();
    ~named_exp_data();
};

PyObject* create_named_exp_object(
    spreadsheet::sheet_t origin_sheet, const spreadsheet::document& doc, const ixion::named_expression_t* exp);

PyObject* create_named_exp_dict(
    spreadsheet::sheet_t origin_sheet, const spreadsheet::document& doc, ixion::named_expressions_iterator iter);

PyTypeObject* get_named_exp_type();

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
