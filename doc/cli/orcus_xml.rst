orcus-xml
=========

Usage
-----

.. code-block::

   orcus-xml [OPTIONS] FILE

Options
-------

- ``-h [ --help ]``

  Print this help.

- ``--mode arg``

  Mode of operation. Select one of the following options: dump, map, map-gen, structure, or transform.

- ``-m [ --map ] arg``

  Path to the map file. A map file is required for all modes except for the structure mode.

- ``-o [ --output ] arg``

  Path to either an output directory, or an output file.

- ``-f [ --output-format ] arg``

  Specify the output format. Supported format types are:
  
    - check - Flat format that fully encodes document content. Suitable for automated testing.
    - csv - CSV format.
    - debug-state - This format dumps the internal state of the document in detail, useful for debugging.
    - flat - Flat text format that displays document content in grid.
    - html - HTML format.
    - json - JSON format.
    - none - No output to be generated. Maybe useful during development.
    - xml - This format is currently unsupported.
    - yaml - This format is currently unsupported.

