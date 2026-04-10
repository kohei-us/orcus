.. _json-mapping-basic:

Mapping JSON to spreadsheet
============================

This section demonstrates how to use the :cpp:class:`~orcus::orcus_json`
class to map the contents of a JSON document onto a spreadsheet.

Consider the following JSON document:

.. literalinclude:: ../../../doc_example/json/files/books.json
   :language: JSON

The document has a ``meta`` object at the top level containing three scalar
fields, followed by a ``books`` array where each element describes a book.

JSON path expressions
---------------------

Paths used by :cpp:class:`~orcus::orcus_json` use a JSONPath-inspired syntax,
but are not a strict subset of the JSONPath standard.  A path always starts
with ``$`` representing the document root.  Object keys are addressed with
bracket-and-quote notation (``['key']``), and array elements are addressed
with the wildcard notation (``[*]``), or the equivalent empty bracket notation (``[]``).

For example, in the document above:

* ``$['meta']['title']`` points to the string ``"My Book Collection"``
* ``$['books'][*]['author']`` points to the ``author`` field of every element
  in the ``books`` array

Cell links
----------

Start by including the necessary headers:

.. literalinclude:: ../../../doc_example/json/json_mapping_basic.cpp
   :language: C++
   :start-after: //!code-start: headers
   :end-before: //!code-end: headers

Load the input file into memory using :cpp:class:`~orcus::file_content`:

.. literalinclude:: ../../../doc_example/json/json_mapping_basic.cpp
   :language: C++
   :start-after: //!code-start: load-input
   :end-before: //!code-end: load-input
   :dedent: 4

Create a spreadsheet document and its associated import factory:

.. literalinclude:: ../../../doc_example/json/json_mapping_basic.cpp
   :language: C++
   :start-after: //!code-start: create-doc
   :end-before: //!code-end: create-doc
   :dedent: 4

Construct an :cpp:class:`~orcus::orcus_json` filter instance connected to the
import factory:

.. literalinclude:: ../../../doc_example/json/json_mapping_basic.cpp
   :language: C++
   :start-after: //!code-start: setup-orcus-json
   :end-before: //!code-end: setup-orcus-json
   :dedent: 4

Use :cpp:func:`~orcus::orcus_json::set_cell_link` to map individual JSON
values to specific cells:

.. literalinclude:: ../../../doc_example/json/json_mapping_basic.cpp
   :language: C++
   :start-after: //!code-start: cell-links
   :end-before: //!code-end: cell-links
   :dedent: 4

Each call takes a JSONPath expression identifying a node in the JSON document,
the name of the target sheet, and the row and column where the value should
be placed.

Now insert the target sheet and parse the input stream:

.. literalinclude:: ../../../doc_example/json/json_mapping_basic.cpp
   :language: C++
   :start-after: //!code-start: read-json
   :end-before: //!code-end: read-json
   :dedent: 4

Then retrieve the sheet and dump its content:

.. literalinclude:: ../../../doc_example/json/json_mapping_basic.cpp
   :language: C++
   :start-after: //!code-start: dump-content
   :end-before: //!code-end: dump-content
   :dedent: 4

This should produce the following output:

.. code-block:: none

   rows: 3  cols: 1
   +--------------------+
   | My Book Collection |
   +--------------------+
   | John Doe           |
   +--------------------+
   | 2025-01-15         |
   +--------------------+

Range mapping
-------------

While :cpp:func:`~orcus::orcus_json::set_cell_link` maps individual values to
individual cells, :cpp:class:`~orcus::orcus_json` also supports mapping an
array of objects to a range of rows — similar to a database table.  Use
:cpp:func:`~orcus::orcus_json::start_range` to begin defining a range,
:cpp:func:`~orcus::orcus_json::append_field_link` to map each JSON field to a
named column, :cpp:func:`~orcus::orcus_json::set_range_row_group` to identify
the array that determines row boundaries, and
:cpp:func:`~orcus::orcus_json::commit_range` to finalize the mapping:

.. literalinclude:: ../../../doc_example/json/json_mapping_range.cpp
   :language: C++
   :start-after: //!code-start: range
   :end-before: //!code-end: range
   :dedent: 4

The first three arguments to :cpp:func:`~orcus::orcus_json::start_range` are
the target sheet name and the row and column of the top-left corner of the
range.  The fourth argument is a boolean flag that, when ``true``, reserves
the first row for column headers.  Each
:cpp:func:`~orcus::orcus_json::append_field_link` call takes a JSONPath
expression and a column label.  The path ``$['books'][*]['title']`` selects
the ``title`` field from every element of the ``books`` array.  The call to
:cpp:func:`~orcus::orcus_json::set_range_row_group` with ``$['books']`` tells
orcus that each element of the ``books`` array represents one row in the
output.

Running this code produces the following output:

.. code-block:: none

   rows: 6  cols: 5
   +-------------------------------+------------------------+----------+---------------+---------+
   | Title                         | Author                 | Year     | Genre         | Rating  |
   +-------------------------------+------------------------+----------+---------------+---------+
   | Crime and Punishment          | Fyodor Dostoevsky      | 1866 [v] | Novel         | 4.8 [v] |
   +-------------------------------+------------------------+----------+---------------+---------+
   | The Great Gatsby              | F. Scott Fitzgerald    | 1925 [v] | Novel         | 4 [v]   |
   +-------------------------------+------------------------+----------+---------------+---------+
   | One Hundred Years of Solitude | Gabriel Garcia Marquez | 1967 [v] | Magic Realism | 4.7 [v] |
   +-------------------------------+------------------------+----------+---------------+---------+
   | To Kill a Mockingbird         | Harper Lee             | 1960 [v] | Novel         | 4.9 [v] |
   +-------------------------------+------------------------+----------+---------------+---------+
   | Brave New World               | Aldous Huxley          | 1932 [v] | Dystopian     | 4.2 [v] |
   +-------------------------------+------------------------+----------+---------------+---------+

Note that the ``[v]`` marker in the output indicates that the value is stored
as a numeric type rather than a string.

Cell links and range mappings can be freely mixed within the same session.
When combined, both become active when
:cpp:func:`~orcus::orcus_json::read_stream` is called, and orcus populates
all of them in a single pass over the document.  Multiple ranges can also be
defined in the same session by repeating the
:cpp:func:`~orcus::orcus_json::start_range` …
:cpp:func:`~orcus::orcus_json::commit_range` sequence before calling
:cpp:func:`~orcus::orcus_json::read_stream`.
