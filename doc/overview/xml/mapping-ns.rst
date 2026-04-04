.. _xml-mapping-ns:

Mapping XML with namespaces
===========================

This section extends the :ref:`previous example of mapping a basic XML
document <xml-mapping-basic>` to cover documents that use XML
namespaces.  When element and attribute names are namespace-qualified,
the XPath expressions used to identify them must include the
corresponding namespace prefixes.

Consider the following XML document:

.. literalinclude:: ../../../doc_example/xml/files/server-logs.xml
   :language: XML

The root element ``<log:serverLogs>`` and every child element carry the
``log`` namespace prefix, while the ``host`` and ``date`` attributes on the
root element carry the ``meta`` prefix.  Each prefix is bound to a URI in the
document's namespace declarations.

The setup is the same as in the :ref:`basic example <xml-mapping-basic>`.
First, load the input file:

.. literalinclude:: ../../../doc_example/xml/xml_mapping_ns.cpp
   :language: C++
   :start-after: //!code-start: load-input
   :end-before: //!code-end: load-input
   :dedent: 4

then create a spreadsheet document and an import factory:

.. literalinclude:: ../../../doc_example/xml/xml_mapping_ns.cpp
   :language: C++
   :start-after: //!code-start: create-doc
   :end-before: //!code-end: create-doc
   :dedent: 4

and finally construct the :cpp:class:`~orcus::orcus_xml` filter and pass
an :cpp:class:`~orcus::xmlns_repository` instance and the factory instance to it:

.. literalinclude:: ../../../doc_example/xml/xml_mapping_ns.cpp
   :language: C++
   :start-after: //!code-start: setup-orcus-xml
   :end-before: //!code-end: setup-orcus-xml
   :dedent: 4

Here is where we need to do something different; before defining any
mapping rules, register short aliases for the namespace URIs used in the
document by calling :cpp:func:`~orcus::orcus_xml::set_namespace_alias()`
one per alias:

.. literalinclude:: ../../../doc_example/xml/xml_mapping_ns.cpp
   :language: C++
   :start-after: //!code-start: ns-aliases
   :end-before: //!code-end: ns-aliases
   :dedent: 4

Each call maps a short prefix string to its full URI.  The prefixes chosen
here do not need to match the ones declared in the XML document; they are
local to the mapping session and are used solely to qualify element and
attribute names inside the XPath expressions that follow.

.. note::

   Documents that declare a *default namespace* (``xmlns="..."``) require
   special handling.  Pass an empty string as the alias to mark that URI as
   the default namespace for the mapping session::

      filter.set_namespace_alias("", "http://example.com/default-ns");

   Once a default namespace is set, any unprefixed name in an XPath expression
   is automatically resolved to that namespace, so the paths can be written
   without a prefix::

      filter.set_cell_link("/root/child", "Sheet", 0, 0);

With the aliases in place, define cell links for the two metadata attributes
on the root element:

.. literalinclude:: ../../../doc_example/xml/xml_mapping_ns.cpp
   :language: C++
   :start-after: //!code-start: cell-links
   :end-before: //!code-end: cell-links
   :dedent: 4

The namespace prefix appears before the colon in each path segment, so
``/log:serverLogs/@meta:host`` targets an attribute named ``host`` in the
``meta`` namespace on the root element.

Define a range mapping for the repeating ``<log:entry>`` elements:

.. literalinclude:: ../../../doc_example/xml/xml_mapping_ns.cpp
   :language: C++
   :start-after: //!code-start: range
   :end-before: //!code-end: range
   :dedent: 4

Namespace prefixes are required on every qualified name in the path.  The
``@log:id`` segment selects the ``id`` attribute in the ``log`` namespace.

Finally, insert a new sheet, parse the input:

.. literalinclude:: ../../../doc_example/xml/xml_mapping_ns.cpp
   :language: C++
   :start-after: //!code-start: read-xml
   :end-before: //!code-end: read-xml
   :dedent: 4

and dump the sheet content:

.. literalinclude:: ../../../doc_example/xml/xml_mapping_ns.cpp
   :language: C++
   :start-after: //!code-start: dump-content
   :end-before: //!code-end: dump-content
   :dedent: 4

Note that the two label strings ``"Host"`` and ``"Date"`` are inserted into
the sheet programmatically after parsing, since the XML document does not
contain them.  The values in column 1 of those rows were already populated by
the cell links during :cpp:func:`~orcus::orcus_xml::read_stream()`.

Running this code produces the following output:

.. code-block:: none

   rows: 10  cols: 5
   +-------+-------------+----------------+--------------------------------------------------------------------+----------------------+
   | Host  | web-prod-04 |                |                                                                    |                      |
   +-------+-------------+----------------+--------------------------------------------------------------------+----------------------+
   | Date  | 2026-03-23  |                |                                                                    |                      |
   +-------+-------------+----------------+--------------------------------------------------------------------+----------------------+
   |       |             |                |                                                                    |                      |
   +-------+-------------+----------------+--------------------------------------------------------------------+----------------------+
   | ID    | Level       | Service        | Message                                                            | Timestamp            |
   +-------+-------------+----------------+--------------------------------------------------------------------+----------------------+
   | 1 [v] | INFO        | AuthService    | User alice@example.com authenticated successfully.                 | 2026-03-23T08:02:11Z |
   +-------+-------------+----------------+--------------------------------------------------------------------+----------------------+
   | 2 [v] | WARN        | AuthService    | Failed login attempt for user bob@example.com. Attempt 3 of 5.     | 2026-03-23T08:14:37Z |
   +-------+-------------+----------------+--------------------------------------------------------------------+----------------------+
   | 3 [v] | ERROR       | SessionManager | Cache connection timed out after 30s. Session store unreachable.   | 2026-03-23T08:31:05Z |
   +-------+-------------+----------------+--------------------------------------------------------------------+----------------------+
   | 4 [v] | INFO        | SessionManager | Cache connection restored. Resuming normal operations.             | 2026-03-23T08:31:09Z |
   +-------+-------------+----------------+--------------------------------------------------------------------+----------------------+
   | 5 [v] | ERROR       | ApiGateway     | Request to /api/orders returned 503. Upstream service unavailable. | 2026-03-23T09:45:22Z |
   +-------+-------------+----------------+--------------------------------------------------------------------+----------------------+
   | 6 [v] | INFO        | Scheduler      | Daily report job completed. 1,402 records processed in 4.2s.       | 2026-03-23T10:00:00Z |
   +-------+-------------+----------------+--------------------------------------------------------------------+----------------------+
