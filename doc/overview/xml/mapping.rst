.. _xml-mapping-basic:

Mapping XML to spreadsheet
==========================

This section demonstrates how to use the :cpp:class:`~orcus::orcus_xml`
class to map the contents of an XML document onto a spreadsheet.

Consider the following XML document:

.. literalinclude:: ../../../doc_example/xml/files/cities.xml
   :language: XML

This document contains a ``<header>`` element with metadata, followed by
a series of ``<city>`` elements each describing a major city.  We will
first focus on extracting the header metadata into a sheet.

Start by including the necessary headers:

.. literalinclude:: ../../../doc_example/xml/xml_mapping_basic.cpp
   :language: C++
   :start-after: //!code-start: headers
   :end-before: //!code-end: headers

Next, load the input file into memory using
:cpp:class:`~orcus::file_content`:

.. literalinclude:: ../../../doc_example/xml/xml_mapping_basic.cpp
   :language: C++
   :start-after: //!code-start: load-input
   :end-before: //!code-end: load-input
   :dedent: 4

``INPUTDIR`` is a constant that stores a path to the directory where the
input file is located.

Now, create a spreadsheet document and its associated import factory.
The :cpp:class:`~orcus::spreadsheet::range_size_t` value defines the
maximum number of rows and columns the document will support:

.. literalinclude:: ../../../doc_example/xml/xml_mapping_basic.cpp
   :language: C++
   :start-after: //!code-start: create-doc
   :end-before: //!code-end: create-doc
   :dedent: 4

Here, we are specifying the sheet size to be 200 rows and 10 columns.

Create an :cpp:class:`~orcus::xmlns_repository` to manage XML namespace
identifiers for the session, and use it to construct an
:cpp:class:`~orcus::orcus_xml` filter instance connected to the import
factory:

.. literalinclude:: ../../../doc_example/xml/xml_mapping_basic.cpp
   :language: C++
   :start-after: //!code-start: setup-orcus-xml
   :end-before: //!code-end: setup-orcus-xml
   :dedent: 4

Use :cpp:func:`~orcus::orcus_xml::set_cell_link()` to define individual
cell mapping rules:

.. literalinclude:: ../../../doc_example/xml/xml_mapping_basic.cpp
   :language: C++
   :start-after: //!code-start: cell-links
   :end-before: //!code-end: cell-links
   :dedent: 4

Each call takes an XPath expression identifying a node in the XML tree,
the name of the target sheet, and the row and column position where the
value should be placed.  Here we map the header metadata into the first
three rows of the sheet.

Note that the ``@`` prefix is used to reference an XML attribute, as
opposed to a child element.  The ``date-generated`` attribute of the
``<header>`` element is therefore addressed as
``/cities/header/@date-generated``.

Now that the mapping rules have been defined, insert the target sheet by
calling :cpp:func:`~orcus::orcus_xml::append_sheet()` and parse the
input stream:

.. literalinclude:: ../../../doc_example/xml/xml_mapping_basic.cpp
   :language: C++
   :start-after: //!code-start: read-xml
   :end-before: //!code-end: read-xml
   :dedent: 4

Finally, retrieve the first sheet from the document and dump its content
to standard output:

.. literalinclude:: ../../../doc_example/xml/xml_mapping_basic.cpp
   :language: C++
   :start-after: //!code-start: dump-content
   :end-before: //!code-end: dump-content
   :dedent: 4

Since :cpp:func:`~orcus::spreadsheet::document::get_sheet()` may return
a null pointer if the referenced sheet doesn't exist, you should check
the returned value to make sure it's not null.  Calling
:cpp:func:`~orcus::spreadsheet::sheet::dump_flat()` dumps the content of
the sheet to ``std::cout`` in an ASCII-art box-style format.

This code should produce the following output:

.. code-block:: none

   rows: 3  cols: 1
   +--------------------------------+
   | 2026-03-23                     |
   +--------------------------------+
   | Basic Facts About Major Cities |
   +--------------------------------+
   | Public knowledge               |
   +--------------------------------+

While :cpp:func:`~orcus::orcus_xml::set_cell_link()` maps individual XML
nodes to individual cells, orcus also supports mapping repeating XML
elements to a range of rows — similar to how a database table is
structured.  Use :cpp:func:`~orcus::orcus_xml::start_range()` to begin
defining a range mapping,
:cpp:func:`~orcus::orcus_xml::append_field_link()` to map each XML node
to a named column, :cpp:func:`~orcus::orcus_xml::set_range_row_group()`
to identify the repeating element that determines row boundaries, and
:cpp:func:`~orcus::orcus_xml::commit_range()` to finalize the mapping:

.. literalinclude:: ../../../doc_example/xml/xml_mapping_repeat.cpp
   :language: C++
   :start-after: //!code-start: range
   :end-before: //!code-end: range
   :dedent: 4

The first argument to :cpp:func:`~orcus::orcus_xml::start_range()` is
the target sheet name, followed by the row and column of the top-left
corner of the range.  Each call to
:cpp:func:`~orcus::orcus_xml::append_field_link()` takes an XPath
expression and a column label.  The call to
:cpp:func:`~orcus::orcus_xml::set_range_row_group()` tells orcus that
each ``<city>`` element represents one row in the output.  Once
:cpp:func:`~orcus::orcus_xml::commit_range()` is called, the mapping is
finalized and ready for parsing.  Re-running the code with this range
mapping added should produce the following output:

.. code-block:: none

   rows: 5  cols: 5
   +----------+----------------+--------------+----------------------------------------------+-------------------+
   | City     | Country        | Population   | Fact                                         | Popular Spot      |
   +----------+----------------+--------------+----------------------------------------------+-------------------+
   | Tokyo    | Japan          | 37400000 [v] | World's most populous metropolitan area.     | Tokyo Skytree     |
   +----------+----------------+--------------+----------------------------------------------+-------------------+
   | New York | United States  | 19200000 [v] | Home to the United Nations headquarters.     | Statue of Liberty |
   +----------+----------------+--------------+----------------------------------------------+-------------------+
   | London   | United Kingdom | 9300000 [v]  | One of the world's oldest financial centers. | Big Ben           |
   +----------+----------------+--------------+----------------------------------------------+-------------------+
   | Paris    | France         | 12100000 [v] | Known as the City of Light.                  | Eiffel Tower      |
   +----------+----------------+--------------+----------------------------------------------+-------------------+

Note that while the example above runs the header metadata mapping and
the city range mapping as two separate sessions, cell links and range
mappings can be freely mixed within the same session.  When mixed, both
become active when :cpp:func:`~orcus::orcus_xml::read_stream()` is
called, and orcus will populate all of them in a single pass over the
document.  You can also define multiple ranges in the same session by
calling :cpp:func:`~orcus::orcus_xml::start_range()`,
:cpp:func:`~orcus::orcus_xml::append_field_link()`,
:cpp:func:`~orcus::orcus_xml::set_range_row_group()`, and
:cpp:func:`~orcus::orcus_xml::commit_range()` again for each additional
range before calling :cpp:func:`~orcus::orcus_xml::read_stream()`.
