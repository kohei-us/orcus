/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "formula_tokens.hpp"
#include "orcus/spreadsheet/document.hpp"

#include <structmember.h>

namespace ss = orcus::spreadsheet;

namespace orcus { namespace python {

namespace {

/** non-python part. */
struct formula_tokens_data
{
};

/** python object */
struct pyobj_formula_tokens
{
    PyObject_HEAD

    formula_tokens_data* data;
};

void tp_dealloc(pyobj_formula_tokens* self)
{
    delete self->data;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

int tp_init(pyobj_formula_tokens* self, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    return 0;
}

PyObject* tp_new(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    pyobj_formula_tokens* self = (pyobj_formula_tokens*)type->tp_alloc(type, 0);
    self->data = new formula_tokens_data;
    return reinterpret_cast<PyObject*>(self);
}

PyObject* tp_iter(PyObject* self)
{
    Py_INCREF(self);
    return self;
}

PyObject* tp_iternext(PyObject* self)
{
    // No more elements.  Stop the iteration.
    PyErr_SetNone(PyExc_StopIteration);
    return nullptr;
}

PyMethodDef tp_methods[] =
{
    { nullptr }
};

PyMemberDef tp_members[] =
{
    { nullptr }
};

PyTypeObject formula_tokens_type =
{
    PyVarObject_HEAD_INIT(nullptr, 0)
    "orcus.FormulaTokens",                    // tp_name
    sizeof(pyobj_formula_tokens),             // tp_basicsize
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
    "orcus spreadsheet formula tokens",       // tp_doc
    0,		                                  // tp_traverse
    0,		                                  // tp_clear
    0,		                                  // tp_richcompare
    0,		                                  // tp_weaklistoffset
    tp_iter,		                          // tp_iter
    tp_iternext,                              // tp_iternext
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

} // anonymous namespace

PyTypeObject* get_formula_tokens_type()
{
    return &formula_tokens_type;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
