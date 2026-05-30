.. _xml-dom-tree:

DOM tree
========

:cpp:class:`~orcus::dom::document_tree` is a DOM-style, in-memory representation
of an XML document.  It is built on top of the low-level
:ref:`SAX parsers <xml-parsers>`: loading a document runs a parser internally
and assembles the resulting events into a navigable tree of nodes.

The tree is accessed through lightweight value-type handles.
:cpp:class:`~orcus::dom::const_node` is a read-only handle that exposes a node's
type, name, attributes, children and parent, while
:cpp:class:`~orcus::dom::node` derives from it and adds methods for building and
editing the tree in place.  Both handles refer to storage owned by the
enclosing :cpp:class:`~orcus::dom::document_tree`, so they must not be used after
the tree is destroyed.  Names are represented by
:cpp:struct:`~orcus::dom::entity_name`, which pairs a local name with an optional
namespace identifier.

.. note::

   A :cpp:class:`~orcus::dom::document_tree` is constructed with a reference to
   an :cpp:class:`~orcus::xmlns_repository`, which it uses to create namespace
   identifiers for the names it stores.  The repository must outlive the tree.

Both examples below share the following headers:

.. literalinclude:: ../../../doc_example/xml/dom_tree_1.cpp
   :language: C++
   :start-after: //!code-start: headers
   :end-before: //!code-end: headers


Loading and navigating
-----------------------

Consider the following XML document, stored in a file named ``library.xml``:

.. literalinclude:: ../../../doc_example/xml/files/library.xml
   :language: XML

Construct a tree from an :cpp:class:`~orcus::xmlns_repository`, load the file
into memory with :cpp:class:`~orcus::file_content`, and parse it with
:cpp:func:`~orcus::dom::document_tree::load()`:

.. literalinclude:: ../../../doc_example/xml/dom_tree_1.cpp
   :language: C++
   :start-after: //!code-start: load
   :end-before: //!code-end: load
   :dedent: 4

``INPUTDIR`` is a constant that stores a path to the directory where the input
file is located.

Obtain the root element with :cpp:func:`~orcus::dom::document_tree::root()` and
inspect it.  Attribute values are looked up by name via
:cpp:func:`~orcus::dom::const_node::attribute`, which returns an empty value when
no such attribute exists:

.. literalinclude:: ../../../doc_example/xml/dom_tree_1.cpp
   :language: C++
   :start-after: //!code-start: navigate-root
   :end-before: //!code-end: navigate-root
   :dedent: 4

Walk the child elements with
:cpp:func:`~orcus::dom::const_node::child_count` and
:cpp:func:`~orcus::dom::const_node::child`.  Note that ``child_count()`` counts
only child *elements*, so the whitespace between the elements in the source does
not contribute to the count:

.. literalinclude:: ../../../doc_example/xml/dom_tree_1.cpp
   :language: C++
   :start-after: //!code-start: navigate-children
   :end-before: //!code-end: navigate-children
   :dedent: 4

This produces the following output:

.. code-block:: text

    --- load and navigate ---
    root: library
      name attribute: City Library
      child count: 2
      child 0: book id=b1 title='The Go Programming Language'
      child 1: book id=b2 title='Effective Modern C++'


Building a tree
---------------

The same API can build a document from scratch.
:cpp:func:`~orcus::dom::document_tree::set_root()` installs the root element and
returns a mutable :cpp:class:`~orcus::dom::node`, which is then populated with
:cpp:func:`~orcus::dom::node::append_element()`,
:cpp:func:`~orcus::dom::node::set_attribute()` and
:cpp:func:`~orcus::dom::node::append_content()`:

.. literalinclude:: ../../../doc_example/xml/dom_tree_1.cpp
   :language: C++
   :start-after: //!code-start: build
   :end-before: //!code-end: build
   :dedent: 4

.. note::

   Each name is passed as an :cpp:struct:`~orcus::dom::entity_name`.  The braces
   in ``{"message"}`` are what construct that ``entity_name`` from the string:
   passing a bare string literal would not compile, because converting it to an
   ``entity_name`` requires two user-defined conversions (first to
   ``std::string_view``, then to ``entity_name``), and only one is allowed in an
   implicit conversion.  The braced-initializer form sidesteps this by
   constructing the argument in place.  To give a name a namespace, pass both an
   :cpp:type:`~orcus::xmlns_id_t` and the local name, as in ``{ns, "message"}``.

Finally, serialize the tree back to XML with
:cpp:func:`~orcus::dom::document_tree::dump()`.  The indent argument gives the
number of spaces per nesting level:

.. literalinclude:: ../../../doc_example/xml/dom_tree_1.cpp
   :language: C++
   :start-after: //!code-start: serialize
   :end-before: //!code-end: serialize
   :dedent: 4

This produces the following output:

.. code-block:: text

    --- build and serialize ---
    <message lang="en">
      <greeting>Hello, world!</greeting>
    </message>
