
.. highlight:: cpp

Overview
========

The orcus library is designed to be used by a program that needs a way to
import the contents of documents stored in various spreadsheet, or
spreadsheet-like formats.  It supports two primary use cases.  One use case is
where the client program already has its own internal document model, and you
wish to use this library to populate its document model.  In this use case,
you can implement your own set of classes that supports necessary interfaces,
and that to the orcus filter instance.

Another use case is where the client program does not have its own document
model, but needs to import data from a spreadsheet document file and access
its content in some way.  In this use case, you can simply instantiate the
:cpp:class:`~orcus::spreadsheet::document` class provided by this library, get
it populated, and access its content through its API.

For each document type that orcus supports, there is a top-level loader class
that serves as an entry point for loading the content of a document you wish
to load.  You don't pass your document to this filter directly; instead, you
wrap your document with what we call an *import factory*, then pass this
factory instance to the loader  This import factory is required to implement
necessary interfaces that the loader calls in order for it to pass data to the
document as it parses the file.

If you have your own document store, then you need to implement your own
import factory class that implements the required interfaces then pass that
factory instance to the loader.

If you want to use orcus' :cpp:class:`~orcus::spreadsheet::document` as your
document store instead, then you can use the
:cpp:class:`~orcus::spreadsheet::import_factory` class that orcus provides
which already implements all necessary interfaces.  The example code shown
below illustrates how to do this::

    #include <orcus/spreadsheet/document.hpp>
    #include <orcus/spreadsheet/factory.hpp>
    #include <orcus/orcus_ods.hpp>

    #include <ixion/model_context.hpp>
    #include <iostream>

    using namespace orcus;

    int main()
    {
        // Instantiate a document, and wrap it with a factory.
        spreadsheet::document doc;
        spreadsheet::import_factory factory(doc);

        // Pass the factory to the document loader, and read the content from a file
        // to populate the document.
        orcus_ods loader(&factory);
        loader.read_file("/path/to/document.ods");

        // Now that the document is fully populated, access its content.
        const ixion::model_context& model = doc.get_model_context();

        // Read the header row and print its content.

        ixion::abs_address_t pos(0, 0, 0); // Set the cell position to A1.
        ixion::string_id_t str_id = model.get_string_identifier(pos);

        const std::string* s = model.get_string(str_id);
        assert(s);
        std::cout << "A1: " << *s << std::endl;

        pos.column = 1; // Move to B1
        str_id = model.get_string_identifier(pos);
        s = model.get_string(str_id);
        assert(s);
        std::cout << "B1: " << *s << std::endl;

        pos.column = 2; // Move to C1
        str_id = model.get_string_identifier(pos);
        s = model.get_string(str_id);
        assert(s);
        std::cout << "C1: " << *s << std::endl;

        return EXIT_SUCCESS;
    }

This example code loads a file saved in the Open Document Spreadsheet format.
It consists of the following content on its first sheet.

.. figure:: /_static/images/overview/doc-content.png

Let's walk through this code step by step.  First, we need to instantiate the
document store.  Here we are using the concrete :cpp:class:`~orcus::spreadsheet::document`
class available in orcus.  Then immediately pass this document to the
:cpp:class:`~orcus::spreadsheet::import_factory` instance also from orcus::

    // Instantiate a document, and wrap it with a factory.
    spreadsheet::document doc;
    spreadsheet::import_factory factory(doc);

The next step is to create the loader instance and pass the factory to it::

    // Pass the factory to the document loader, and read the content from a file
    // to populate the document.
    orcus_ods loader(&factory);

In this example we are using the :cpp:class:`~orcus::orcus_ods` class because
the document we are loading is in the Open Document Spreadsheet format.  Once
the loader is constructed, we'll simply load the file by calling its
:cpp:func:`~orcus::orcus_ods::read_file` method and passing the path to the
file as its argument::

    loader.read_file("/path/to/document.ods");

Once this call returns, the document has been fully populated.  What the rest
of the code does is access the content of the first row of the first sheet of
the document.  First, you need to get a reference to the internal cell value
store that we call *model context*::

    const ixion::model_context& model = doc.get_model_context();

Since the content of cell A1 is a string, to get the value you need to first
get the ID of the string::

    ixion::abs_address_t pos(0, 0, 0); // Set the cell position to A1.
    ixion::string_id_t str_id = model.get_string_identifier(pos);

Once you have the ID of the string, you can pass that to the model to get the
actual string value and print it to the standard output::

    const std::string* s = model.get_string(str_id);
    assert(s);
    std::cout << "A1: " << *s << std::endl;

Here we do assume that the string value exists for the given ID.  In case you
pass a string ID value to the :cpp:func:`get_string` method and there isn't a string
value associated with it, you'll get a null pointer instead.

The reason you need to take this 2-step process to get a string value is
because all the string values stored in the cells are pooled at the document
model level, and the cells themselves only store the ID values.

You may also have noticed that the types surrounding the :cpp:class:`ixion::model_context`
class are all in the :cpp:any:`ixion` namespace.  It is because orcus' own
:cpp:class:`~orcus::spreadsheet::document` class uses the formula engine from
the `ixion library <https://gitlab.com/ixion/ixion>`_ in order to calculate
the results of the formula cells inside the document, and the formula engine
requires all cell values to be stored in the :cpp:class:`ixion::model_context`
instance.

.. note:: The :cpp:class:`~orcus::spreadsheet::document` class in orcus uses
   the formula engine from the `ixion library <https://gitlab.com/ixion/ixion>`_
   to calculate the results of the formula cells stored in the document.


Other document types
--------------------

The orcus library also includes support for other, non-spreadsheet-like
document types.  The following sections delve more into the support for those
types of documents.

.. toctree::
   :maxdepth: 1

   json.rst

