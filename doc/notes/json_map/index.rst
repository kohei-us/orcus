
Mapping JSON to spreadsheet
===========================

This tutorial covers how to map JSON document to a spreadsheet document, very
similar to what we covered in :ref:`this tutorial <mapping-xml-to-ss>`
where we illustrated how to map XML document to a spreadsheet document.

Throughout this tutorial, we will be using :download:`this sample JSON document <example.json>`
to illustrate how to achieve it using the ``orcus-json`` command.  The structure
of this tutorial will be similar to the structure of the XML mapping counterpart,
since the steps are very similar.

Examining the structure of the input JSON document
--------------------------------------------------

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

Auto-mapping the JSON document
------------------------------

Let's map this JSON document to a spreadsheet document by running:

.. code-block::

    orcus-json --mode map -o out -f flat example.json

This is very similar to what we did in the XML mapping tutorial, except that
the command used is ``orcus-json`` and the input file is ``example.json``.
This will create file named ``out/range-0.txt`` which contains the following:

.. code-block::

    ---
    Sheet name: range-0
    rows: 21  cols: 6
    +--------+-----------+-------------+-----------+--------+----------------+
    | id     | field 0   | field 1     | active    | gender | language       |
    +--------+-----------+-------------+-----------+--------+----------------+
    | 1 [v]  | Tab       | Limpenny    | true [b]  | Male   | Kazakh         |
    +--------+-----------+-------------+-----------+--------+----------------+
    | 2 [v]  | Manda     | Hadgraft    | false [b] | Female | Bislama        |
    +--------+-----------+-------------+-----------+--------+----------------+
    | 3 [v]  | Mickie    | Boreham     | false [b] | Male   | Swahili        |
    +--------+-----------+-------------+-----------+--------+----------------+
    | 4 [v]  | Celinka   | Brookfield  | false [b] | Female | Gagauz         |
    +--------+-----------+-------------+-----------+--------+----------------+
    | 5 [v]  | Muffin    | Bleas       | false [b] | Female | Hiri Motu      |
    +--------+-----------+-------------+-----------+--------+----------------+
    | 6 [v]  | Jackelyn  | Crumb       | false [b] | Female | Northern Sotho |
    +--------+-----------+-------------+-----------+--------+----------------+
    | 7 [v]  | Tessie    | Hollingsbee | true [b]  | Female | Fijian         |
    +--------+-----------+-------------+-----------+--------+----------------+
    | 8 [v]  | Yank      | Wernham     | false [b] | Male   | Tok Pisin      |
    +--------+-----------+-------------+-----------+--------+----------------+
    | 9 [v]  | Brendan   | Lello       | true [b]  | Male   | Fijian         |
    +--------+-----------+-------------+-----------+--------+----------------+
    | 10 [v] | Arabel    | Rigg        | false [b] | Female | Kyrgyz         |
    +--------+-----------+-------------+-----------+--------+----------------+
    | 11 [v] | Carolann  | McElory     | false [b] | Female | Pashto         |
    +--------+-----------+-------------+-----------+--------+----------------+
    | 12 [v] | Gasparo   | Flack       | false [b] | Male   | Telugu         |
    +--------+-----------+-------------+-----------+--------+----------------+
    | 13 [v] | Eolanda   | Polendine   | false [b] | Female | Kashmiri       |
    +--------+-----------+-------------+-----------+--------+----------------+
    | 14 [v] | Brock     | McCaw       | false [b] | Male   | Tsonga         |
    +--------+-----------+-------------+-----------+--------+----------------+
    | 15 [v] | Wenda     | Espinas     | false [b] | Female | Bulgarian      |
    +--------+-----------+-------------+-----------+--------+----------------+
    | 16 [v] | Zachary   | Banane      | true [b]  | Male   | Persian        |
    +--------+-----------+-------------+-----------+--------+----------------+
    | 17 [v] | Sallyanne | Mengue      | false [b] | Female | Latvian        |
    +--------+-----------+-------------+-----------+--------+----------------+
    | 18 [v] | Elizabet  | Hoofe       | true [b]  | Female | Tswana         |
    +--------+-----------+-------------+-----------+--------+----------------+
    | 19 [v] | Alastair  | Hutchence   | true [b]  | Male   | Ndebele        |
    +--------+-----------+-------------+-----------+--------+----------------+
    | 20 [v] | Minor     | Worland     | true [b]  | Male   | Dutch          |
    +--------+-----------+-------------+-----------+--------+----------------+

Again, this is very similar to what we saw in the XML-mapping example.  Note
that cell values with ``[v]`` and ``[b]`` indicate numeric and boolean values,
respectively.  Cells with no suffixes are string cells.
