/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "named_expression.hpp"
#include "formula_token.hpp"
#include "formula_tokens.hpp"
#include "orcus/spreadsheet/document.hpp"

#include <ixion/formula.hpp>
#include <ixion/model_context.hpp>
#include <ixion/named_expressions_iterator.hpp>
#include <ixion/formula_name_resolver.hpp>
#include <structmember.h>

namespace ss = orcus::spreadsheet;

namespace orcus { namespace python {

namespace {

/** non-python part of the object. */
struct named_exp_data
{
    const ss::document* doc = nullptr;
    const ixion::formula_tokens_t* tokens = nullptr;
    ixion::abs_address_t origin;
};

/**
 * Python object for orcus.NamedExpression.
 */
struct pyobj_named_exp
{
    PyObject_HEAD

    PyObject* origin;
    PyObject* formula;

    named_exp_data* data;
};

inline pyobj_named_exp* t(PyObject* self)
{
    return reinterpret_cast<pyobj_named_exp*>(self);
}

void init_members(pyobj_named_exp* self)
{
    Py_INCREF(Py_None);
    self->origin = Py_None;

    Py_INCREF(Py_None);
    self->formula = Py_None;
}

void tp_dealloc(pyobj_named_exp* self)
{
    delete self->data;
    self->data = nullptr;

    Py_CLEAR(self->origin);
    Py_CLEAR(self->formula);

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

PyObject* ne_get_formula_tokens(PyObject* self, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    named_exp_data& data = *t(self)->data;
    if (!data.tokens)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    return create_formula_tokens_iterator_object(*data.doc, data.origin, *data.tokens);
}

PyMethodDef tp_methods[] =
{
    { "get_formula_tokens", (PyCFunction)ne_get_formula_tokens, METH_NOARGS, "Get a formula tokens iterator." },
    { nullptr }
};

PyMemberDef tp_members[] =
{
    { (char*)"origin", T_OBJECT_EX, offsetof(pyobj_named_exp, origin), READONLY, (char*)"anchoring cell for the named expression" },
    { (char*)"formula", T_OBJECT_EX, offsetof(pyobj_named_exp, formula), READONLY, (char*)"formula string" },
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

PyObject* create_named_exp_object(const spreadsheet::document& doc, const ixion::named_expression_t* exp)
{
    PyTypeObject* named_exp_type = get_named_exp_type();
    if (!named_exp_type)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get the named expression type object.");
        return nullptr;
    }

    PyObject* obj = named_exp_type->tp_new(named_exp_type, nullptr, nullptr);
    if (!obj)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to instantiate a named expression object.");
        return nullptr;
    }

    pyobj_named_exp* self = reinterpret_cast<pyobj_named_exp*>(obj);
    init_members(self);

    if (exp)
    {
        named_exp_data& data = *self->data;
        data.doc = &doc;
        data.origin = exp->origin;
        data.tokens = &exp->tokens;

        const ixion::model_context& cxt = doc.get_model_context();
        auto* resolver = doc.get_formula_name_resolver(spreadsheet::formula_ref_context_t::global);

        // Create base
        std::string origin_s = resolver->get_name(exp->origin, ixion::abs_address_t(), true);
        self->origin = PyUnicode_FromStringAndSize(origin_s.data(), origin_s.size());

        // Create formula expression string.
        std::string formula_s = ixion::print_formula_tokens(cxt, exp->origin, *resolver, exp->tokens);
        self->formula = PyUnicode_FromStringAndSize(formula_s.data(), formula_s.size());
    }

    return obj;
}

PyObject* create_named_exp_dict(const ss::document& doc, ixion::named_expressions_iterator iter)
{
    PyObject* dict = PyDict_New();
    for (; iter.has(); iter.next())
    {
        auto ne = iter.get();
        PyObject* name = PyUnicode_FromStringAndSize(ne.name->data(), ne.name->size());
        PyObject* tokens = create_named_exp_object(doc, ne.expression);
        PyDict_SetItem(dict, name, tokens);
    }

    return dict;
}

PyTypeObject* get_named_exp_type()
{
    return &named_exp_type;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
