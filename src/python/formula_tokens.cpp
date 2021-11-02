/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "formula_tokens.hpp"
#include "formula_token.hpp"
#include "orcus/spreadsheet/document.hpp"

#include <ixion/formula_tokens.hpp>
#include <structmember.h>

namespace ss = orcus::spreadsheet;

namespace orcus { namespace python {

namespace {

/** non-python part. */
struct formula_tokens_data
{
    const ss::document* doc;
    ixion::abs_address_t origin;
    const ixion::formula_tokens_t* tokens = nullptr;
    ixion::formula_tokens_t::const_iterator pos;
    ixion::formula_tokens_t::const_iterator end;
};

/** python object */
struct pyobj_formula_tokens
{
    PyObject_HEAD

    formula_tokens_data* data = nullptr;
};

inline pyobj_formula_tokens* t(PyObject* self)
{
    return reinterpret_cast<pyobj_formula_tokens*>(self);
}

void init_members(
    pyobj_formula_tokens* self, const ss::document& doc, const ixion::abs_address_t& origin, const ixion::formula_tokens_t& tokens)
{
    assert(self->data);
    self->data->doc = &doc;
    self->data->origin = origin;
    self->data->tokens = &tokens;
}

void tp_dealloc(pyobj_formula_tokens* self)
{
    delete self->data;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

int tp_init(pyobj_formula_tokens* /*self*/, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    return 0;
}

PyObject* tp_new(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    pyobj_formula_tokens* self = t(type->tp_alloc(type, 0));
    self->data = new formula_tokens_data;
    return reinterpret_cast<PyObject*>(self);
}

PyObject* tp_iter(PyObject* self)
{
    formula_tokens_data& data = *t(self)->data;
    data.pos = data.tokens->cbegin();
    data.end = data.tokens->cend();

    Py_INCREF(self);
    return self;
}

PyObject* tp_iternext(PyObject* self)
{
    formula_tokens_data& data = *t(self)->data;

    if (data.pos == data.end)
    {
        // No more elements.  Stop the iteration.
        PyErr_SetNone(PyExc_StopIteration);
        return nullptr;
    }

    PyObject* ft_obj = create_formula_token_object(*data.doc, data.origin, **data.pos);
    ++data.pos;
    return ft_obj;
}

Py_ssize_t sq_length(PyObject* self)
{
    formula_tokens_data& data = *t(self)->data;
    return data.tokens->size();
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

PyObject* create_formula_tokens_iterator_object(
    const ss::document& doc, const ixion::abs_address_t& origin, const ixion::formula_tokens_t& tokens)
{
    PyTypeObject* ft_type = get_formula_tokens_type();
    if (!ft_type)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get the formula tokens type object.");
        return nullptr;
    }

    PyObject* obj = ft_type->tp_new(ft_type, nullptr, nullptr);
    if (!obj)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to instantiate a formula tokens object.");
        return nullptr;
    }

    init_members(t(obj), doc, origin, tokens);

    return obj;
}

PyTypeObject* get_formula_tokens_type()
{
    return &formula_tokens_type;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
