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

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
