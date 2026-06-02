/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <Python.h>

namespace orcus { namespace python {

struct pyobj_unique_deleter
{
    void operator() (PyObject* p) const;
};

using py_unique_ptr = std::unique_ptr<PyObject, pyobj_unique_deleter>;

/**
 * RAII that owns one PyObject reference.
 *
 * When the CPython API doc says:
 *
 * <li><em>new reference</em> (e.g. PyTuple_New, tp_new): assign to a new
 * instance of py_scoped_ref directly.</li>
 * <li><em>borrowed reference</em> (e.g. PyTuple_GetItem): don't wrap it
 * in py_scoped_ref</li>
 * <li><em>steals</em> (e.g. PyTuple_SetItem): call release() to surrender
 *     ownership; prefer to call set_tuple_item_new().</li>
 * <li>non-stealing store that increases ref count (e.g. PyDict_SetItem,
 * PySet_Add): pass get(); prefer to call set_dict_item_new() or
 * add_to_set_new().</li>
 */
class py_scoped_ref
{
    PyObject* m_pyobj;
public:
    py_scoped_ref();
    py_scoped_ref(PyObject* p);
    py_scoped_ref(const py_scoped_ref& other);
    py_scoped_ref(py_scoped_ref&& other) noexcept;
    ~py_scoped_ref();

    py_scoped_ref& operator=(PyObject* p);
    py_scoped_ref& operator=(const py_scoped_ref& other);
    py_scoped_ref& operator=(py_scoped_ref&& other) noexcept;

    PyObject* get();
    const PyObject* get() const;
    operator bool() const;

    [[nodiscard]] PyObject* release() noexcept;
    void reset(PyObject* p = nullptr) noexcept;
};

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
