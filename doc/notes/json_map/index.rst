
Mapping JSON to spreadsheet
===========================

This tutorial covers how to map JSON document to a spreadsheet document, very
similar to what we covered in :ref:`this tutorial <mapping-xml-to-ss>`
where we illustrated how to map XML document to a spreadsheet document.

Throughout this tutorial, we will be using :download:`this sample JSON document <example.json>`
to illustrate how to achieve it using the ``orcus-json`` command.

Let's first take a look at the sample JSON document:

.. code-block::

    [
        {
            "id": 1,
            "name": [
                "Tab",
                "Limpenny"
            ],
            "active": true,
            "gender": "Male",
            "language": "Kazakh"
        },
        {
            "id": 2,
            "name": [
                "Manda",
                "Hadgraft"
            ],
            "active": false,
            "gender": "Female",
            "language": "Bislama"
        },
        {
            "id": 3,
            "name": [
                "Mickie",
                "Boreham"
            ],
            "active": false,
            "gender": "Male",
            "language": "Swahili"
        },

        ...

This is essentially the same content as the XML sample document we used in the
:ref:`last tutorial <mapping-xml-to-ss>` but re-formatted in JSON.

Let run the following command:

.. code-block::

    orcus-json --mode structure example.json

to analyze the structure of this JSON document.  The command will generate the
following output:

.. code-block::

    $array[20].object(*)['active'].value
    $array[20].object(*)['gender'].value
    $array[20].object(*)['id'].value
    $array[20].object(*)['language'].value
    $array[20].object(*)['name'].array[2].value[0,1]

This structure output resembles a variant of JSONPath but some modifications
are applied.  It has the following characteristics:

* The ``$`` symbol represents the root of the structure.
* Array node takes the form of either ``array[N]``, where the value of ``N``
  represents the number of elements.
* Object node takes the form of ``object['key']``.
* Value node, which is always a leaf node, is represented by ``value`` except
  when the leaf node is an array containing values, it takes the form of ``value[0,1,2,...]``.
* The ``.`` symbols represent the node boundaries.
* The ``(*)`` symbols represent recurring nodes, which can be either array or
  object.
