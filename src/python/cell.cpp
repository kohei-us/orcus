/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "cell.hpp"
#include "memory.hpp"
#include "global.hpp"
#include "formula_token.hpp"
#include "formula_tokens.hpp"
#include "orcus/spreadsheet/document.hpp"

#include <ixion/cell.hpp>
#include <ixion/formula_result.hpp>
#include <ixion/formula.hpp>
#include <ixion/formula_name_resolver.hpp>
#include <ixion/model_context.hpp>

#include <structmember.h>
#include <string>

namespace ss = orcus::spreadsheet;

namespace orcus { namespace python {

namespace {

/** non-python part of the object. */
struct cell_data
{
    const ss::document* doc = nullptr;
    const ixion::formula_cell* formula_cell = nullptr;
    ixion::abs_address_t origin;
};

/**
 * Python object for orcus.Cell.
 */
struct pyobj_cell
{
    PyObject_HEAD

    PyObject* type;
    PyObject* value;
    PyObject* formula;

    cell_data* data = nullptr;
};

void initialize_cell_members(pyobj_cell* self)
{
    Py_INCREF(Py_None);
    self->value = Py_None;

    Py_INCREF(Py_None);
    self->formula = Py_None;
}

PyObject* create_and_init_cell_object(const char* type_name)
{
    PyTypeObject* cell_type = get_cell_type();
    if (!cell_type)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get the cell type object.");
        return nullptr;
    }

    PyObject* obj = cell_type->tp_new(cell_type, nullptr, nullptr);
    if (!obj)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to instantiate a cell object.");
        return nullptr;
    }

    pyobj_cell* self = reinterpret_cast<pyobj_cell*>(obj);
    self->type = get_python_enum_value("CellType", type_name);
    initialize_cell_members(self);

