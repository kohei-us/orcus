/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "csv.hpp"
#include "global.hpp"

#ifdef __ORCUS_PYTHON_CSV
#include "document.hpp"
#include "orcus/orcus_csv.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/global.hpp"
#endif

namespace orcus { namespace python {

#ifdef __ORCUS_PYTHON_CSV

namespace {

py_unique_ptr read_stream_object_from_string(PyObject* args, PyObject* kwargs)
{
    static const char* kwlist[] = { "stream", nullptr };

    py_unique_ptr ret;
    PyObject* file = nullptr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", const_cast<char**>(kwlist), &file))
        return ret;

    if (!file)
    {
        PyErr_SetString(PyExc_RuntimeError, "Invalid file object has been passed.");
        return ret;
    }

    PyObject* obj_str = nullptr;

    if (PyObject_HasAttrString(file, "read"))
    {
        PyObject* func_read = PyObject_GetAttrString(file, "read"); // new reference
        obj_str = PyObject_CallFunction(func_read, nullptr);
        Py_XDECREF(func_read);
    }

    if (!obj_str)
    {
        if (PyObject_TypeCheck(file, &PyUnicode_Type))
            obj_str = PyUnicode_FromObject(file); // new reference
    }

    if (!obj_str)
    {
        PyErr_SetString(PyExc_RuntimeError, "failed to extract bytes from this object.");
        return ret;
    }

    ret.reset(obj_str);
    return ret;
}

} // anonymous namespace

PyObject* csv_read(PyObject* /*module*/, PyObject* args, PyObject* kwargs)
{
    py_unique_ptr str = read_stream_object_from_string(args, kwargs);
    if (!str)
        return nullptr;

    try
    {
        spreadsheet::range_size_t ss{1048576, 16384};
        std::unique_ptr<spreadsheet::document> doc = orcus::make_unique<spreadsheet::document>(ss);
        spreadsheet::import_factory fact(*doc);
        orcus_csv app(&fact);

        Py_ssize_t n = 0;
        const char* p = PyUnicode_AsUTF8AndSize(str.get(), &n);
        app.read_stream(p, n);

        return create_document(std::move(doc));
    }
    catch (const std::exception& e)
    {
        set_python_exception(PyExc_RuntimeError, e);
        return nullptr;
    }
}

#else

PyObject* csv_read(PyObject*, PyObject*, PyObject*)
{
    PyErr_SetString(PyExc_RuntimeError, "The csv module is not enabled.");
    return nullptr;
}

#endif

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
