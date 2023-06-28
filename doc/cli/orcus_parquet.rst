orcus-parquet
=============

Usage
-----

.. code-block::

   orcus-parquet [options] FILE

The FILE must specify a path to an existing file.

Options
-------

- ``-h [ --help ]``

  Print this help.

- ``-d [ --debug ]``

  Turn on a debug mode and optionally specify a debug level in order to generate run-time debug outputs.

- ``-r [ --recalc ]``

  Re-calculate all formula cells after the documetn is loaded.

- ``-e [ --error-policy ] arg (=fail)``

  Specify whether to abort immediately when the loader fails to parse the first formula cell ('fail'), or skip the offending cells and continue ('skip').

- ``--dump-check``

  Dump the content to stdout in a special format used for content verification in automated tests.

- ``-o [ --output ] arg``

  Output directory path, or output file when --dump-check option is used.

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

- ``--row-size arg``

  Specify the number of maximum rows in each sheet.

