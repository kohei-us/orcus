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
py_scoped_ref::py_scoped_ref(const py_scoped_ref& other) : m_pyobj(other.m_pyobj)
{
    if (m_pyobj)
        Py_INCREF(m_pyobj);
}

py_scoped_ref::py_scoped_ref(py_scoped_ref&& other) noexcept : m_pyobj(other.m_pyobj)
{
    other.m_pyobj = nullptr;
}

py_scoped_ref::~py_scoped_ref()
{
    if (m_pyobj)
        Py_DECREF(m_pyobj);
}

py_scoped_ref& py_scoped_ref::operator=(PyObject* p)
{
    if (m_pyobj)
        Py_DECREF(m_pyobj);
    m_pyobj = p;
    return *this;
}

py_scoped_ref& py_scoped_ref::operator=(const py_scoped_ref& other)
{
    if (this == &other)
        return *this;

    if (other.m_pyobj)
        Py_INCREF(other.m_pyobj);

    if (m_pyobj)
        Py_DECREF(m_pyobj);

    m_pyobj = other.m_pyobj;
    return *this;
}

py_scoped_ref& py_scoped_ref::operator=(py_scoped_ref&& other) noexcept
{
    if (this == &other)
        return *this;

    if (m_pyobj)
        Py_DECREF(m_pyobj);

    m_pyobj = other.m_pyobj;
    other.m_pyobj = nullptr;
    return *this;
}

PyObject* py_scoped_ref::get()
{
    return m_pyobj;
}

const PyObject* py_scoped_ref::get() const
{
    return m_pyobj;
}

py_scoped_ref::operator bool() const
{
    return m_pyobj != nullptr;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
