/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xlsx.hpp"

#ifdef __ORCUS_PYTHON_XLSX
#include "document.hpp"
#include "orcus/orcus_xlsx.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/global.hpp"
#endif

namespace orcus { namespace python {

#ifdef __ORCUS_PYTHON_XLSX

PyObject* xlsx_read(PyObject* /*module*/, PyObject* args, PyObject* kwargs)
{
    PyObject* obj_bytes = read_stream_object_from_args(args, kwargs);
    if (!obj_bytes)
        return nullptr;

    std::unique_ptr<spreadsheet::document> doc = orcus::make_unique<spreadsheet::document>();
    spreadsheet::import_factory fact(*doc);
    orcus_xlsx app(&fact);

    import_from_stream_object(app, obj_bytes);
    PyObject* obj_doc = create_document_object();
    store_document(obj_doc, std::move(doc));
    return obj_doc;
}

#else

PyObject* xlsx_read(PyObject*, PyObject*, PyObject*)
{
    PyErr_SetString(PyExc_RuntimeError, "The xlsx module is not enabled.");
    return nullptr;
}

#endif

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
