
.. highlight:: cpp

Overview
========

The primary goal of the orcus library is to provide methods to import the
contents of documents stored in various spreadsheet or spreadsheet-like
formats.  It also provides several low-level parsers that can be used
independently of the spreadsheet-related features.

TODO: continue on...


Loading spreadsheet documents
-----------------------------

The orcus library's primary aim is to provide a method to import the contents
of documents stored in various spreadsheet, or spreadsheet-like formats.  It
supports two primary use cases.  One use case is where the client program
already has its own internal document model, and you wish to use orcus
to populate its document model.  In this particular use case, you can
implement your own set of classes that support necessary interfaces, and pass
that to the orcus import filter.

Another use case is where the client program does not have its own document
model, and wishes to import data from a spreadsheet-like document file and
access its content.  In this particular use case, you can simply use
the :cpp:class:`~orcus::spreadsheet::document` class to get it populated, and
access its content through its API afterward.

For each document type that orcus supports, there is a top-level import filter
class that serves as an entry point for loading the content of a document you
wish to load.  You don't pass your document to this filter directly; instead,
you wrap your document with what we call an *import factory*, then pass this
factory instance to the loader This import factory is then required to
implement necessary interfaces that the filter class expects in order for it
to pass data to the document as the file is getting parsed.

The following sections describe how to load a spreadsheet document by using 1)
orcus's own spreadsheet document class, and 2) a user-defined custom docuemnt
class.

.. toctree::
   :maxdepth: 1

   doc-orcus.rst
   doc-user.rst


Non-spreadsheet document types
------------------------------

The orcus library also includes support for other, non-spreadsheet-like
document types.  The following sections delve more into the support for these
types of documents.

.. toctree::
   :maxdepth: 1

   json.rst
   yaml.rst
