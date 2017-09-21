/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xlsx.hpp"

#ifdef __ORCUS_PYTHON_XLSX
#include "document.hpp"
#include "orcus/orcus_xlsx.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/global.hpp"
#endif

#include <iostream>

using namespace std;

namespace orcus { namespace python {

#ifdef __ORCUS_PYTHON_XLSX

PyObject* xlsx_read(PyObject* /*module*/, PyObject* args, PyObject* kwargs)
{
    static const char* kwlist[] = { "file", nullptr };

    PyObject* file = nullptr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", const_cast<char**>(kwlist), &file))
        return nullptr;

    if (!file)
    {
        PyErr_SetString(PyExc_RuntimeError, "Invalid file object has been passed.");
        return nullptr;
    }

    PyObject* func_read = PyObject_GetAttrString(file, "read");
    if (!func_read)
    {
        PyErr_SetString(PyExc_RuntimeError, "'read' function was expected, but not found.");
        return nullptr;
    }

    PyObject* obj_bytes = PyObject_CallFunction(func_read, nullptr);
    if (!obj_bytes)
    {
        PyErr_SetString(PyExc_RuntimeError, "The read function didn't return bytes.");
        return nullptr;
    }

    std::unique_ptr<spreadsheet::document> doc = orcus::make_unique<spreadsheet::document>();
    spreadsheet::import_factory fact(*doc);
    orcus_xlsx app(&fact);

    import_from_file_object(app, obj_bytes);
    PyObject* obj_doc = create_document_object();
    store_document(obj_doc, std::move(doc));
    return obj_doc;
}

#else

PyObject* xlsx_read(PyObject*, PyObject*, PyObject*)
{
    // TODO : raise a python exception here.
    Py_INCREF(Py_None);
    return Py_None;
}

#endif

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
