
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


Other document types
--------------------

The orcus library also includes support for other, non-spreadsheet-like
document types.  The following sections delve more into the support for those
types of documents.

.. toctree::
   :maxdepth: 1

   json.rst

