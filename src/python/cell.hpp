/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_PYTHON_CELL_HPP
#define INCLUDED_ORCUS_PYTHON_CELL_HPP

#include <string>
#include <Python.h>

namespace ixion {

class formula_cell;
struct abs_address_t;

}

namespace orcus {

namespace spreadsheet {

class document;

}

namespace python {

PyObject* create_cell_object_empty();
PyObject* create_cell_object_boolean(bool v);
PyObject* create_cell_object_string(const std::string* p);
PyObject* create_cell_object_numeric(double v);
PyObject* create_cell_object_formula(
    const spreadsheet::document& doc, const ixion::abs_address_t& pos, const ixion::formula_cell* fc);

PyTypeObject* get_cell_type();

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
