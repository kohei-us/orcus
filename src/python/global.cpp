/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "global.hpp"
#include "memory.hpp"

#include <sstream>

namespace orcus { namespace python {

void set_python_exception(PyObject* type, const std::exception& e)
{
    std::ostringstream os;
    os << "C++ exception caught: " << e.what();
    PyErr_SetString(type, os.str().data());
}

PyObject* get_python_enum_value(const char* enum_class_name, const char* value_name)
{
    py_scoped_ref orcus_mod = PyImport_ImportModule("orcus");
    if (!orcus_mod)
    {
        PyErr_SetString(PyExc_RuntimeError, "failed to import orcus module.");
        return nullptr;
    }

    py_scoped_ref cls = PyObject_GetAttrString(orcus_mod.get(), enum_class_name);
    if (!cls)
    {
        std::ostringstream os;
        os << "failed to find class orcus." << enum_class_name << ".";
        PyErr_SetString(PyExc_RuntimeError, os.str().data());
        return nullptr;
    }

    return PyObject_GetAttrString(cls.get(), value_name);
}

PyObject* create_object_from_type(PyTypeObject* type)
{
    if (!type)
    {
        PyErr_SetString(PyExc_RuntimeError, "Type object is null.");
        return nullptr;
    }

    PyObject* obj = type->tp_new(type, nullptr, nullptr);
    if (!obj)
    {
        std::ostringstream os;
        os << "Failed to instantiate an object of type " << type->tp_name << ".";
        PyErr_SetString(PyExc_RuntimeError, os.str().data());
        return nullptr;
    }

    return obj;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
