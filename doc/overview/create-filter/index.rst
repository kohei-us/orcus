
Creating an import filter by type
=================================

Typical process of loading a document and populating an in-memory document
model involves the following steps:

1. create a document model instance,
2. wrap it in a factory instance that implements the :cpp:class:`~orcus::spreadsheet::iface::import_factory` interface,
3. create an import filter instance and pass the factory instance to it,
   and
4. load the file using the filter.

You can create an import filter instance in one of two ways.  The first
is to use the :cpp:func:`orcus::create_filter()` function and pass the
desired format type, while the second is to instantiate a filter class
directly, such as :cpp:class:`orcus::orcus_ods` for loading an ODS
document.

First, we are going to illustrate how to use the :cpp:func:`orcus::create_filter()`
function.  Let's include the necessary headers first:

.. literalinclude:: ../../../doc_example/create_filter.cpp
   :language: C++
   :start-after: //!code-start: header
   :end-before: //!code-end: header

We also define the namespace alias ``fs`` for brevity.

As in the :ref:`previous example <detect-format>`, we read an
environment variable named ``TESTDIR`` that points to the top-level ``test``
directory of this project repository:

.. literalinclude:: ../../../doc_example/create_filter.cpp
   :language: C++
   :start-after: //!code-start: testdir
   :end-before: //!code-end: testdir
   :dedent: 4

and construct the path to a test file, storing it in ``filepath``:

.. literalinclude:: ../../../doc_example/create_filter.cpp
   :language: C++
   :start-after: //!code-start: filepath
   :end-before: //!code-end: filepath
   :dedent: 4

Next step is to create a document model instance and wrap it in a factory
instance:

.. literalinclude:: ../../../doc_example/create_filter.cpp
   :language: C++
   :start-after: //!code-start: doc-and-factory
   :end-before: //!code-end: doc-and-factory
   :dedent: 4

This creates a :cpp:class:`~orcus::spreadsheet::document` instance
and wraps it in a matching :cpp:class:`~orcus::spreadsheet::import_factory`
instance.

The final step is to create an import filter instance to load the
document:

.. literalinclude:: ../../../doc_example/create_filter.cpp
   :language: C++
   :start-after: //!code-start: create-filter
   :end-before: //!code-end: create-filter
   :dedent: 4

Here, :cpp:enumerator:`orcus::format_t::ods` specifies the filter type,
and ``&factory`` passes the factory by pointer to
:cpp:func:`~orcus::create_filter()`.

Once created, call :cpp:func:`~orcus::iface::import_filter::read_file()`
on the returned filter to load the document.

In comparison, if you want to create a filter instance by specifying its
class directly, here is how to do it:

.. literalinclude:: ../../../doc_example/create_ods.cpp
   :language: C++
   :start-after: //!code-start: direct
   :end-before: //!code-end: direct
   :dedent: 4

Since this example loads an ODS document, we instantiate :cpp:class:`orcus::orcus_ods`
directly and call :cpp:func:`~orcus::orcus_ods::read_file()` to load it.

Orcus provides the following concrete filter types, each corresponding
to a :cpp:enum:`orcus::format_t` enumerator:

* :cpp:class:`orcus::orcus_ods`
* :cpp:class:`orcus::orcus_xlsx`
* :cpp:class:`orcus::orcus_gnumeric`
* :cpp:class:`orcus::orcus_xls_xml`
* :cpp:class:`orcus::orcus_csv`
* :cpp:class:`orcus::orcus_parquet`
* :cpp:class:`orcus::orcus_json`
* :cpp:class:`orcus::orcus_xml`
