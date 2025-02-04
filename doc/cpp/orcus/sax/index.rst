.. _ns-orcus-sax:

namespace orcus::sax
====================

Enum
----

parse_token_t
^^^^^^^^^^^^^
.. doxygenenum:: orcus::sax::parse_token_t


Type aliases
------------

parse_tokens_t
^^^^^^^^^^^^^^
.. doxygentypedef:: orcus::sax::parse_tokens_t


Functions
---------

decode_xml_encoded_char
^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::sax::decode_xml_encoded_char(const char *p, size_t n)

decode_xml_unicode_char
^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::sax::decode_xml_unicode_char(const char *p, size_t n)


Struct
------

doctype_declaration
^^^^^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::sax::doctype_declaration
   :members:

parse_token
^^^^^^^^^^^
.. doxygenstruct:: orcus::sax::parse_token
   :members:

parser_attribute
^^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::sax::parser_attribute
   :members:

parser_element
^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::sax::parser_element
   :members:


Classes
-------

parser_base
^^^^^^^^^^^
.. doxygenclass:: orcus::sax::parser_base
   :members:

parser_thread
^^^^^^^^^^^^^
.. doxygenclass:: orcus::sax::parser_thread
   :members:


Child namespaces
----------------
.. toctree::
   :maxdepth: 1

   detail/index.rst