/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "named_expressions.hpp"
#include "named_expression.hpp"
#include "global.hpp"
#include "orcus/spreadsheet/document.hpp"

#include <ixion/formula.hpp>
#include <ixion/model_context.hpp>
#include <ixion/named_expressions_iterator.hpp>
#include <ixion/formula_name_resolver.hpp>
#include <structmember.h>

namespace ss = orcus::spreadsheet;

namespace orcus { namespace python {

namespace {

/** non-python part. */
struct named_exps_data
{
    ss::sheet_t origin_sheet = -1; // -1 for global, >=0 for sheet local.
    const ss::document* doc = nullptr;
    ixion::named_expressions_iterator src; // original iterator to copy from.
    ixion::named_expressions_iterator iter;
};

/** python object. */
struct pyobj_named_exps
{
    PyObject_HEAD

    named_exps_data* data;
};

inline pyobj_named_exps* t(PyObject* self)
{
    return reinterpret_cast<pyobj_named_exps*>(self);
}

void tp_dealloc(pyobj_named_exps* self)
{
    delete self->data;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

int tp_init(pyobj_named_exps* self, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    return 0;
}

PyObject* tp_new(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    pyobj_named_exps* self = t(type->tp_alloc(type, 0));
    self->data = new named_exps_data;
    return reinterpret_cast<PyObject*>(self);
}

PyObject* tp_iter(PyObject* self)
{
    named_exps_data& data = *t(self)->data;
    data.iter = data.src;

    Py_INCREF(self);
    return self;
}

PyObject* tp_iternext(PyObject* self)
{
    named_exps_data& data = *t(self)->data;
    auto& iter = data.iter;

    if (!iter.has())
    {
        PyErr_SetNone(PyExc_StopIteration);
        return nullptr;
    }

    ixion::named_expressions_iterator::named_expression item = iter.get();
    iter.next();

    PyObject* name = PyUnicode_FromStringAndSize(item.name->data(), item.name->size());
    if (!name)
        return nullptr;

    PyObject* ne = create_named_exp_object(data.origin_sheet, *data.doc, item.expression);
    if (!ne)
        return nullptr;

    PyObject* tup = PyTuple_New(2);
    PyTuple_SET_ITEM(tup, 0, name);
    PyTuple_SET_ITEM(tup, 1, ne);

    return tup;
}

Py_ssize_t sq_length(PyObject* self)
{
    named_exps_data& data = *t(self)->data;
    return data.src.size();
}

PySequenceMethods tp_as_sequence =
{
    sq_length, // lenfunc sq_length
    0,         // binaryfunc sq_concat
    0,         // ssizeargfunc sq_repeat
    0,         // ssizeargfunc sq_item
    0,         // void *was_sq_slice
    0,         // ssizeobjargproc sq_ass_item
    0,         // void *was_sq_ass_slice
    0,         // objobjproc sq_contains
    0,         // binaryfunc sq_inplace_concat
    0,         // ssizeargfunc sq_inplace_repeat
};

PyMethodDef tp_methods[] =
{
    { nullptr }
};

PyMemberDef tp_members[] =
{
    { nullptr }
};

PyTypeObject named_exps_type =
{
    PyVarObject_HEAD_INIT(nullptr, 0)
    "orcus.NamedExpressions",                 // tp_name
    sizeof(pyobj_named_exps),                 // tp_basicsize
    0,                                        // tp_itemsize
    (destructor)tp_dealloc,                   // tp_dealloc
    0,                                        // tp_print
    0,                                        // tp_getattr
    0,                                        // tp_setattr
    0,                                        // tp_compare
    0,                                        // tp_repr
    0,                                        // tp_as_number
    &tp_as_sequence,                          // tp_as_sequence
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

PyObject* create_named_expressions_object(
    spreadsheet::sheet_t origin_sheet, const spreadsheet::document& doc, ixion::named_expressions_iterator iter)
{
    PyTypeObject* type = get_named_exps_type();

    PyObject* obj = create_object_from_type(type);
    if (!obj)
        return nullptr;

    type->tp_init(obj, nullptr, nullptr);

    named_exps_data& data = *t(obj)->data;
    data.src = iter;
    data.origin_sheet = origin_sheet;
    data.doc = &doc;

    return obj;
}

PyTypeObject* get_named_exps_type()
{
    return &named_exps_type;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
