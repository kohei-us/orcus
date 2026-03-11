.. _detect-format:

Detecting format of data stream
===============================

Orcus provides an API to detect the format of a data stream via the :cpp:any:`orcus::detect`
function.  This function parses the content of a stream given as ``std::string_view``
and reports the detected format type.  In this section we will walk through
how to use this function with some test input files from the project
repository.

First, let's include the necessary headers:

.. literalinclude:: ../../../doc_example/detect_1.cpp
   :language: C++
   :start-after: //!code-start: headers
   :end-before: //!code-end: headers

We will also create a namespace alias ``fs`` to reference the ``std::filesystem``
namespace for brevity.

Let's assume that there is an environment variable named ``TESTDIR`` that
points to the top-level ``test`` directory of the project repository:

.. literalinclude:: ../../../doc_example/detect_1.cpp
   :language: C++
   :start-after: //!code-start: getenv
   :end-before: //!code-end: getenv
   :dedent: 4

We will use this path throughout the examples.  Now, let's try to detect
a test input file that we know is of Open Document Spreadsheet (ODS) format.

.. literalinclude:: ../../../doc_example/detect_1.cpp
   :language: C++
   :start-after: //!code-start: ods
   :end-before: //!code-end: ods
   :dedent: 8

The first two lines load the input file and references expose its
content through the :cpp:class:`orcus::file_content` class.  This class
internally uses mmap to map the content of the loaded file into virtual
memory, and its :cpp:func:`~orcus::file_content::str()` method returns a
view of its content as ``std::string_view``.  This view can then be
passed to the :cpp:func:`~orcus::detect()` function.  The returned
value, which is of enum type :cpp:enum:`orcus::format_t`, can be printed
directly to ``stdout`` via ``std::cout``.  Running this code should
produce the following output:

.. code-block:: none

   format: ods

Let's try another input file.  This time it is an Excel 2007 file:

.. literalinclude:: ../../../doc_example/detect_1.cpp
   :language: C++
   :start-after: //!code-start: xlsx
   :end-before: //!code-end: xlsx
   :dedent: 8

The file is loaded and examined the same way before.  Running this code
should produce the following output:

.. code-block:: none

   format: xlsx

You can also detect a generic XML file, as in the following example:

.. literalinclude:: ../../../doc_example/detect_1.cpp
   :language: C++
   :start-after: //!code-start: xml
   :end-before: //!code-end: xml
   :dedent: 8

The input file used above is an XML file but does not correspond to any
specific XML-based file format.  It is simply a generic XML document.
Running this code should produce the following output:

.. code-block:: none

   format: xml

Similarly, using a generic JSON file as the input to detect:

.. literalinclude:: ../../../doc_example/detect_1.cpp
   :language: C++
   :start-after: //!code-start: json
   :end-before: //!code-end: json
   :dedent: 8

should produce the following output:

.. code-block:: none

   format: json

You can also use a variant of :cpp:func:`~orcus::detect()` that checks
whether an input stream is of a specified format.  Let's take a look at
the following example:

.. literalinclude:: ../../../doc_example/detect_1.cpp
   :language: C++
   :start-after: //!code-start: is-ods
   :end-before: //!code-end: is-ods
   :dedent: 8

Here, we are passing the content of what we know to be an ODS file to
the :cpp:func:`~orcus::detect()` function and asking it to report 1) whether
it is an ODS file then 2) whether it is an XLSX file.

The expected output is:

.. code-block:: none

   ods? 1
   xlsx? 0

Next, we are going to use an Excel 2003 XML file as the input and ask the
:cpp:func:`~orcus::detect()` function whether it is:

* an Excel 2003 XML file,
* an XML file, and
* a JSON file.

.. literalinclude:: ../../../doc_example/detect_1.cpp
   :language: C++
   :start-after: //!code-start: is-xls-xml
   :end-before: //!code-end: is-xls-xml
   :dedent: 8

Here is the output from this code:

.. code-block:: none

   xls-xml? 1
   xml? 1
   json? 0

The ``xls-xml`` alias is what orcus uses to reference the Excel 2003 XML
format (often referred to as the `SpreadsheetML <https://en.wikipedia.org/wiki/SpreadsheetML>`_
format).  Since this is an XML-based format, asking whether it is an XML
format should also yield true.  But since it is clearly not a JSON format,
the last inquiry should rightly yield false.
