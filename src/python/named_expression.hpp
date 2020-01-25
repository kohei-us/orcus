/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_NAMED_EXPRESSION_HPP
#define INCLUDED_NAMED_EXPRESSION_HPP

#include <Python.h>

namespace orcus { namespace python {

/** non-python part. */
struct named_exp_data
{
    named_exp_data();
    ~named_exp_data();
};

PyTypeObject* get_named_exp_type();

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
