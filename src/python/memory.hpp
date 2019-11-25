/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_PYTHON_MEMORY_HPP
#define INCLUDED_ORCUS_PYTHON_MEMORY_HPP

#include <memory>
#include <Python.h>

namespace orcus { namespace python {

struct pyobj_unique_deleter
{
    void operator() (PyObject* p) const;
};

using py_unique_ptr = std::unique_ptr<PyObject, pyobj_unique_deleter>;

class py_scoped_ref
{
    PyObject* m_pyobj;
public:
    py_scoped_ref();
    py_scoped_ref(PyObject* p);
    ~py_scoped_ref();

    py_scoped_ref& operator= (PyObject* p);
    PyObject* get();
    operator bool() const;
};

}}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
