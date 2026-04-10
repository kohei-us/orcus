
.. highlight:: cpp

JSON
====

The JSON support in orcus covers three distinct use cases.

The first is **document parsing**, where you want to load a JSON string into
memory and traverse its contents.  The high-level
:cpp:class:`~orcus::json::document_tree` class handles this: call its
:cpp:func:`~orcus::json::document_tree::load()` method to populate the tree,
then navigate it via :cpp:func:`~orcus::json::document_tree::get_document_root()`.
For cases where you already have your own data structure to populate, the
low-level :cpp:class:`~orcus::json_parser` class lets you supply a handler
that receives parser callbacks directly.
:cpp:class:`~orcus::json::document_tree` itself is built on top of
:cpp:class:`~orcus::json_parser`.

The second is **subtree extraction**, where you want to isolate a portion of
a larger JSON document using a JSONPath expression.
:cpp:class:`~orcus::json::subtree` supports this by taking a document and a
path expression and exposing the selected subtree for further inspection or
serialisation.

The third is **spreadsheet mapping**, where you want to project the contents
of a JSON document onto a spreadsheet by declaring how repeating structures
map to rows and columns.  :cpp:class:`~orcus::orcus_json` provides this
capability, either through a programmatic API where you register paths and
ranges by hand, or through automatic structure detection that infers the
mapping from the document itself.

Parsing
-------

.. toctree::
   :maxdepth: 1

   doctree.rst
   parser.rst
   doctree-direct.rst

Subtree extraction
------------------

.. toctree::
   :maxdepth: 1

   subtree.rst

JSON mapping
------------

.. toctree::
   :maxdepth: 1

   mapping.rst
   mapping-autodetect.rst
