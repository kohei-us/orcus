/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sheet_rows.hpp"
#include "memory.hpp"
#include "cell.hpp"
#include "orcus/spreadsheet/sheet.hpp"
#include "orcus/spreadsheet/document.hpp"

namespace orcus { namespace python {

sheet_rows_data::sheet_rows_data() :
    m_doc(nullptr),
    m_sheet(nullptr),
    m_range(ixion::abs_range_t::invalid),
    m_current_row(-1) {}

sheet_rows_data::~sheet_rows_data() {}

namespace {

/**
 * Python object for orcus.SheetRows.
 */
struct pyobj_sheet_rows
{
    PyObject_HEAD

    sheet_rows_data* m_data;
};

void sheet_rows_dealloc(pyobj_sheet_rows* self)
{
    delete self->m_data;

    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

PyObject* sheet_rows_new(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    pyobj_sheet_rows* self = (pyobj_sheet_rows*)type->tp_alloc(type, 0);
    self->m_data = new sheet_rows_data;
    return reinterpret_cast<PyObject*>(self);
}

int sheet_rows_init(pyobj_sheet_rows* self, PyObject* /*args*/, PyObject* /*kwargs*/)
{
    return 0;
}

PyObject* sheet_rows_iter(PyObject* self)
{
    sheet_rows_data* data = reinterpret_cast<pyobj_sheet_rows*>(self)->m_data;
    data->m_current_row = 0;
    data->m_row_pos = data->m_sheet_range.row_begin();
    data->m_row_end = data->m_sheet_range.row_end();

    Py_INCREF(self);
    return self;
}

PyObject* sheet_rows_iternext(PyObject* self)
{
    sheet_rows_data* data = reinterpret_cast<pyobj_sheet_rows*>(self)->m_data;
    spreadsheet::sheet_range::const_row_iterator& row_pos = data->m_row_pos;
    const spreadsheet::sheet_range::const_row_iterator& row_end = data->m_row_end;

    if (row_pos == row_end)
    {
        // No more elements.  Stop the iteration.
        PyErr_SetNone(PyExc_StopIteration);
        return nullptr;
    }

    py_scoped_ref orcus_mod = PyImport_ImportModule("orcus");
    if (!orcus_mod)
    {
        PyErr_SetString(PyExc_RuntimeError, "failed to import orcus module.");
        return nullptr;
    }

    py_scoped_ref cls_celltype = PyObject_GetAttrString(orcus_mod.get(), "CellType");
    if (!cls_celltype)
    {
        PyErr_SetString(PyExc_RuntimeError, "failed to find class orcus.CellType.");
        return nullptr;
    }

    size_t row_position = row_pos->position;
    PyObject* pyobj_row = PyTuple_New(data->m_range.last.column+1);

    for (; row_pos != row_end && row_position == row_pos->position; ++row_pos)
    {
        size_t col_pos = row_pos->index;
        PyObject* obj = nullptr;

        switch (row_pos->type)
        {
            case ixion::element_type_empty:
            {
                obj = create_cell_object_empty();
                break;
            }
            case ixion::element_type_boolean:
            {
                bool v = row_pos->get<ixion::boolean_element_block>();
                obj = create_cell_object_boolean(v);
                break;
            }
            case ixion::element_type_string:
            {
                ixion::string_id_t sid = row_pos->get<ixion::string_element_block>();
                const std::string* ps = data->m_sheet_range.get_string(sid);
                obj = create_cell_object_string(ps);
                break;
            }
            case ixion::element_type_numeric:
            {
                double v = row_pos->get<ixion::numeric_element_block>();
                obj = create_cell_object_numeric(v);
                break;
            }
            case ixion::element_type_formula:
            {
                const ixion::formula_cell* fc = row_pos->get<ixion::formula_element_block>();
                ixion::abs_address_t pos(data->m_sheet->get_index(), row_pos->position, col_pos);
                obj = create_cell_object_formula(*data->m_doc, pos, fc);
                break;
            }
            default:
                ;
        }

        if (!obj)
            return nullptr;

        PyTuple_SetItem(pyobj_row, col_pos, obj);
    }

    return pyobj_row;
}

PyTypeObject sheet_rows_type =
{
    PyVarObject_HEAD_INIT(nullptr, 0)
    "orcus.SheetRows",                        // tp_name
    sizeof(pyobj_sheet_rows),                       // tp_basicsize
    0,                                        // tp_itemsize
    (destructor)sheet_rows_dealloc,           // tp_dealloc
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
    "orcus sheet rows iterator",              // tp_doc
    0,		                                  // tp_traverse
    0,		                                  // tp_clear
    0,		                                  // tp_richcompare
    0,		                                  // tp_weaklistoffset
    (getiterfunc)sheet_rows_iter,		      // tp_iter
    (iternextfunc)sheet_rows_iternext,        // tp_iternext
    0,                                        // tp_methods
    0,                                        // tp_members
    0,                                        // tp_getset
    0,                                        // tp_base
    0,                                        // tp_dict
    0,                                        // tp_descr_get
    0,                                        // tp_descr_set
    0,                                        // tp_dictoffset
    (initproc)sheet_rows_init,                // tp_init
    0,                                        // tp_alloc
    sheet_rows_new,                           // tp_new
};

}

PyTypeObject* get_sheet_rows_type()
{
    return &sheet_rows_type;
}

void store_sheet_rows_data(PyObject* self, const spreadsheet::document* orcus_doc, const spreadsheet::sheet* orcus_sheet)
{
    sheet_rows_data* data = reinterpret_cast<pyobj_sheet_rows*>(self)->m_data;
    data->m_doc = orcus_doc;
    data->m_sheet = orcus_sheet;
    data->m_range = orcus_sheet->get_data_range();

    const ixion::abs_range_t& range = data->m_range;
    if (range.valid())
    {
        data->m_sheet_range = orcus_sheet->get_sheet_range(
            0, 0, range.last.row, range.last.column);
    }
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
