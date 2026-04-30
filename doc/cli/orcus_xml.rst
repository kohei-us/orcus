orcus-xml
=========

Help output
-----------

.. literalinclude:: ../_static/cli/orcus-xml-help.txt
   :language: none

Supported modes
---------------

This command supports the following modes:

* dump
* lint
* map
* map-gen
* structure
* transform

dump
^^^^

The ``dump`` mode parses the XML document into a full DOM tree and prints its
content in a compact, human-readable format to standard output.  This is useful
for quickly inspecting the full content of a document.

lint
^^^^

The ``lint`` mode is used to reformat an XML document optionally with a
different indent level.

map and map-gen
^^^^^^^^^^^^^^^

The ``map`` and ``map-gen`` modes are related, and are typically used together.
The ``map`` mode is used to map an XML document to a spreadsheet document model
with a user-defined mapping rule, and the ``map-gen`` mode is used to
auto-generate a mapping rule based on the structure of the source XML document.

Refer to the :ref:`xml-mapping-basic` section for a detailed example of how to
use these modes to map an XML document to a spreadsheet document model.

structure
^^^^^^^^^

The ``structure`` mode analyses the overall structure of the source XML document
and prints a compact representation of all unique element paths to standard
output.  Unlike the ``dump`` mode, which prints the full document content, this
mode focuses on the schema-like element hierarchy, making it useful for
understanding the shape of an unfamiliar document.

transform
^^^^^^^^^

The ``transform`` mode loads the XML document into a spreadsheet document model
via a mapping rule, then writes the mapped content back out as an XML file.  It
requires both a map file (``--map``) and an output file path (``--output``).


Example usage
-------------

Reformat XML document
^^^^^^^^^^^^^^^^^^^^^

You can use this command to re-format an XML document by specifying ``--mode``
to ``lint``, with an optional indent level via ``--indent`` option.  The
following command reformats the input XML file with an indent level of 2:

.. code-block:: none

   orcus-xml --mode lint --indent 2 path/to/input.xml

The command writes the output to standard output by default, or you can specify
the ``--output`` option to have it written to a local file instead.  Specifying
``--indent 0`` produces compact single-line output.

Inspect document structure
^^^^^^^^^^^^^^^^^^^^^^^^^^

To get a quick overview of the element hierarchy of an XML document without
looking at its full content, use the ``structure`` mode:

.. code-block:: none

   orcus-xml --mode structure path/to/input.xml

Generate a map file
^^^^^^^^^^^^^^^^^^^

To auto-generate a map file from an XML document, use the ``map-gen`` mode:

.. code-block:: none

   orcus-xml --mode map-gen path/to/input.xml -o map.xml

The generated ``map.xml`` can then be edited and used with the ``map`` or
``transform`` modes via the ``--map`` option.
