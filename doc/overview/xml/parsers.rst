.. _xml-parsers:

Low-level XML parsers
=====================

Orcus provides three low-level SAX-style XML parsers that form a layered
hierarchy.  Each is a class template parameterized on a user-supplied *handler*
type, and each fires event callbacks into that handler as it walks the document:

* :cpp:class:`~orcus::sax_parser` is the foundation.  It reports elements,
  attributes, text content, declarations and other low-level events without any
  namespace awareness.
* :cpp:class:`~orcus::sax_ns_parser` builds on top of it, adding namespace
  resolution so that each element and attribute carries a resolved namespace
  identifier in addition to its prefix.
* :cpp:class:`~orcus::sax_token_parser` builds on the namespace parser, further
  translating element and attribute names into integer *tokens* against a
  predefined vocabulary so that downstream code can dispatch on integers rather
  than strings.

In all three cases the handler does not need to be derived from any particular
base class; the parser simply calls the expected member functions on whatever
type it is given.  The library does, however, provide base handler classes
(:cpp:class:`~orcus::sax_handler`, :cpp:class:`~orcus::sax_ns_handler` and
:cpp:class:`~orcus::sax_token_handler`) that supply empty implementations for
every callback, so that by deriving from one of them you only need to implement
the callbacks you actually care about.

.. note::

   Several callbacks receive a ``transient`` flag.  When it is set, the string
   value was decoded into a temporary buffer because it contained one or more
   encoded characters, and is only valid for the duration of the callback.  In
   that case the handler must copy or intern the value before returning rather
   than holding on to the ``std::string_view``.


Basic parsing with sax_parser
-----------------------------

:cpp:class:`~orcus::sax_parser` is the most basic of the three.  It does not
track namespaces and does not verify that opening and closing tags match; it
simply reports each event as it is encountered.

Start by including the parser header:

.. literalinclude:: ../../../doc_example/xml/sax_parser_1.cpp
   :language: C++
   :start-after: //!code-start: headers
   :end-before: //!code-end: headers

Define a handler that derives from :cpp:class:`~orcus::sax_handler` and
overrides the callbacks of interest.  This one prints each element, attribute
and text segment:

.. literalinclude:: ../../../doc_example/xml/sax_parser_1.cpp
   :language: C++
   :start-after: //!code-start: handler
   :end-before: //!code-end: handler

The :cpp:func:`~orcus::sax_handler::characters` callback skips whitespace-only
segments here, since the indentation between elements is itself reported as text
content.  Refer to the :cpp:class:`~orcus::sax_handler` class definition for the
full set of available callbacks.

Next, prepare the XML content to parse:

.. literalinclude:: ../../../doc_example/xml/sax_parser_1.cpp
   :language: C++
   :start-after: //!code-start: content
   :end-before: //!code-end: content
   :dedent: 4

Finally, construct the parser with the content and the handler, and parse:

.. literalinclude:: ../../../doc_example/xml/sax_parser_1.cpp
   :language: C++
   :start-after: //!code-start: parse
   :end-before: //!code-end: parse
   :dedent: 4

Note that the attributes of an element are reported through the
:cpp:func:`~orcus::sax_handler::attribute` callback *before* the element's
:cpp:func:`~orcus::sax_handler::start_element` callback fires.  Executing this
code generates the following output:

.. code-block:: text

      attribute: version='1.0'
    start element: catalog
      attribute: id='b1'
    start element: book
      characters: Go
    end element: book
      attribute: id='b2'
    start element: book
      characters: C++
    end element: book
    end element: catalog


Namespace-aware parsing with sax_ns_parser
-------------------------------------------

:cpp:class:`~orcus::sax_ns_parser` adds namespace handling on top of the basic
parser.  It uses an :cpp:class:`~orcus::xmlns_context` to resolve namespace
prefixes into stable :cpp:type:`~orcus::xmlns_id_t` identifiers, and tracks
element scopes so that non-matching closing tags are detected.

Include the parser and namespace headers:

.. literalinclude:: ../../../doc_example/xml/sax_ns_parser_1.cpp
   :language: C++
   :start-after: //!code-start: headers
   :end-before: //!code-end: headers

Define the handler.  The element and attribute structs passed to it carry both
the source prefix and the resolved namespace identifier, and namespace
declarations are reported through the
:cpp:func:`~orcus::sax_ns_handler::namespace_declaration` callback:

.. literalinclude:: ../../../doc_example/xml/sax_ns_parser_1.cpp
   :language: C++
   :start-after: //!code-start: handler
   :end-before: //!code-end: handler

