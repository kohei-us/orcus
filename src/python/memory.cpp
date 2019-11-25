/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "memory.hpp"

namespace orcus { namespace python {

void pyobj_unique_deleter::operator ()(PyObject* p) const
{
    Py_XDECREF(p);
}

py_scoped_ref::py_scoped_ref() : m_pyobj(nullptr) {}
py_scoped_ref::py_scoped_ref(PyObject* p) : m_pyobj(p) {}

py_scoped_ref::~py_scoped_ref()
{
    if (m_pyobj)
        Py_DECREF(m_pyobj);
}

py_scoped_ref& py_scoped_ref::operator= (PyObject* p)
{
    if (m_pyobj)
        Py_DECREF(m_pyobj);
    m_pyobj = p;
    return *this;
}

PyObject* py_scoped_ref::get()
{
    return m_pyobj;
}

py_scoped_ref::operator bool() const
{
    return m_pyobj != nullptr;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
