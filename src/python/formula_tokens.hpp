/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_PYTHON_FORMULA_TOKENS_HPP
#define INCLUDED_ORCUS_PYTHON_FORMULA_TOKENS_HPP

#include <ixion/formula_tokens_fwd.hpp>
#include <Python.h>

namespace ixion {

struct abs_address_t;

}

namespace orcus {

namespace spreadsheet {

class document;

}

namespace python {

PyObject* create_formula_tokens_iterator_object(
    const spreadsheet::document& doc, const ixion::abs_address_t& origin, const ixion::formula_tokens_t& tokens);

PyTypeObject* get_formula_tokens_type();

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
