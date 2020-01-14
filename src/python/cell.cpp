/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "cell.hpp"

namespace orcus { namespace python {

cell_data::cell_data() {}
cell_data::~cell_data() {}

namespace {

/**
 * Python object for orcus.Cell.
 */
struct pyobj_cell
{
    PyObject_HEAD

    cell_data* m_data;
};

void cell_dealloc(pyobj_cell* self)
{
    delete self->m_data;

    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

PyObject* cell_new(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    pyobj_cell* self = (pyobj_cell*)type->tp_alloc(type, 0);
    self->m_data = new cell_data;
    return reinterpret_cast<PyObject*>(self);
}

int cell_init(pyobj_cell* self, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    return 0;
}

PyTypeObject cell_type =
{
    PyVarObject_HEAD_INIT(nullptr, 0)
    "orcus.Cell",                             // tp_name
    sizeof(pyobj_cell),                       // tp_basicsize
    0,                                        // tp_itemsize
    (destructor)cell_dealloc,                 // tp_dealloc
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
    "orcus spreadsheet cell",                 // tp_doc
    0,		                                  // tp_traverse
    0,		                                  // tp_clear
    0,		                                  // tp_richcompare
    0,		                                  // tp_weaklistoffset
    0,		                                  // tp_iter
    0,                                        // tp_iternext
    0,                                        // tp_methods
    0,                                        // tp_members
    0,                                        // tp_getset
    0,                                        // tp_base
    0,                                        // tp_dict
    0,                                        // tp_descr_get
    0,                                        // tp_descr_set
    0,                                        // tp_dictoffset
    (initproc)cell_init,                      // tp_init
    0,                                        // tp_alloc
    cell_new,                                 // tp_new
};

}

PyTypeObject* get_cell_type()
{
    return &cell_type;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
