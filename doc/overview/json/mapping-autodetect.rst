.. _json-mapping-autodetect:

Mapping JSON with automatic structure detection
===============================================

This section demonstrates how :cpp:class:`~orcus::orcus_json` can detect the
mapping structure of a JSON document automatically, without writing any path
expressions by hand.

Consider the following JSON document:

.. literalinclude:: ../../../doc_example/json/files/planets.json
   :language: JSON

The document is a top-level array of objects, each with the same set of
fields.

Simple auto-detection
---------------------

The setup is identical to the :ref:`previous example <json-mapping-basic>`:
create a document, a factory, and a filter instance:

.. literalinclude:: ../../../doc_example/json/json_mapping_autodetect.cpp
   :language: C++
   :start-after: //!code-start: simple-setup
   :end-before: //!code-end: simple-setup
   :dedent: 4

Then replace the manual path registration with a single call to
:cpp:func:`~orcus::orcus_json::detect_map_definition`, followed by
:cpp:func:`~orcus::orcus_json::read_stream`:

.. literalinclude:: ../../../doc_example/json/json_mapping_autodetect.cpp
   :language: C++
   :start-after: //!code-start: simple-detect
   :end-before: //!code-end: simple-detect
   :dedent: 4

:cpp:func:`~orcus::orcus_json::detect_map_definition` analyzes the document,
identifies every repeating array of objects, and registers the mapping rules
internally.  Each detected range is assigned to a new sheet named
``range-0``, ``range-1``, and so on.
:cpp:func:`~orcus::orcus_json::read_stream` then imports the data using those
rules.

.. note::

   :cpp:func:`~orcus::orcus_json::detect_map_definition` also appends the
   necessary sheets to the document automatically — one per detected range —
   so there is no need to call :cpp:func:`~orcus::orcus_json::append_sheet`
   manually.

Dumping the first sheet:

.. literalinclude:: ../../../doc_example/json/json_mapping_autodetect.cpp
   :language: C++
   :start-after: //!code-start: simple-dump
   :end-before: //!code-end: simple-dump
   :dedent: 4

produces:

.. code-block:: none

   rows: 9  cols: 5
   +---------+-------------+-------------+-------------+---------+
   | name    | type        | distance-au | diameter-km | moons   |
   +---------+-------------+-------------+-------------+---------+
   | Mercury | Terrestrial | 0.39 [v]    | 4879 [v]    | 0 [v]   |
   +---------+-------------+-------------+-------------+---------+
   | Venus   | Terrestrial | 0.72 [v]    | 12104 [v]   | 0 [v]   |
   +---------+-------------+-------------+-------------+---------+
   | Earth   | Terrestrial | 1 [v]       | 12742 [v]   | 1 [v]   |
   +---------+-------------+-------------+-------------+---------+
   | Mars    | Terrestrial | 1.52 [v]    | 6779 [v]    | 2 [v]   |
   +---------+-------------+-------------+-------------+---------+
   | Jupiter | Gas Giant   | 5.2 [v]     | 139820 [v]  | 95 [v]  |
   +---------+-------------+-------------+-------------+---------+
   | Saturn  | Gas Giant   | 9.58 [v]    | 116460 [v]  | 146 [v] |
   +---------+-------------+-------------+-------------+---------+
   | Uranus  | Ice Giant   | 19.22 [v]   | 50724 [v]   | 28 [v]  |
   +---------+-------------+-------------+-------------+---------+
   | Neptune | Ice Giant   | 30.05 [v]   | 49244 [v]   | 16 [v]  |
   +---------+-------------+-------------+-------------+---------+

The column headers are taken directly from the JSON object keys, and the
column order matches the order in which the keys appear in the source
document.

Inspecting and editing the map definition
-----------------------------------------

:cpp:func:`~orcus::orcus_json::detect_map_definition` commits the mapping
immediately.  When you need to inspect or adjust the detected rules before
importing, use :cpp:func:`~orcus::orcus_json::write_map_definition` instead.
It performs the same structure analysis but serializes the result to a JSON
string rather than applying it:

.. literalinclude:: ../../../doc_example/json/json_mapping_autodetect.cpp
   :language: C++
   :start-after: //!code-start: write-map-def
   :end-before: //!code-end: write-map-def
   :dedent: 4

The map definition is written as a single compact line.  The following is its
pretty-printed form for readability:

.. code-block:: JSON

   {
     "sheets": ["range-0"],
     "ranges": [
       {
         "sheet": "range-0",
         "row": 0,
         "column": 0,
         "row-header": true,
         "fields": [
           {"path": "$[*]['name']"},
           {"path": "$[*]['type']"},
           {"path": "$[*]['distance-au']"},
           {"path": "$[*]['diameter-km']"},
           {"path": "$[*]['moons']"}
         ],
         "row-groups": [
           {"path": "$"}
         ]
       }
     ]
   }

The string can be edited freely at this point — fields can be removed,
reordered, or relabelled via an optional ``"label"`` key, sheet names can
be changed, and unwanted ranges can be dropped entirely.

Once any edits are complete, load the definition and import the data:

.. literalinclude:: ../../../doc_example/json/json_mapping_autodetect.cpp
   :language: C++
   :start-after: //!code-start: read-map-def
   :end-before: //!code-end: read-map-def
   :dedent: 4

:cpp:func:`~orcus::orcus_json::read_map_definition` parses the map definition
JSON, appends the necessary sheets to the document, and populates the internal
mapping rules — exactly as if the rules had been set up manually.
:cpp:func:`~orcus::orcus_json::read_stream` then applies those rules to import
the source document.
