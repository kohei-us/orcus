/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_PYTHON_DOCUMENT_HPP
#define INCLUDED_ORCUS_PYTHON_DOCUMENT_HPP

#include "orcus/spreadsheet/document.hpp"

#include <memory>
#include <Python.h>

namespace orcus { namespace python {

/** non-python part of the document object. */
struct document_data
{
    std::unique_ptr<spreadsheet::document> m_doc;

    ~document_data();
};

document_data* get_document_data(PyObject* self);

/**
 * Extract a python object representing the byte stream from the arguments
 * passed to the python orcus.<file format>.read() function.
 *
 * @param args positional argument object.
 * @param kwargs keyword argument object.
 *
 * @return object representing the bytes.
 */
PyObject* read_stream_object_from_args(PyObject* args, PyObject* kwargs);

/**
 * Import a document from a python object containing the byte stream, and
 * create a python object of class orcus.Document.
 *
 * @param obj_bytes python object containing the byte stream.
 * @param app filter instance to use to load the document.
 * @param doc orcus document instance which will be stored within the python
 *            document object.
 *
 * @return python document object.
 */
PyObject* import_from_stream_into_document(
    PyObject* obj_bytes, iface::import_filter& app, std::unique_ptr<spreadsheet::document>&& doc);

/**
 * Get the definition of the python class Document.
 */
PyTypeObject* get_document_type();

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
