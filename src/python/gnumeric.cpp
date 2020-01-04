/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gnumeric.hpp"
#include "global.hpp"

#ifdef __ORCUS_PYTHON_GNUMERIC
#include "document.hpp"
#include "orcus/orcus_gnumeric.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/global.hpp"
#endif

namespace orcus { namespace python {

#ifdef __ORCUS_PYTHON_GNUMERIC

PyObject* gnumeric_read(PyObject* /*module*/, PyObject* args, PyObject* kwargs)
{
    stream_data data = read_stream_object_from_args(args, kwargs);
    if (!data.stream)
        return nullptr;

    try
    {
        std::unique_ptr<spreadsheet::document> doc = orcus::make_unique<spreadsheet::document>();
        spreadsheet::import_factory fact(*doc);
        fact.set_recalc_formula_cells(data.recalc_formula_cells);
        orcus_gnumeric app(&fact);

        return import_from_stream_into_document(data.stream.get(), app, std::move(doc));
    }
    catch (const std::exception& e)
    {
        set_python_exception(PyExc_RuntimeError, e);
        return nullptr;
    }
}

#else

PyObject* gnumeric_read(PyObject*, PyObject*, PyObject*)
{
    PyErr_SetString(PyExc_RuntimeError, "The gnumeric module is not enabled.");
    return nullptr;
}

#endif

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
