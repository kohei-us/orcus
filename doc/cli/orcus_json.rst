orcus-json
==========

Usage
-----

.. code-block::

   orcus-json [options] FILE

The FILE must specify the path to an existing file.

Options
-------

- ``-h [ --help ]``

  Print this help.

- ``--mode arg``

  Mode of operation. Select one of the following options: convert, map, map-gen, or structure.

- ``--resolve-refs``

  Resolve JSON references to external files.

- ``-o [ --output ] arg``

  Output file path.

- ``-f [ --output-format ] arg``

  Specify the format of output file. Supported format types are:
  
    - XML (xml)
    - JSON (json)
    - YAML (yaml)
    - flat tree dump (check)
    - no output (none)

- ``-m [ --map ] arg``

  Path to a map file. This parameter is only used for map mode, and it is required for map mode.

