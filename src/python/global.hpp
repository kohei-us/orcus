/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_PYTHON_GLOBAL_HPP
#define INCLUDED_ORCUS_PYTHON_GLOBAL_HPP

#include "memory.hpp"

#include <exception>
#include <string_view>
#include <Python.h>

namespace orcus { namespace python {

void set_python_exception(PyObject* type, const std::exception& e);

PyObject* get_python_enum_value(const char* enum_class_name, const char* value_name);

PyObject* create_object_from_type(PyTypeObject* type);

/**
 * Set a new-reference value into a dict under the given key.
 *
 * @param dict Dictionary object to set the value to.
 * @param key Key to associated the value with.
 * @param value Value object to set into the dict.
 *
 * @return true if the value is successfully set, otherwise false.
 */
bool set_dict_item_new(PyObject* dict, const char* key, py_scoped_ref value);

/**
 * Set a new-reference key into a set.
 *
 * @param set Set object to set the key to.
 * @param key Key to set.
 *
 * @return true if the value is successfully set, otherwise false.
 */
bool add_to_set_new(PyObject* set, py_scoped_ref key);

py_scoped_ref from_long(long v);

py_scoped_ref from_string(std::string_view s);

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
