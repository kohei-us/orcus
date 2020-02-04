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
#include "cell.hpp"
#include "named_expression.hpp"
#include "named_expressions.hpp"
#include "formula_token.hpp"
#include "formula_tokens.hpp"
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

bool add_type_to_module(PyObject* m, PyTypeObject* typeobj, const char* type_name)
{
    if (PyType_Ready(typeobj))
        return false;

    Py_INCREF(typeobj);
    if (PyModule_AddObject(m, type_name, reinterpret_cast<PyObject*>(typeobj)) < 0)
    {
        Py_DECREF(m);
        Py_DECREF(typeobj);
        return false;
    }

    return true;
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
    if (!orcus::python::add_type_to_module(m, orcus::python::get_document_type(), "Document"))
        return nullptr;

    if (!orcus::python::add_type_to_module(m, orcus::python::get_sheet_type(), "Sheet"))
        return nullptr;

    if (!orcus::python::add_type_to_module(m, orcus::python::get_sheet_rows_type(), "SheetRows"))
        return nullptr;

    if (!orcus::python::add_type_to_module(m, orcus::python::get_cell_type(), "Cell"))
        return nullptr;

    if (!orcus::python::add_type_to_module(m, orcus::python::get_named_exp_type(), "NamedExpression"))
        return nullptr;

    if (!orcus::python::add_type_to_module(m, orcus::python::get_named_exps_type(), "NamedExpressions"))
        return nullptr;

    if (!orcus::python::add_type_to_module(m, orcus::python::get_formula_token_type(), "FormulaToken"))
        return nullptr;

    if (!orcus::python::add_type_to_module(m, orcus::python::get_formula_tokens_type(), "FormulaTokens"))
        return nullptr;
#endif

    return m;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
