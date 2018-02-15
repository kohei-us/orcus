/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xls_xml.hpp"

#ifdef __ORCUS_PYTHON_XLS_XML
#include "document.hpp"
#include "orcus/orcus_xls_xml.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/global.hpp"
#endif

namespace orcus { namespace python {

#ifdef __ORCUS_PYTHON_XLS_XML

PyObject* xls_xml_read(PyObject* /*module*/, PyObject* args, PyObject* kwargs)
{
    PyObject* obj_bytes = read_stream_object_from_args(args, kwargs);
    if (!obj_bytes)
        return nullptr;

    std::unique_ptr<spreadsheet::document> doc = orcus::make_unique<spreadsheet::document>();
    spreadsheet::import_factory fact(*doc);
    orcus_xls_xml app(&fact);

    return import_from_stream_into_document(obj_bytes, app, std::move(doc));
}

#else

PyObject* xls_xml_read(PyObject*, PyObject*, PyObject*)
{
    PyErr_SetString(PyExc_RuntimeError, "The xls-xml module is not enabled.");
    return nullptr;
}

#endif

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */