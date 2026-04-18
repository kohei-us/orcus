.. _xml-mapping-autodetect:

Mapping XML with automatic structure detection
==============================================

This section extends the :ref:`previous example <xml-mapping-ns>` to show how
:cpp:class:`~orcus::orcus_xml` can detect the mapping structure of an XML
document automatically, without any hand-written XPath rules.

Simple auto-detection
---------------------

The setup is identical to the previous example.  The only difference is that
instead of registering namespace aliases and mapping rules manually, a single
call to :cpp:func:`~orcus::orcus_xml::detect_map_definition` handles everything:

.. literalinclude:: ../../../doc_example/xml/xml_mapping_autodetect.cpp
   :language: C++
   :start-after: //!code-start: simple-detect
   :end-before: //!code-end: simple-detect
   :dedent: 4

:cpp:func:`~orcus::orcus_xml::detect_map_definition` scans the document,
identifies every repeating element group, registers namespace aliases
automatically, and populates the internal mapping rules.  Each detected range
is assigned to a new sheet named ``range-0``, ``range-1``, and so on.
:cpp:func:`~orcus::orcus_xml::read_stream` then imports the data using those
rules.

The output is:

.. code-block:: none

   rows: 7  cols: 5
   +--------+----------------------+-----------+----------------+--------------------------------------------------------------------+
   | ns0:id | ns0:timestamp        | ns0:level | ns0:service    | ns0:message                                                        |
   +--------+----------------------+-----------+----------------+--------------------------------------------------------------------+
   | 1 [v]  | 2026-03-23T08:02:11Z | INFO      | AuthService    | User alice@example.com authenticated successfully.                 |
   +--------+----------------------+-----------+----------------+--------------------------------------------------------------------+
   | 2 [v]  | 2026-03-23T08:14:37Z | WARN      | AuthService    | Failed login attempt for user bob@example.com. Attempt 3 of 5.     |
   +--------+----------------------+-----------+----------------+--------------------------------------------------------------------+
   | 3 [v]  | 2026-03-23T08:31:05Z | ERROR     | SessionManager | Cache connection timed out after 30s. Session store unreachable.   |
   +--------+----------------------+-----------+----------------+--------------------------------------------------------------------+
   | 4 [v]  | 2026-03-23T08:31:09Z | INFO      | SessionManager | Cache connection restored. Resuming normal operations.             |
   +--------+----------------------+-----------+----------------+--------------------------------------------------------------------+
   | 5 [v]  | 2026-03-23T09:45:22Z | ERROR     | ApiGateway     | Request to /api/orders returned 503. Upstream service unavailable. |
   +--------+----------------------+-----------+----------------+--------------------------------------------------------------------+
   | 6 [v]  | 2026-03-23T10:00:00Z | INFO      | Scheduler      | Daily report job completed. 1,402 records processed in 4.2s.       |
   +--------+----------------------+-----------+----------------+--------------------------------------------------------------------+

The column headers carry ``ns0:`` prefixes because the auto-detection assigns
short aliases to each namespace it encounters, and those aliases become part of
the field names used as column headers.  ``ns0`` corresponds to the primary
namespace ``http://example.com/server-logs`` in this document.  The aliases are
assigned in the order the namespaces are first seen during parsing, so
``ns0`` is always the first-encountered namespace and ``ns1`` the second, and
so on.

.. note::

   :cpp:func:`~orcus::orcus_xml::detect_map_definition` also appends the
   necessary sheets to the document automatically — one per detected range —
   so there is no need to call :cpp:func:`~orcus::orcus_xml::append_sheet`
   manually.

Inspecting and editing the map definition
-----------------------------------------

:cpp:func:`~orcus::orcus_xml::detect_map_definition` commits the mapping
immediately.  When you need to inspect or adjust the detected rules before
importing, use :cpp:func:`~orcus::orcus_xml::write_map_definition` instead.
It performs the same structure analysis but serializes the result to a map
definition XML string rather than applying it:

.. literalinclude:: ../../../doc_example/xml/xml_mapping_autodetect.cpp
   :language: C++
   :start-after: //!code-start: write-map-def
   :end-before: //!code-end: write-map-def
   :dedent: 4

This code will print the map definition XML to stdout, but do note that it
is written as a single unbroken line with no indentation.  The following is
the pretty-printed form of the map definition XML:

.. code-block:: xml

   <?xml version="1.0"?>
   <map xmlns="https://gitlab.com/orcus/orcus/xml-map-definition">
     <ns alias="ns0" uri="http://example.com/server-logs"/>
     <ns alias="ns1" uri="http://example.com/server-logs/meta"/>
     <sheet name="range-0"/>
     <range sheet="range-0" row="0" column="0">
       <field path="/ns0:serverLogs/ns0:entry/@ns0:id"/>
       <field path="/ns0:serverLogs/ns0:entry/ns0:level"/>
       <field path="/ns0:serverLogs/ns0:entry/ns0:message"/>
       <field path="/ns0:serverLogs/ns0:entry/ns0:service"/>
       <field path="/ns0:serverLogs/ns0:entry/ns0:timestamp"/>
       <row-group path="/ns0:serverLogs/ns0:entry"/>
     </range>
   </map>

Because the document uses XML namespaces, the auto-detection assigns short
aliases automatically: ``ns0`` for the ``log`` namespace and ``ns1`` for the
``meta`` namespace, indexed in the order they are first encountered during
parsing.  The string can be edited freely at this point — fields can be
removed, reordered, or relabelled, sheet names can be changed, and unwanted
ranges can be dropped entirely.

Once any edits are complete, load the definition and import the data:

.. literalinclude:: ../../../doc_example/xml/xml_mapping_autodetect.cpp
   :language: C++
   :start-after: //!code-start: read-map-def
   :end-before: //!code-end: read-map-def
   :dedent: 4

:cpp:func:`~orcus::orcus_xml::read_map_definition` parses the map definition
XML, appends the necessary sheets to the document, and populates the internal
mapping rules, exactly as if the rules had been set up manually.
:cpp:func:`~orcus::orcus_xml::read_stream` then applies those rules to import
the source document.