Because the derived handler declares its own
:cpp:func:`~orcus::sax_ns_handler::attribute` overload, the ``using`` declaration
keeps the base overload that handles declaration and processing-instruction
attributes visible.

Prepare some namespaced XML content:

.. literalinclude:: ../../../doc_example/xml/sax_ns_parser_1.cpp
   :language: C++
   :start-after: //!code-start: content
   :end-before: //!code-end: content
   :dedent: 4

Create an :cpp:class:`~orcus::xmlns_repository` and a fresh
:cpp:class:`~orcus::xmlns_context` from it.  A new context should be created per
stream:

.. literalinclude:: ../../../doc_example/xml/sax_ns_parser_1.cpp
   :language: C++
   :start-after: //!code-start: context
   :end-before: //!code-end: context
   :dedent: 4

Then construct the parser - passing the context in addition to the content and
handler - and parse:

.. literalinclude:: ../../../doc_example/xml/sax_ns_parser_1.cpp
   :language: C++
   :start-after: //!code-start: parse
   :end-before: //!code-end: parse
   :dedent: 4

For more on namespace management see the
:cpp:class:`~orcus::xmlns_repository` and :cpp:class:`~orcus::xmlns_context`
class definitions.  Executing this code generates the following output:

.. code-block:: text

    namespace declaration: alias='' uri='http://example.com/default'
    namespace declaration: alias='x' uri='http://example.com/extra'
    start element: list (ns: http://example.com/default)
      attribute: rank='1' (ns: http://example.com/extra)
    start element: item (ns: http://example.com/default)
    end element: item
      attribute: rank='2' (ns: http://example.com/extra)
    start element: item (ns: http://example.com/default)
    end element: item
    end element: list


Tokenized parsing with sax_token_parser
----------------------------------------

:cpp:class:`~orcus::sax_token_parser` further translates element and attribute
names into integer tokens while parsing.  The caller supplies a predefined set
of names via a :cpp:class:`~orcus::tokens` instance; any name found in that
vocabulary is reported as its integer token, while names not in the vocabulary
are reported as :cpp:var:`~orcus::XML_UNKNOWN_TOKEN`.

Include the parser, token store and namespace headers:

.. literalinclude:: ../../../doc_example/xml/sax_token_parser_1.cpp
   :language: C++
   :start-after: //!code-start: headers
   :end-before: //!code-end: headers

Define the token vocabulary.  Each name maps to a token by its position in the
array, with index ``0`` reserved for :cpp:var:`~orcus::XML_UNKNOWN_TOKEN`:

.. literalinclude:: ../../../doc_example/xml/sax_token_parser_1.cpp
   :language: C++
   :start-after: //!code-start: tokens
   :end-before: //!code-end: tokens

Define the handler.  Because known names arrive as integer tokens, it can
dispatch on them - for example with a ``switch`` statement - instead of
comparing strings.  The :cpp:struct:`~orcus::xml_token_element_t` passed to the
handler carries the element's token, its raw name, and the list of tokenized
attributes:

.. literalinclude:: ../../../doc_example/xml/sax_token_parser_1.cpp
   :language: C++
   :start-after: //!code-start: handler
   :end-before: //!code-end: handler

Prepare the XML content.  The ``<magazine>`` element is intentionally absent
from the vocabulary so that its token resolves to
:cpp:var:`~orcus::XML_UNKNOWN_TOKEN`:

.. literalinclude:: ../../../doc_example/xml/sax_token_parser_1.cpp
   :language: C++
   :start-after: //!code-start: content
   :end-before: //!code-end: content
   :dedent: 4

Construct the :cpp:class:`~orcus::tokens` store and an
:cpp:class:`~orcus::xmlns_context`.  The token store is not copyable and is
typically created once as a global constant:

.. literalinclude:: ../../../doc_example/xml/sax_token_parser_1.cpp
   :language: C++
   :start-after: //!code-start: setup
   :end-before: //!code-end: setup
   :dedent: 4

Finally, construct the parser with the content, token store, context and
handler, and parse:

.. literalinclude:: ../../../doc_example/xml/sax_token_parser_1.cpp
   :language: C++
   :start-after: //!code-start: parse
   :end-before: //!code-end: parse
   :dedent: 4

Executing this code generates the following output:

.. code-block:: text

    start element: catalog (token=1) -> recognized as catalog
    start element: book (token=2) -> recognized as book
      attribute: id (token=3) = 'b1' -> recognized as id
    start element: book (token=2) -> recognized as book
      attribute: id (token=3) = 'b2' -> recognized as id
    start element: magazine (token=0) -> unknown
      attribute: id (token=3) = 'm1' -> recognized as id
