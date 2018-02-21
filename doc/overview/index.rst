
.. highlight:: cpp

Overview
========

The primary goal of the orcus library is to provide methods to import the
contents of documents stored in various spreadsheet or spreadsheet-like
formats.  It also provides several low-level parsers that can be used
independently of the spreadsheet-related features if so desired.

The library is physically split into three parts: 1) the parser part that
includes the aforementioned low-level parsers, 2) the filter part that
includes higher level import filters that internally use the parsers, and 3)
the spreadsheet document model part that includes the document model suitable
for storing spreadsheet-like document contents.  As such, if you need to just
use the parser part of the library, you need to only link against the
``liborcus-parser`` library file.  If you need to use the import filter part,
you link againt both the ``liborcus-parser`` and the ``liborcus`` libraries.
Likewise, if you need to use the spreadsheet document model part, you need to
link against the aforementioned two plus the ``liborcus-spreadsheet-model``
library.  Also note that the spreadsheet document model part has additional
dependency on the `ixion library <https://gitlab.com/ixion/ixion>`_, for
handling formula re-calculations on document load.


Loading spreadsheet documents
-----------------------------

The orcus library's primary aim is to provide a method to import the contents
of documents stored in various spreadsheet, or spreadsheet-like formats.  It
supports two primary use cases.  The first use case is where the client
program does not have its own document model, but needs to import data from a
spreadsheet-like document file and access its content without implementing its
own document store from scratch.  In this particular use case, you can simply
use the :cpp:class:`~orcus::spreadsheet::document` class to get it populated,
and access its content through its API afterward.

The second use case, which is a bit more advanced, is where the client program
already has its own internal document model, and needs to use orcus
to populate its document model.  In this particular use case, you can
implement your own set of classes that support necessary interfaces, and pass
that to the orcus import filter.

For each document type that orcus supports, there is a top-level import filter
class that serves as an entry point for loading the content of a document you
wish to load.  You don't pass your document to this filter directly; instead,
you wrap your document with what we call an **import factory**, then pass this
factory instance to the loader.  This import factory is then required to
implement necessary interfaces that the filter class uses in order for it
to pass data to the document as the file is getting parsed.

When using orcus's own document model, you can simply use orcus's own import
factory implementation to wrap its document.  When using your own document
model, on the other hand, you'll need to implement your own set of interface
classes to wrap your document with.

The following sections describe how to load a spreadsheet document by using 1)
orcus's own spreadsheet document class, and 2) a user-defined custom docuemnt
class.

.. toctree::
   :maxdepth: 1

   doc-orcus.rst
   doc-user.rst


Loading hierarchical documents
------------------------------

The orcus library also includes support for hierarchical document types such
as JSON and YAML.  The following sections delve more into the support for
these types of documents.

.. toctree::
   :maxdepth: 1

   json.rst
   yaml.rst
