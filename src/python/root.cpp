/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "root.hpp"
#include "document.hpp"

#include "orcus/format_detection.hpp"

#include "orcus/info.hpp"

#include <iostream>
#include <sstream>

using namespace std;

namespace orcus { namespace python {

PyObject* info(PyObject*, PyObject*, PyObject*)
{
    cout << "orcus version: "
        << orcus::get_version_major() << '.'
        << orcus::get_version_minor() << '.'
        << orcus::get_version_micro() << endl;

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject* detect_format(PyObject* /*module*/, PyObject* args, PyObject* kwargs)
{
    PyObject* obj_bytes = read_stream_object_from_args(args, kwargs);
    if (!obj_bytes)
        return nullptr;

    const char* p = PyBytes_AS_STRING(obj_bytes);
    size_t n = PyBytes_Size(obj_bytes);

    try
    {
        format_t ft = orcus::detect(reinterpret_cast<const unsigned char*>(p), n);
        std::ostringstream os;
        os << ft;
        std::string s = os.str();

        return PyUnicode_FromStringAndSize(s.data(), s.size());
    }
    catch (const std::exception&)
    {
        PyErr_SetString(PyExc_ValueError, "failed to perform deep detection on this file.");
        return nullptr;
    }
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

