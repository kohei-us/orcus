/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "global.hpp"

#include <sstream>

namespace orcus { namespace python {

void set_python_exception(PyObject* type, const std::exception& e)
{
    std::ostringstream os;
    os << "C++ exception caught: " << e.what();
    PyErr_SetString(PyExc_RuntimeError, os.str().data());
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
