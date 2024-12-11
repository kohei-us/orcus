orcus-json
==========

Help output
-----------

.. literalinclude:: ../_static/cli/orcus-json-help.txt
   :language: none

Supported modes
---------------

This command supports the following modes:

* convert
* lint
* map
* map-gen
* structure
* subtree

convert
^^^^^^^

The ``convert`` mode is used to perform simple conversion of a JSON document to
a different format that supports similar property-tree structure, such as XML
and YAML.

You can also "convert" a JSON document to JSON as well.  This may seem
pointless, but may be useful when the source JSON document contains JSON
references, and you want to resolve them by specifying the ``--resolve-refs``
option.  Note, however, that this command currently only supports resolving
references to external files via relative paths.  References to other resource
types may be added in the future.

lint
^^^^

The ``lint`` mode is used to reformat a JSON document optionally with a
different indent level.

map and map-gen
^^^^^^^^^^^^^^^

The ``map`` and ``map-gen`` modes are related, and are typically used together.
The ``map`` mode is used to map a JSON document to a spreadsheet document model
with a used-defined mapping rule, and the ``map-gen`` mode is used to
auto-generate a mapping rule based on the content of the source JSON document.

Refer to the :ref:`map-json-to-spreadsheet` section on a detailed example of how
to use the these modes to map a JSON document to a spreadsheet document model.

structure
^^^^^^^^^

The ``structure`` mode analyses the overall structure of the source JSON
document, and prints paths to all identified "leaf-node" values to standard
output.  Each path is expressed in a JSONPath-like format.  Refer to the
:ref:`map-json-to-spreadsheet` section for the detailed description of this
format.

subtree
^^^^^^^

the ``subtree`` mode allows you to extract a subtree structure from a JSON
document using a `JSONPath
<https://www.ietf.org/archive/id/draft-ietf-jsonpath-base-01.html>`_ expression
to specify the root of the subtree.  Note that this mode only supports a limited
subset of the full JSONPath expression syntax.  Both the dot notation and the
bracket notation are supported, as well as the wildcard symbol.


Example usage
-------------

Reformat JSON document
^^^^^^^^^^^^^^^^^^^^^^

You can use this command to re-format a JSON document by specifying ``--mode``
to ``lint``, with an optional indent level via ``--indent`` option.  The
following command reformats the input JSON file in a "prettified" format with the
indent level of 2:

.. code-block:: none

   orcus-json --mode lint --indent 2 path/to/input.json

The command writes the output to standard output by default, or you can specify
the ``--output`` option to have it written to a local file instead.  If you
don't specify the ``--indent`` option, it defaults to the indent level of 4.

Extract subtree from JSON document
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To extract a subtree structure from a JSON document, use the ``subtree`` mode with
the root of a desired subtree to extract:

.. code-block:: none

   orcus-json --mode subtree -p "$.store.fruits[3]" -i 2 path/to/source.json

This command traverses the original JSON document by following the ``store`` and ``fruits``
object keys and taking the 4th element of an array as the new root of the tree, and
writes it to stdout with the indent level of 2.

You can also use the wildcard expression to select all elements of an array, as
in the following example:

.. code-block:: none

   orcus-json --mode subtree -p "$.store.fruits[*].name" -i 2 path/to/source.json

This command traverses the original JSON document through the ``store`` and
``fruits`` objects keys as in the previous example, but selects all elements of
the array, and in each element only grabs the ``name`` key of the object below
it.

Convert JSON document to YAML
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To convert a JSON document to YAML, use the ``convert`` mode and specify the
output format to ``yaml`` in the ``--output-format`` option or simply ``-f``:

.. code-block:: none

   orcus-json --mode convert -f yaml path/to/input.json

The command then writes the converted document to standard output.  If you wish
to write it to a local file, use the ``--output`` option to specify the path to
output file the command can write to.
