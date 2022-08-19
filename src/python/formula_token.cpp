/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "formula_token.hpp"
#include "global.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/global.hpp"

#include <ixion/formula.hpp>
#include <ixion/formula_name_resolver.hpp>
#include <ixion/model_context.hpp>
#include <structmember.h>

namespace ss = orcus::spreadsheet;

namespace orcus { namespace python {

namespace {

/** non-python part of the object's internal. */
struct data_formula_token
{
    std::string repr;
};

/**
 * Python object for orcus.NamedExpression.
 */
struct pyobj_formula_token
{
    PyObject_HEAD

    PyObject* type;
    PyObject* op;

    data_formula_token* data;
};

const char* to_formula_token_type(ixion::fopcode_t op)
{
    switch (op)
    {
        case ixion::fop_single_ref:
        case ixion::fop_range_ref:
        case ixion::fop_table_ref:
            return "REFERENCE";
        case ixion::fop_named_expression:
            return "NAME";
        case ixion::fop_function:
            return "FUNCTION";
        case ixion::fop_string:
        case ixion::fop_value:
            return "VALUE";
        case ixion::fop_plus:
        case ixion::fop_minus:
        case ixion::fop_divide:
        case ixion::fop_multiply:
        case ixion::fop_exponent:
        case ixion::fop_concat:
        case ixion::fop_equal:
        case ixion::fop_not_equal:
        case ixion::fop_less:
        case ixion::fop_greater:
        case ixion::fop_less_equal:
        case ixion::fop_greater_equal:
        case ixion::fop_open:
        case ixion::fop_close:
        case ixion::fop_sep:
            return "OPERATOR";
        case ixion::fop_error:
            return "ERROR";
        case ixion::fop_unknown:
        default:
            ;
    }

    return "UNKNOWN";
}

const char* to_formula_token_op(ixion::fopcode_t op)
{
    const char* names[] = {
        "UNKNOWN",
        "SINGLE_REF",
        "RANGE_REF",
        "TABLE_REF",
        "NAMED_EXPRESSION",
        "STRING",
        "VALUE",
        "FUNCTION",
        "PLUS",
        "MINUS",
        "DIVIDE",
        "MULTIPLY",
        "EXPONENT",
        "CONCAT",
        "EQUAL",
        "NOT_EQUAL",
        "LESS",
        "GREATER",
        "LESS_EQUAL",
        "GREATER_EQUAL",
        "OPEN",
        "CLOSE",
        "SEP",
        "ERROR",
    };

    auto n_names = std::size(names);
    return op < n_names ? names[op] : names[0];
}

void init_members(pyobj_formula_token* self)
{
    Py_INCREF(Py_None);
    self->type = Py_None;
    Py_INCREF(Py_None);
    self->op = Py_None;
}

PyObject* create_and_init_formula_token_object(ixion::fopcode_t op, std::string repr)
{
    PyTypeObject* ft_type = get_formula_token_type();
    if (!ft_type)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get the formula token type object.");
        return nullptr;
    }

    PyObject* obj = ft_type->tp_new(ft_type, nullptr, nullptr);
    if (!obj)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to instantiate a formula token object.");
        return nullptr;
    }

    pyobj_formula_token* self = reinterpret_cast<pyobj_formula_token*>(obj);
    init_members(self);
    self->type = get_python_enum_value("FormulaTokenType", to_formula_token_type(op));
    self->op = get_python_enum_value("FormulaTokenOp", to_formula_token_op(op));
    self->data->repr = std::move(repr);

    return obj;
}

void tp_dealloc(pyobj_formula_token* self)
{
    delete self->data;
    self->data = nullptr;

    Py_CLEAR(self->op);
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
    self->data = new data_formula_token;
    return reinterpret_cast<PyObject*>(self);
}

PyObject* tp_repr(pyobj_formula_token* self)
{
    return PyUnicode_FromStringAndSize(self->data->repr.data(), self->data->repr.size());
}

PyMemberDef tp_members[] =
{
    { (char*)"type", T_OBJECT_EX, offsetof(pyobj_formula_token, type), READONLY, (char*)"formula token type" },
    { (char*)"op", T_OBJECT_EX, offsetof(pyobj_formula_token, op), READONLY, (char*)"formula token operator" },
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
    (reprfunc)tp_repr,                        // tp_repr
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

PyObject* create_formula_token_object(const ss::document& doc, const ixion::abs_address_t& pos, const ixion::formula_token& token)
{
    const ixion::model_context& cxt = doc.get_model_context();
    auto* resolver = doc.get_formula_name_resolver(ss::formula_ref_context_t::global);
    assert(resolver);
    std::string ft_s = ixion::print_formula_token(cxt, pos, *resolver, token);

    PyObject* obj = create_and_init_formula_token_object(token.get_opcode(), std::move(ft_s));
    if (!obj)
        return nullptr;

    return obj;
}

PyTypeObject* get_formula_token_type()
{
    return &formula_token_type;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
