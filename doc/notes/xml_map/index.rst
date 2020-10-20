
Mapping XML to spreadsheet
==========================

In this tutorial, we will go over how to use the ``orcus-xml`` command to map an
XML content into a spreadsheet document.  We will be using :download:`this sample XML
document <example.xml>` throughout this tutorial.

Examining the structure of input XML document
---------------------------------------------

First, let's examine the general structure of this XML document:

.. code-block:: XML

    <?xml version="1.0" encoding="UTF-8"?>
    <dataset>
      <record id="1">
        <name>
          <first>Tab</first>
          <last>Limpenny</last>
        </name>
        <active>true</active>
        <gender>Male</gender>
        <language>Kazakh</language>
      </record>
      <record id="2">
        <name>
          <first>Manda</first>
          <last>Hadgraft</last>
        </name>
        <active>false</active>
        <gender>Female</gender>
        <language>Bislama</language>
      </record>
      <record id="3">

      ...


It starts with the ``<dataset>`` element as its root element, which contains
recurring ``<record>`` elements each of which contains multiple fields.  By
looking at each ``<record>`` element structure, you can easily infer how the
record content is structured.  You can also run ``orcus-xml`` in structure
mode in order to detect the structure of its content.

Running the following command

.. code-block::

    orcus-xml --mode structure example.xml

should generate the following output:

.. code-block::

    /dataset
    /dataset/record[*]
    /dataset/record[*]/@id
    /dataset/record[*]/name
    /dataset/record[*]/name/first
    /dataset/record[*]/name/last
    /dataset/record[*]/active
    /dataset/record[*]/gender
    /dataset/record[*]/language

This output lists the paths of all encountered "leaf node" items one item per
line, in order of occurrence.  Each path is expressed in a XPath-like format,
except for recurring "anchor" elements which are suffixed with the ``[*]``
symbols.  An anchor element in this context is defined as a recurring non-leaf
element that contains either an attribute or a leaf element.  You can think of
anchor elements as elements that define the individual record boundaries.

Auto-mapping the XML document
-----------------------------

Mapping this XML document to a spreadsheet document can be done by simply running
``orcus-xml`` in map mode.  You also need to specify the output format type and
the output directory in order to see the content of the mapped spreadsheet
document.  Running the command:

.. code-block::

    orcus-xml --mode map -f flat -o out example.xml

will create an output file named ``out/range-0.txt`` which contains the following:

.. code-block::

    ---
    Sheet name: range-0
    rows: 21  cols: 6
    +--------+-----------+-------------+--------+--------+----------------+
    | id     | first     | last        | active | gender | language       |
    +--------+-----------+-------------+--------+--------+----------------+
    | 1 [v]  | Tab       | Limpenny    | true   | Male   | Kazakh         |
    +--------+-----------+-------------+--------+--------+----------------+
    | 2 [v]  | Manda     | Hadgraft    | false  | Female | Bislama        |
    +--------+-----------+-------------+--------+--------+----------------+
    | 3 [v]  | Mickie    | Boreham     | false  | Male   | Swahili        |
    +--------+-----------+-------------+--------+--------+----------------+
    | 4 [v]  | Celinka   | Brookfield  | false  | Female | Gagauz         |
    +--------+-----------+-------------+--------+--------+----------------+
    | 5 [v]  | Muffin    | Bleas       | false  | Female | Hiri Motu      |
    +--------+-----------+-------------+--------+--------+----------------+
    | 6 [v]  | Jackelyn  | Crumb       | false  | Female | Northern Sotho |
    +--------+-----------+-------------+--------+--------+----------------+
    | 7 [v]  | Tessie    | Hollingsbee | true   | Female | Fijian         |
    +--------+-----------+-------------+--------+--------+----------------+
    | 8 [v]  | Yank      | Wernham     | false  | Male   | Tok Pisin      |
    +--------+-----------+-------------+--------+--------+----------------+
    | 9 [v]  | Brendan   | Lello       | true   | Male   | Fijian         |
    +--------+-----------+-------------+--------+--------+----------------+
    | 10 [v] | Arabel    | Rigg        | false  | Female | Kyrgyz         |
    +--------+-----------+-------------+--------+--------+----------------+
    | 11 [v] | Carolann  | McElory     | false  | Female | Pashto         |
    +--------+-----------+-------------+--------+--------+----------------+
    | 12 [v] | Gasparo   | Flack       | false  | Male   | Telugu         |
    +--------+-----------+-------------+--------+--------+----------------+
    | 13 [v] | Eolanda   | Polendine   | false  | Female | Kashmiri       |
    +--------+-----------+-------------+--------+--------+----------------+
    | 14 [v] | Brock     | McCaw       | false  | Male   | Tsonga         |
    +--------+-----------+-------------+--------+--------+----------------+
    | 15 [v] | Wenda     | Espinas     | false  | Female | Bulgarian      |
    +--------+-----------+-------------+--------+--------+----------------+
    | 16 [v] | Zachary   | Banane      | true   | Male   | Persian        |
    +--------+-----------+-------------+--------+--------+----------------+
    | 17 [v] | Sallyanne | Mengue      | false  | Female | Latvian        |
    +--------+-----------+-------------+--------+--------+----------------+
    | 18 [v] | Elizabet  | Hoofe       | true   | Female | Tswana         |
    +--------+-----------+-------------+--------+--------+----------------+
    | 19 [v] | Alastair  | Hutchence   | true   | Male   | Ndebele        |
    +--------+-----------+-------------+--------+--------+----------------+
    | 20 [v] | Minor     | Worland     | true   | Male   | Dutch          |
    +--------+-----------+-------------+--------+--------+----------------+

The mapped sheet content is the result of the automatic mapping of the original
XML document.  In automatic mapping, all attributes and element contents that
can be mapped as field values will be mapped, and the sheet name will be automatically
generated.

Although not applicable to this particular example, when the source XML document
contains multiple mappable ranges, they will get mapped to multiple sheets, one
sheet per range.
