
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

For each document type that orcus supports, there is a top-level filter class
that serves as an entry point for loading the content of a document you wish
to load.  You don't pass your document to this filter directly; instead, you
wrap your document with what we call an "import factory", then pass this
factory instance to the filter.  This import factory is required to implement
necessary interfaces that the filter calls in order to pass data to the
document.  If you use orcus' own :cpp:class:`~orcus::spreadsheet::document`
class as the document store, then you can use the
:cpp:class:`~orcus::spreadsheet::import_factory` class provided in the library
which already implements all necessary interfaces.  If you need to use your
own document store, then you'll need to implement your own import factory
class that implements the required interfaces, and pass that instance to the
filter instead.


Other document types
--------------------

The orcus library also includes support for other, non-spreadsheet-like
document types.  The following sections delve more into the support for those
types of documents.

.. toctree::
   :maxdepth: 1

   json.rst

