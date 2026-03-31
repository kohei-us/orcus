/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "csv.hpp"
#include "global.hpp"

#ifdef __ORCUS_PYTHON_CSV
#include "document.hpp"
#include "orcus/orcus_csv.hpp"
#include "orcus/config.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/factory.hpp"
#endif

#include <cstring>

namespace orcus { namespace python {

#ifdef __ORCUS_PYTHON_CSV

PyObject* csv_read(PyObject* /*module*/, PyObject* args, PyObject* kwargs)
{
    static const char* kwlist[] = { "stream", "split", "delimiters", "text_qualifier", nullptr };

    py_unique_ptr ret;
    PyObject* file = nullptr;

    // Get the default CSV configuration values except on delimiters.
    orcus::config conf(format_t::csv);
    config::csv_config csvconf = std::get<config::csv_config>(conf.data);
    int split = csvconf.split_to_multiple_sheets;
    const char* delimiters = nullptr;
    const char* text_qualifier = nullptr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|pss", const_cast<char**>(kwlist), &file, &split, &delimiters, &text_qualifier))
        return nullptr;

    if (!file)
    {
        PyErr_SetString(PyExc_RuntimeError, "Invalid file object has been passed.");
        return nullptr;
    }

    // Set CSV configuration values. Only delimiters are not set to default value.
    csvconf.split_to_multiple_sheets = (bool)split;
    if (delimiters)
        csvconf.delimiters = std::string(delimiters);

    if (text_qualifier)
    {
        // make sure it's a string of length 1
        if (std::strlen(text_qualifier) != 1u)
        {
            PyErr_SetString(PyExc_ValueError, "text_qualifier must be of length 1");
            return nullptr;
        }
        csvconf.text_qualifier = *text_qualifier;
    }

    conf.data = csvconf;

    PyObject* obj_str = nullptr;

    if (PyObject_HasAttrString(file, "read"))
    {
        PyObject* func_read = PyObject_GetAttrString(file, "read"); // new reference
        obj_str = PyObject_CallFunction(func_read, nullptr);
        Py_XDECREF(func_read);
    }

    if (!obj_str)
    {
        if (PyObject_TypeCheck(file, &PyUnicode_Type))
            obj_str = PyUnicode_FromObject(file); // new reference
    }

    if (!obj_str)
    {
        PyErr_SetString(PyExc_RuntimeError, "failed to extract bytes from this object.");
        return nullptr;
    }

    ret.reset(obj_str);

    if (!ret)
        return nullptr;

    try
    {
        spreadsheet::range_size_t ss{1048576, 16384};
        std::unique_ptr<spreadsheet::document> doc = std::make_unique<spreadsheet::document>(ss);
        spreadsheet::import_factory fact(*doc);
        orcus_csv app(&fact);

        app.set_config(conf);

        Py_ssize_t n = 0;
        const char* p = PyUnicode_AsUTF8AndSize(ret.get(), &n);
        app.read_stream({p, static_cast<std::string_view::size_type>(n)});

        return create_document(std::move(doc));
    }
    catch (const std::exception& e)
    {
        set_python_exception(PyExc_RuntimeError, e);
        return nullptr;
    }
}

#else

PyObject* csv_read(PyObject*, PyObject*, PyObject*)
{
    PyErr_SetString(PyExc_RuntimeError, "The csv module is not enabled.");
    return nullptr;
}

#endif

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
