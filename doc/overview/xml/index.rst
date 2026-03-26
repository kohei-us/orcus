
XML
===

Orcus provides a set of low-level, high-performance XML parsing
primitives that can be used independently of its spreadsheet import
features.  At the foundation is :cpp:class:`~orcus::sax_parser`, a
template-based SAX parser that fires event callbacks into a
user-supplied handler as it encounters elements, attributes, text
content, declarations, CDATA sections, and DOCTYPE declarations.  On top
of it sit two higher-level variants: :cpp:class:`~orcus::sax_ns_parser`,
which adds namespace resolution and element-scope tracking via an
:cpp:class:`~orcus::xmlns_context` object, and
:cpp:class:`~orcus::sax_token_parser`, which further tokenizes element
and attribute names into integer tokens against a predefined vocabulary
for faster downstream dispatch.

Namespace management is handled by two cooperating types.
:cpp:class:`~orcus::xmlns_repository` is a session-level intern table
that maps each unique namespace URI to a stable
:cpp:type:`~orcus::xmlns_id_t` identifier.
:cpp:class:`~orcus::xmlns_context` is a per-document companion that
manages the stack of active prefix-to-URI bindings as parsing progresses
through element scopes; a fresh context should be created from the
repository for each XML stream.

Finally, orcus provides a higher-level mapping feature through
:cpp:class:`~orcus::orcus_xml`, which allows you to project the contents
of an XML document onto a spreadsheet by defining how repeating
structures in the XML tree correspond to rows and columns in a sheet.
This mapping can be defined either via a map file or programmatically
through the C++ API.

Contents:

.. toctree::
   :maxdepth: 1

   mapping.rst

