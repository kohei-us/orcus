/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "named_expression.hpp"

#include <structmember.h>

namespace orcus { namespace python {

named_exp_data::named_exp_data() {}
named_exp_data::~named_exp_data() {}

namespace {

/**
 * Python object for orcus.NamedExpression.
 */
struct pyobj_named_exp
{
    PyObject_HEAD

    PyObject* formula;
    PyObject* formula_tokens;

    named_exp_data* data;
};

void init_members(pyobj_named_exp* self)
{
    Py_INCREF(Py_None);
    self->formula = Py_None;

    Py_INCREF(Py_None);
    self->formula_tokens = Py_None;
}

void tp_dealloc(pyobj_named_exp* self)
{
    delete self->data;

    Py_CLEAR(self->formula);
    Py_CLEAR(self->formula_tokens);

    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

int tp_init(pyobj_named_exp* self, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    init_members(self);
    return 0;
}

PyObject* tp_new(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    pyobj_named_exp* self = (pyobj_named_exp*)type->tp_alloc(type, 0);
    self->data = new named_exp_data;
    return reinterpret_cast<PyObject*>(self);
}

PyMemberDef tp_members[] =
{
    { (char*)"formula", T_OBJECT_EX, offsetof(pyobj_named_exp, formula), READONLY, (char*)"formula string" },
    { (char*)"formula_tokens", T_OBJECT_EX, offsetof(pyobj_named_exp, formula_tokens), READONLY, (char*)"tuple of individual formula token strings" },
    { nullptr }
};

PyTypeObject named_exp_type =
{
    PyVarObject_HEAD_INIT(nullptr, 0)
    "orcus.NamedExpression",                  // tp_name
    sizeof(pyobj_named_exp),                  // tp_basicsize
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
    "orcus spreadsheet named expression",     // tp_doc
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

PyTypeObject* get_named_exp_type()
{
    return &named_exp_type;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