    return obj;
}

void tp_dealloc(pyobj_cell* self)
{
    delete self->data;
    self->data = nullptr;

    Py_CLEAR(self->type);
    Py_CLEAR(self->value);
    Py_CLEAR(self->formula);

    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

PyObject* tp_new(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    pyobj_cell* self = (pyobj_cell*)type->tp_alloc(type, 0);
    self->data = new cell_data;
    return reinterpret_cast<PyObject*>(self);
}

int tp_init(pyobj_cell* self, PyObject* args, PyObject* kwargs)
{
    static const char* kwlist[] = { "type", nullptr };

    self->type = nullptr;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O", const_cast<char**>(kwlist), &self->type))
        return -1;

    if (!self->type)
        self->type = get_python_enum_value("CellType", "UNKNOWN");

    initialize_cell_members(self);
    return 0;
}

PyObject* cell_get_formula_tokens(PyObject* self, PyObject* args, PyObject* kwargs)
{
    pyobj_cell* obj = reinterpret_cast<pyobj_cell*>(self);
    cell_data& data = *obj->data;
    if (!data.formula_cell)
    {
        // This is not a formula cell.
        Py_INCREF(Py_None);
        return Py_None;
    }

    const ixion::formula_tokens_t& tokens = data.formula_cell->get_tokens()->get();
    return create_formula_tokens_iterator_object(*data.doc, data.origin, tokens);
}

PyMethodDef tp_methods[] =
{
    { "get_formula_tokens", (PyCFunction)cell_get_formula_tokens, METH_NOARGS, "Get a formula tokens iterator." },
    { nullptr }
};

PyMemberDef tp_members[] =
{
    { (char*)"type", T_OBJECT_EX, offsetof(pyobj_cell, type), READONLY, (char*)"cell type" },
    { (char*)"value", T_OBJECT_EX, offsetof(pyobj_cell, value), READONLY, (char*)"cell value" },
    { (char*)"formula", T_OBJECT_EX, offsetof(pyobj_cell, formula), READONLY, (char*)"formula string" },
    { nullptr }
};

PyTypeObject cell_type =
{
    PyVarObject_HEAD_INIT(nullptr, 0)
    "orcus.Cell",                             // tp_name
    sizeof(pyobj_cell),                       // tp_basicsize
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
    "orcus spreadsheet cell",                 // tp_doc
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

PyObject* create_cell_object_empty()
{
    PyObject* obj = create_and_init_cell_object("EMPTY");
    if (!obj)
        return nullptr;

    return obj;
}

PyObject* create_cell_object_boolean(bool v)
{
    PyObject* obj = create_and_init_cell_object("BOOLEAN");
    if (!obj)
        return nullptr;

    pyobj_cell* obj_data = reinterpret_cast<pyobj_cell*>(obj);

    if (v)
    {
        Py_INCREF(Py_True);
        obj_data->value = Py_True;
    }
    else
    {
        Py_INCREF(Py_False);
        obj_data->value = Py_False;
    }

    return obj;
}

PyObject* create_cell_object_string(const std::string* p)
{
    PyObject* obj = create_and_init_cell_object("STRING");
    if (!obj)
        return nullptr;

    pyobj_cell* obj_data = reinterpret_cast<pyobj_cell*>(obj);

    if (p)
    {
        obj_data->value = PyUnicode_FromStringAndSize(p->data(), p->size());
        if (!obj_data->value)
        {
            // The string contains invalid utf-8 sequence, and the function has
            // already set a python exception which needs to be cleared.
            PyErr_Clear();
            Py_XDECREF(obj);
            obj = create_and_init_cell_object("STRING_WITH_ERROR");
        }
    }
    else
    {
        Py_INCREF(Py_None);
        obj_data->value = Py_None;
    }

    return obj;
}

PyObject* create_cell_object_numeric(double v)
{
    PyObject* obj = create_and_init_cell_object("NUMERIC");
    if (!obj)
        return nullptr;

    pyobj_cell* obj_data = reinterpret_cast<pyobj_cell*>(obj);
    obj_data->value = PyFloat_FromDouble(v);

    return obj;
}

PyObject* create_cell_object_formula(
    const spreadsheet::document& doc, const ixion::abs_address_t& origin, const ixion::formula_cell* fc)
{
    if (!fc)
    {
        PyErr_SetString(PyExc_RuntimeError, "failed to find class orcus.CellType.");
        return nullptr;
    }

    const ixion::formula_tokens_t& tokens = fc->get_tokens()->get();
    bool is_error = !tokens.empty() && tokens[0]->get_opcode() == ixion::fop_error;

    PyObject* obj = create_and_init_cell_object(is_error ? "FORMULA_WITH_ERROR": "FORMULA");
    if (!obj)
        return nullptr;

    pyobj_cell* obj_data = reinterpret_cast<pyobj_cell*>(obj);
    obj_data->data->doc = &doc;
    obj_data->data->origin = origin;
    obj_data->data->formula_cell = fc;

    // Create formula expression string.
    auto* resolver = doc.get_formula_name_resolver(spreadsheet::formula_ref_context_t::global);
    const ixion::model_context& cxt = doc.get_model_context();
    std::string formula_s = ixion::print_formula_tokens(cxt, origin, *resolver, tokens);
    obj_data->formula = PyUnicode_FromStringAndSize(formula_s.data(), formula_s.size());

    ixion::formula_result res;

    try
    {
        res = fc->get_result_cache(
            ixion::formula_result_wait_policy_t::throw_exception);
    }
    catch (const std::exception&)
    {
        Py_INCREF(Py_None);
        obj_data->value = Py_None;
        return obj;
    }

    switch (res.get_type())
    {
        case ixion::formula_result::result_type::value:
        {
            obj_data->value = PyFloat_FromDouble(res.get_value());
            break;
        }
        case ixion::formula_result::result_type::string:
        {
            const std::string& s = res.get_string();
            obj_data->value = PyUnicode_FromStringAndSize(s.data(), s.size());
            break;
        }
        case ixion::formula_result::result_type::error:
        {
            ixion::formula_error_t fe = res.get_error();
            const char* fename = ixion::get_formula_error_name(fe);
            if (fename)
                obj_data->value = PyUnicode_FromString(fename);
            else
            {
                // This should not be hit, but just in case...
                Py_INCREF(Py_None);
                obj_data->value = Py_None;
            }
            break;
        }
        default:
        {
            // This should not be hit, but just in case...
            Py_INCREF(Py_None);
            obj_data->value = Py_None;
        }
    }

    return obj;
}

PyTypeObject* get_cell_type()
{
    return &cell_type;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
