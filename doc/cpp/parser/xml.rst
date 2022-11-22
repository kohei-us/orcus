.. highlight:: cpp

XML Parsers
===========

SAX base parser
---------------

.. doxygenclass:: orcus::sax_parser
   :members:

.. doxygenstruct:: orcus::sax_parser_default_config
   :members:

.. doxygenclass:: orcus::sax_handler
   :members:

.. doxygenstruct:: orcus::sax::parser_element
   :members:

.. doxygenstruct:: orcus::sax::parser_attribute
   :members:

SAX namespace parser
--------------------

.. doxygenclass:: orcus::sax_ns_parser
   :members:

.. doxygenclass:: orcus::sax_ns_handler
   :members:

.. doxygenstruct:: orcus::sax_ns_parser_element
   :members:

.. doxygenstruct:: orcus::sax_ns_parser_attribute
   :members:

SAX token parser
----------------

.. doxygenclass:: orcus::sax_token_parser
   :members:

.. doxygenclass:: orcus::sax_token_handler
   :members:

Namespace
---------

.. doxygenclass:: orcus::xmlns_repository
   :members:

.. doxygenclass:: orcus::xmlns_context
   :members:

Common
------

.. doxygenstruct:: orcus::sax::doctype_declaration
   :members:

.. doxygenfunction:: orcus::sax::decode_xml_encoded_char

.. doxygenfunction:: orcus::sax::decode_xml_unicode_char
