/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "formula_token.hpp"

#include <structmember.h>

namespace orcus { namespace python {

namespace {

/**
 * Python object for orcus.NamedExpression.
 */
struct pyobj_formula_token
{
    PyObject_HEAD

    PyObject* type;
};

void init_members(pyobj_formula_token* self)
{
    Py_INCREF(Py_None);
    self->type = Py_None;
}

void tp_dealloc(pyobj_formula_token* self)
{
    Py_CLEAR(self->type);

    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

int tp_init(pyobj_formula_token* self, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    init_members(self);
    return 0;
}

PyObject* tp_new(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    pyobj_formula_token* self = (pyobj_formula_token*)type->tp_alloc(type, 0);
    return reinterpret_cast<PyObject*>(self);
}

PyMemberDef tp_members[] =
{
    { (char*)"type", T_OBJECT_EX, offsetof(pyobj_formula_token, type), READONLY, (char*)"formula token type" },
    { nullptr }
};

PyTypeObject formula_token_type =
{
    PyVarObject_HEAD_INIT(nullptr, 0)
    "orcus.FormulaToken",                     // tp_name
    sizeof(pyobj_formula_token),              // tp_basicsize
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
    "orcus spreadsheet formula token",        // tp_doc
    0,		                                  // tp_traverse
    0,		                                  // tp_clear
    0,		                                  // tp_richcompare
    0,		                                  // tp_weaklistoffset
    0,		                                  // tp_iter
    0,                                        // tp_iternext
    0,                                        // tp_methods
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

PyTypeObject* get_formula_token_type()
{
    return &formula_token_type;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
