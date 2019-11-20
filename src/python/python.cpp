/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/env.hpp"
#include "orcus/info.hpp"

#include "root.hpp"
#include "xlsx.hpp"
#include "ods.hpp"
#include "csv.hpp"
#include "xls_xml.hpp"
#include "gnumeric.hpp"

#ifdef __ORCUS_SPREADSHEET_MODEL
#include "document.hpp"
#include "sheet.hpp"
#include "sheet_rows.hpp"
#endif

#include <iostream>
#include <string>

#include <Python.h>

#define ORCUS_DEBUG_PYTHON 0
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))

using namespace std;

namespace orcus { namespace python {

namespace {

#if ORCUS_DEBUG_PYTHON
void print_args(PyObject* args)
{
    string args_str;
    PyObject* repr = PyObject_Repr(args);
    if (repr)
    {
        Py_INCREF(repr);
        args_str = PyBytes_AsString(repr);
        Py_DECREF(repr);
    }
    cout << args_str << "\n";
}
#endif

PyMethodDef orcus_methods[] =
{
    { "info", (PyCFunction)info, METH_NOARGS, "Print orcus module information." },

    { "detect_format", (PyCFunction)detect_format, METH_VARARGS | METH_KEYWORDS,
      "Detect the format type of a spreadsheet file." },

    { "_csv_read", (PyCFunction)csv_read, METH_VARARGS | METH_KEYWORDS,
      "Load specified csv file into a document model." },

    { "_ods_read", (PyCFunction)ods_read, METH_VARARGS | METH_KEYWORDS,
      "Load specified ods file into a document model." },

    { "_xlsx_read", (PyCFunction)xlsx_read, METH_VARARGS | METH_KEYWORDS,
      "Load specified xlsx file into a document model." },

    { "_xls_xml_read", (PyCFunction)xls_xml_read, METH_VARARGS | METH_KEYWORDS,
      "Load specified xls-xml file into a document model." },

    { "_gnumeric_read", (PyCFunction)gnumeric_read, METH_VARARGS | METH_KEYWORDS,
      "Load specified gnumeric file into a document model." },

    { nullptr, nullptr, 0, nullptr }
};

struct module_state
{
    PyObject* error;
};

int orcus_traverse(PyObject* m, visitproc visit, void* arg)
{
    Py_VISIT(GETSTATE(m)->error);
    return 0;
}

int orcus_clear(PyObject* m)
{
    Py_CLEAR(GETSTATE(m)->error);
    return 0;
}

}

struct PyModuleDef moduledef =
{
    PyModuleDef_HEAD_INIT,
    "_orcus",
    nullptr,
    sizeof(struct module_state),
    orcus_methods,
    nullptr,
    orcus_traverse,
    orcus_clear,
    nullptr
};

}}

extern "C" {

ORCUS_DLLPUBLIC PyObject* PyInit__orcus()
{
    PyObject* m = PyModule_Create(&orcus::python::moduledef);

#ifdef __ORCUS_SPREADSHEET_MODEL
    PyTypeObject* doc_type = orcus::python::get_document_type();
    if (!PyType_Ready(doc_type))
    {
        Py_INCREF(doc_type);
        if (PyModule_AddObject(m, "Document", reinterpret_cast<PyObject*>(doc_type)) < 0)
        {
            Py_DECREF(m);
            Py_DECREF(doc_type);
            return nullptr;
        }
    }

    PyTypeObject* sheet_type = orcus::python::get_sheet_type();
    if (!PyType_Ready(sheet_type))
    {
        Py_INCREF(sheet_type);
        if (PyModule_AddObject(m, "Sheet", reinterpret_cast<PyObject*>(sheet_type)) < 0)
        {
            Py_DECREF(m);
            Py_DECREF(sheet_type);
            return nullptr;
        }
    }

    PyTypeObject* sheet_rows_type = orcus::python::get_sheet_rows_type();
    if (!PyType_Ready(sheet_rows_type))
    {
        Py_INCREF(sheet_rows_type);
        if (PyModule_AddObject(m, "SheetRows", reinterpret_cast<PyObject*>(sheet_rows_type)) < 0)
        {
            Py_DECREF(m);
            Py_DECREF(sheet_rows_type);
            return nullptr;
        }
    }
#endif

    return m;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
