
.. highlight:: cpp

JSON
====

The JSON part of orcus consists of a low-level parser class that handles
parsing of JSON strings, and a high-level document class that stores parsed
JSON structures as a node tree.

There are two approaches to processing JSON strings using the orcus library.
One approach is to utilize the :cpp:class:`~orcus::json::document_tree` class
to load and populate the JSON structure tree via its
:cpp:func:`~orcus::json::document_tree::load()` method and traverse the tree
through its :cpp:func:`~orcus::json::document_tree::get_document_root()` method.
This approach is ideal if you want a quick way to parse and access the content
of a JSON document with minimal effort.

Another approach is to use the low-level :cpp:class:`~orcus::json_parser`
class directly by providing your own handler class to receive callbacks from
the parser.  This method requires a bit more effort on your part to provide
and populate your own data structure, but if you already have a data structure
to store the content of JSON, then this approach is ideal.  The
:cpp:class:`~orcus::json::document_tree` class internally uses
:cpp:class:`~orcus::json_parser` to parse JSON contents.

Contents:

.. toctree::
   :maxdepth: 1

   doctree.rst
   parser.rst
   doctree-direct.rst
   subtree.rst
