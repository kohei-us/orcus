/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_PYTHON_GLOBAL_HPP
#define INCLUDED_ORCUS_PYTHON_GLOBAL_HPP

#include <exception>
#include <Python.h>

namespace orcus { namespace python {

void set_python_exception(PyObject* type, const std::exception& e);

PyObject* get_python_enum_value(const char* enum_class_name, const char* value_name);

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
