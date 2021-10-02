/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "document.hpp"
#include "sheet.hpp"
#include "global.hpp"
#include "named_expression.hpp"
#include "named_expressions.hpp"
#include "pstring.hpp"

#include <ixion/model_context.hpp>
#include <ixion/named_expressions_iterator.hpp>
#include <structmember.h>
#include <object.h>
#include <sstream>

using namespace std;
namespace ss = orcus::spreadsheet;

namespace orcus { namespace python {

document_data::~document_data()
{
}

namespace {

/**
 * Python object for orcus.Document.
 */
struct pyobj_document
{
    PyObject_HEAD

    PyObject* sheets; // tuple of sheet objects.

    document_data* data;
};

inline pyobj_document* t(PyObject* self)
{
    return reinterpret_cast<pyobj_document*>(self);
}

void tp_dealloc(pyobj_document* self)
{
    delete self->data;

    // Destroy all sheet objects.
    Py_ssize_t n = PyTuple_Size(self->sheets);
    for (Py_ssize_t i = 0; i < n; ++i)
    {
        PyObject* o = PyTuple_GetItem(self->sheets, i);
        Py_CLEAR(o);
    }
    Py_CLEAR(self->sheets);  // and the tuple containing the sheets.

    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

PyObject* tp_new(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    pyobj_document* self = t(type->tp_alloc(type, 0));
    self->data = new document_data;
    return reinterpret_cast<PyObject*>(self);
}

int tp_init(pyobj_document* self, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    return 0;
}

PyObject* doc_get_named_expressions(PyObject* self, PyObject* args, PyObject* kwargs)
{
    const ss::document& doc = *t(self)->data->m_doc;
    const ixion::model_context& cxt = doc.get_model_context();
    return create_named_expressions_object(-1, doc, cxt.get_named_expressions_iterator());
}

PyMethodDef tp_methods[] =
{
    { "get_named_expressions", (PyCFunction)doc_get_named_expressions, METH_NOARGS, "Get a named expressions iterator." },
    { nullptr }
};

PyMemberDef tp_members[] =
{
    { (char*)"sheets", T_OBJECT_EX, offsetof(pyobj_document, sheets), READONLY, (char*)"sheet objects" },
    { nullptr }
};

PyTypeObject document_type =
{
    PyVarObject_HEAD_INIT(nullptr, 0)
    "orcus.Document",                         // tp_name
    sizeof(pyobj_document),                   // tp_basicsize
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
    "orcus document object",                  // tp_doc
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
    (initproc)tp_init,                        // tp_init
    0,                                        // tp_alloc
    tp_new,                                   // tp_new
};

bool import_from_stream_object(iface::import_filter& app, PyObject* obj_bytes)
{
    const char* p = PyBytes_AsString(obj_bytes);
    if (!p)
        return false;

    size_t n = PyBytes_Size(obj_bytes);

    app.read_stream({p, n});

    return true;
}

PyObject* create_document_object()
{
    PyTypeObject* type = get_document_type();

    PyObject* obj_doc = create_object_from_type(type);
    if (!obj_doc)
        return nullptr;

    type->tp_init(obj_doc, nullptr, nullptr);

    return obj_doc;
}

void store_document(PyObject* self, std::unique_ptr<spreadsheet::document>&& doc)
{
    if (!self)
        return;

    pyobj_document* pydoc = reinterpret_cast<pyobj_document*>(self);
    document_data* pydoc_data = pydoc->data;
    pydoc_data->m_doc = std::move(doc);

    PyTypeObject* sheet_type = get_sheet_type();
    if (!sheet_type)
        return;

    // Create a tuple of sheet objects and store it with the pydoc instance.
    size_t sheet_size = pydoc_data->m_doc->get_sheet_count();

    pydoc->sheets = PyTuple_New(sheet_size);

    for (size_t i = 0; i < sheet_size; ++i)
    {
        spreadsheet::sheet* sheet = pydoc_data->m_doc->get_sheet(i);
        if (!sheet)
            continue;

        PyObject* pysheet = sheet_type->tp_new(sheet_type, nullptr, nullptr);
        if (!pysheet)
            continue;

        sheet_type->tp_init(pysheet, nullptr, nullptr);

        Py_INCREF(pysheet);
        PyTuple_SetItem(pydoc->sheets, i, pysheet);

        store_sheet(pysheet, pydoc_data->m_doc.get(), sheet);
    }
}

} // anonoymous namespace

PyTypeObject* get_document_type()
{
    return &document_type;
}

document_data* get_document_data(PyObject* self)
{
    return reinterpret_cast<pyobj_document*>(self)->data;
}

stream_with_formulas read_stream_and_formula_params_from_args(PyObject* args, PyObject* kwargs)
{
    static const char* kwlist[] = { "stream", "recalc", "error_policy", nullptr };

    stream_with_formulas ret;
    PyObject* file = nullptr;
    int recalc_formula_cells = 0;
    const char* error_policy_s = nullptr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|ps", const_cast<char**>(kwlist), &file, &recalc_formula_cells, &error_policy_s))
        return ret;

    if (!file)
    {
        PyErr_SetString(PyExc_RuntimeError, "Invalid file object has been passed.");
        return ret;
    }

    PyObject* obj_bytes = nullptr;

    if (PyObject_HasAttrString(file, "read"))
    {
        PyObject* func_read = PyObject_GetAttrString(file, "read"); // new reference
        obj_bytes = PyObject_CallFunction(func_read, nullptr);
        Py_XDECREF(func_read);
    }

    if (!obj_bytes)
    {
        if (PyObject_TypeCheck(file, &PyBytes_Type))
            obj_bytes = PyBytes_FromObject(file);
    }

    if (!obj_bytes)
    {
        PyErr_SetString(PyExc_RuntimeError, "failed to extract bytes from this object.");
        return ret;
    }

    if (error_policy_s)
    {
        ss::formula_error_policy_t error_policy = ss::to_formula_error_policy(error_policy_s);
        if (error_policy == ss::formula_error_policy_t::unknown)
        {
            std::ostringstream os;
            os << "invalid error policy value: '" << error_policy_s << "'. The value must be either 'fail' or 'skip'.";
            PyErr_SetString(PyExc_RuntimeError, os.str().data());
            return ret;
        }

        ret.error_policy = error_policy;
    }

    ret.stream.reset(obj_bytes);
    ret.recalc_formula_cells = recalc_formula_cells != 0;

    return ret;
}

py_unique_ptr read_stream_from_args(PyObject* args, PyObject* kwargs)
{
    static const char* kwlist[] = { "stream", nullptr };

    py_unique_ptr obj_bytes;
    PyObject* file = nullptr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", const_cast<char**>(kwlist), &file))
        return obj_bytes;

    if (!file)
    {
        PyErr_SetString(PyExc_RuntimeError, "Invalid file object has been passed.");
        return obj_bytes;
    }

    if (PyObject_HasAttrString(file, "read"))
    {
        PyObject* func_read = PyObject_GetAttrString(file, "read"); // new reference
        obj_bytes.reset(PyObject_CallFunction(func_read, nullptr));
        Py_XDECREF(func_read);
    }

    if (!obj_bytes)
    {
        if (PyObject_TypeCheck(file, &PyBytes_Type))
            obj_bytes.reset(PyBytes_FromObject(file));
    }

    if (!obj_bytes)
    {
        PyErr_SetString(PyExc_RuntimeError, "failed to extract bytes from this object.");
        return obj_bytes;
    }

    return obj_bytes;
}

PyObject* import_from_stream_into_document(
    PyObject* obj_bytes, iface::import_filter& app, std::unique_ptr<spreadsheet::document>&& doc)
{
    if (!import_from_stream_object(app, obj_bytes))
        return nullptr;

    return create_document(std::move(doc));
}

PyObject* create_document(std::unique_ptr<spreadsheet::document>&& doc)
{
    PyObject* obj_doc = create_document_object();
    if (!obj_doc)
        return nullptr;

    store_document(obj_doc, std::move(doc));
    return obj_doc;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
