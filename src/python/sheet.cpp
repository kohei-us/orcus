/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sheet.hpp"
#include "sheet_rows.hpp"
#include "named_expression.hpp"
#include "named_expressions.hpp"

#include <orcus/spreadsheet/types.hpp>
#include <orcus/spreadsheet/sheet.hpp>
#include <orcus/spreadsheet/document.hpp>

#include <ixion/model_context.hpp>
#include <ixion/named_expressions_iterator.hpp>

#include <structmember.h>
#include <bytesobject.h>
#include <iostream>
#include <sstream>
#include <cstring>

namespace ss = orcus::spreadsheet;

namespace orcus { namespace python {

sheet_data::sheet_data() : m_doc(nullptr), m_sheet(nullptr) {}
sheet_data::~sheet_data() {}

namespace {

/**
 * Python object for orcus.Sheet.
 */
struct pyobj_sheet
{
    PyObject_HEAD

    PyObject* name;
    PyObject* sheet_size;
    PyObject* data_size;
    PyObject* named_expressions;

    sheet_data* data;
};

inline pyobj_sheet* t(PyObject* self)
{
    return reinterpret_cast<pyobj_sheet*>(self);
}

inline sheet_data* get_sheet_data(PyObject* self)
{
    return t(self)->data;
}

/**
 * Get the underlying C++ sheet class instance from the python object
 * representation.
 */
inline spreadsheet::sheet* get_core_sheet(PyObject* self)
{
    return get_sheet_data(self)->m_sheet;
}

void tp_dealloc(pyobj_sheet* self)
{
    delete self->data;

    Py_CLEAR(self->name);
    Py_CLEAR(self->sheet_size);
    Py_CLEAR(self->data_size);

    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

PyObject* tp_new(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    pyobj_sheet* self = (pyobj_sheet*)type->tp_alloc(type, 0);
    self->data = new sheet_data;
    return reinterpret_cast<PyObject*>(self);
}

int tp_init(pyobj_sheet* /*self*/, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    return 0;
}

PyObject* sheet_get_rows(PyObject* self, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    PyTypeObject* sr_type = get_sheet_rows_type();

    PyObject* rows = sr_type->tp_new(sr_type, nullptr, nullptr);
    if (!rows)
        return nullptr;

    sr_type->tp_init(rows, nullptr, nullptr);

    // Populate the sheet rows data.
    sheet_data* data = get_sheet_data(self);
    store_sheet_rows_data(rows, data->m_doc, data->m_sheet);

    return rows;
}

namespace {

format_t to_format_type_enum(PyObject* format)
{
    static const char* err_not_format_type = "An enum value of 'orcus.FormatType' was expected.";
    static const char* err_format_not_supported = "Unsupported format type.";

    // Check the type name.

    PyTypeObject* type = Py_TYPE(format);
    if (!type || strncmp(type->tp_name, "FormatType", 10u) != 0)
    {
        PyErr_SetString(PyExc_RuntimeError, err_not_format_type);
        return format_t::unknown;
    }

    // Now check the member name.

    PyObject* format_s = PyObject_GetAttrString(format, "name"); // new reference
    if (!format_s)
    {
        PyErr_SetString(PyExc_RuntimeError, err_not_format_type);
        return format_t::unknown;
    }

    // TODO : currently we only support csv format.  Change this code when we
    // add more format type(s) to support.

    const char* p = PyUnicode_AsUTF8(format_s);
    if (!p || strncmp(p, "CSV", 3u) != 0)
    {
        PyErr_SetString(PyExc_RuntimeError, err_format_not_supported);
        Py_DECREF(format_s);
        return format_t::unknown;
    }

    Py_DECREF(format_s);
    return format_t::csv;
}

bool sheet_write_as_csv(spreadsheet::sheet* sheet, PyObject* file)
{
    std::ostringstream os;
    sheet->dump_csv(os);
    std::string s = os.str();

    if (!s.empty())
    {
        PyObject* func_write = PyObject_GetAttrString(file, "write"); // new reference
        if (!func_write)
        {
            PyErr_SetString(PyExc_RuntimeError, "'write' function was expected, but not found.");
            return false;
        }

        // write the content as python's utf-8 string ('s').  Use 'y' to write
        // it as bytes (for future reference).
        PyObject_CallFunction(func_write, "s", s.data(), nullptr);
        Py_XDECREF(func_write);
    }

    return true;
}

}

PyObject* sheet_write(PyObject* self, PyObject* args, PyObject* kwargs)
{
    static const char* kwlist[] = { "file", "format", nullptr };

    PyObject* file = nullptr;
    PyObject* format = nullptr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO", const_cast<char**>(kwlist), &file, &format))
        return nullptr;

    format_t ft = to_format_type_enum(format);

    if (ft == format_t::unknown)
        return nullptr;

    spreadsheet::sheet* sheet = get_core_sheet(self);

    switch (ft)
    {
        case format_t::csv:
        {
            if (!sheet_write_as_csv(sheet, file))
                return nullptr;

            break;
        }
        default:
        {
            PyErr_SetString(PyExc_RuntimeError, "this format is not yet supported.");
            return nullptr;
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject* sheet_get_named_expressions(PyObject* self, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    const ss::document& doc = *t(self)->data->m_doc;
    ss::sheet_t si = t(self)->data->m_sheet->get_index();
    const ixion::model_context& cxt = doc.get_model_context();
    return create_named_expressions_object(si, doc, cxt.get_named_expressions_iterator(si));
}

PyMethodDef tp_methods[] =
{
    { "get_rows", (PyCFunction)sheet_get_rows, METH_VARARGS, "Get a sheet row iterator." },
    { "write", (PyCFunction)sheet_write, METH_VARARGS | METH_KEYWORDS, "Write sheet content to specified file object." },
    { "get_named_expressions", (PyCFunction)sheet_get_named_expressions, METH_NOARGS, "Get a named expressions iterator." },
    { nullptr }
};

PyMemberDef tp_members[] =
{
    { (char*)"name",       T_OBJECT_EX, offsetof(pyobj_sheet, name),       READONLY, (char*)"sheet name" },
    { (char*)"sheet_size", T_OBJECT_EX, offsetof(pyobj_sheet, sheet_size), READONLY, (char*)"sheet size" },
    { (char*)"data_size",  T_OBJECT_EX, offsetof(pyobj_sheet, data_size),  READONLY, (char*)"data size" },
    { nullptr }
};

PyTypeObject sheet_type =
{
    PyVarObject_HEAD_INIT(nullptr, 0)
    "orcus.Sheet",                            // tp_name
    sizeof(pyobj_sheet),                      // tp_basicsize
    0,                                        // tp_itemsize
    (destructor)tp_dealloc,                   // tp_dealloc
    0,                                        // tp_print
    0,                                        // tp_getattr
    0,                                        // tp_setattr
    0,                                        // tp_compare
    0,                                        // tp_repr
    0,                                        // tp_as_number
    0,                                        // tp_as_sequence
    0,                                        // tp_as_mapping
    0,                                        // tp_hash
    0,                                        // tp_call
    0,                                        // tp_str
    0,                                        // tp_getattro
    0,                                        // tp_setattro
    0,                                        // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
    "orcus sheet object",                     // tp_doc
    0,		                                  // tp_traverse
    0,		                                  // tp_clear
    0,		                                  // tp_richcompare
    0,		                                  // tp_weaklistoffset
    0,		                                  // tp_iter
    0,		                                  // tp_iternext
    tp_methods,                               // tp_methods
    tp_members,                               // tp_members
    0,                                        // tp_getset
    0,                                        // tp_base
    0,                                        // tp_dict
    0,                                        // tp_descr_get
    0,                                        // tp_descr_set
    0,                                        // tp_dictoffset
    (initproc)tp_init,                     // tp_init
    0,                                        // tp_alloc
    tp_new,                                // tp_new
};

}

sheet_data* get_sheet_data(PyObject* self)
{
    return reinterpret_cast<pyobj_sheet*>(self)->data;
}

PyTypeObject* get_sheet_type()
{
    return &sheet_type;
}

void store_sheet(
    PyObject* self, const spreadsheet::document* doc, spreadsheet::sheet* orcus_sheet)
{
    pyobj_sheet* pysheet = reinterpret_cast<pyobj_sheet*>(self);
    pysheet->data->m_doc = doc;
    pysheet->data->m_sheet = orcus_sheet;

    // Populate the python members.

    // Sheet name
    spreadsheet::sheet_t sid = orcus_sheet->get_index();
    std::string_view sheet_name = doc->get_sheet_name(sid);
    pysheet->name = PyUnicode_FromStringAndSize(sheet_name.data(), sheet_name.size());

    // Data size - size of the data area.
    ixion::abs_range_t range = orcus_sheet->get_data_range();
    if (range.valid())
    {
        pysheet->data_size = PyDict_New();
        PyDict_SetItemString(pysheet->data_size, "column", PyLong_FromLong(range.last.column+1));
        PyDict_SetItemString(pysheet->data_size, "row", PyLong_FromLong(range.last.row+1));
    }
    else
    {
        Py_INCREF(Py_None);
        pysheet->data_size = Py_None;
    }

    // Sheet size - size of the entire sheet.
    pysheet->sheet_size = PyDict_New();
    ss::range_size_t sheet_size = doc->get_sheet_size();
    PyDict_SetItemString(pysheet->sheet_size, "column", PyLong_FromLong(sheet_size.columns));
    PyDict_SetItemString(pysheet->sheet_size, "row", PyLong_FromLong(sheet_size.rows));
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
