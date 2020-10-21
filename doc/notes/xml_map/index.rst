
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

We are using the ``flat`` format type which writes the data range of a sheet
in a human-readable grid output.

The mapped sheet content is the result of the automatic mapping of the original
XML document.  In automatic mapping, all attributes and element contents that
can be mapped as field values will be mapped, and the sheet name will be automatically
generated.

Although not applicable to this particular example, if the source XML document
contains multiple mappable ranges, they will get mapped to multiple sheets, one
sheet per range.

Custom-mapping using map file
-----------------------------

Generating map file
^^^^^^^^^^^^^^^^^^^

Automatic-mapping should work reasonably well in many cases, but sometime you
may need to customize how you map your data, and this section will go over how
you could do just that.

The short answer is that you will need to create a map definition file and pass
it to the ``orcus-xml`` command via ``-m`` or ``--map`` option.  The easiest
way to go about it is to have one generated for you.

Running the following command:

.. code-block::

    orcus-xml --mode map-gen -o map.xml example.xml

will generate a map file ``map.xml`` which contains the mapping definition based
on the auto-detected structure.  The content of ``map.xml`` generated from the
example XML document should look like this:

.. code-block:: XML

    <?xml version="1.0"?>
    <map xmlns="https://gitlab.com/orcus/orcus/xml-map-definition">
      <sheet name="range-0"/>
      <range sheet="range-0" row="0" column="0">
        <field path="/dataset/record/@id"/>
        <field path="/dataset/record/name/first"/>
        <field path="/dataset/record/name/last"/>
        <field path="/dataset/record/active"/>
        <field path="/dataset/record/gender"/>
        <field path="/dataset/record/language"/>
        <row-group path="/dataset/record"/>
      </range>
    </map>

Note that since the original map file content does not include any line breaks,
you may want to run it through an XML reformatting tool such as
`xmllint <http://xmlsoft.org/xmllint.html>`_ to "prettify" its content before
viewing.

Map file structure
^^^^^^^^^^^^^^^^^^

Hopefully the structure of the map file is self-explanatory, but let us go over
it a little.  The ``map`` element is the root element which contains one or
more ``sheet`` elements and one or more ``range`` elements.  The ``sheet``
elements specify how many sheets should be created in the spreadsheet model,
and what their names should be via their ``name`` attributes.  The ordering of
the ``sheet`` elements will reflect the ordering of the sheets in the final
spreadsheet document.

Each ``range`` element defines one mapped range of the source XML document, and
this element itself stores the top-left position of the range in the final
spreadsheet document via ``sheet``, ``row`` and ``column`` attributes.  The ``range``
element then contains one or more ``field`` elements, and one or more ``row-group``
elements.

Each ``field`` element defines one field within the mapped range and the path of
the value in the source XML document.  The path is expressed in XPath format.
The ordering of the ``field`` elements reflects the ordering of the field columns
in the final spreadsheet document.

Each ``row-group`` element defines the path of an anchor element.  For a simple
XML document such as our current example, you only need one ``row-group``
element.  But an XML document with more complex structure may need more than one
``row-group`` element to properly map nested recurring elements.

Modifying map file
^^^^^^^^^^^^^^^^^^

Let's make some changes to this map file.  First, the default sheet name ``range-0``
doesn't look very good, so we'll change it to ``My Data``.  Also, let's assume
we aren't really interested in the ID values or the "active" values (whatever
they may mean), so we'll drop those two fields.  Additionally, since we don't like
the default field labels, which are taken literally from the names of the corresponding
attributes or elements, we'll define custom field labels.  And finally, we'll add
two empty rows above the data range so that we can edit in some nice title afterward.

The modified map file will look like this:

.. code-block:: XML

    <?xml version="1.0"?>
    <map xmlns="https://gitlab.com/orcus/orcus/xml-map-definition">
      <sheet name="My Data"/>
      <range sheet="My Data" row="2" column="0">
        <field path="/dataset/record/name/first" label="First Name"/>
        <field path="/dataset/record/name/last" label="Last Name"/>
        <field path="/dataset/record/gender" label="Gender"/>
        <field path="/dataset/record/language" label="Language"/>
        <row-group path="/dataset/record"/>
      </range>
    </map>

We'll save this as ``map-modified.xml``, and pass it to the ``orcus-xml`` command
this time around like so:

.. code-block::

    ./src/orcus-xml --mode map -m map-modified.xml -o out -f flat example.xml

This will output the content of the sheet to ``out/My Data.txt``, which will
look like this:

.. code-block::

    ---
    Sheet name: My Data
    rows: 23  cols: 4
    +------------+-------------+--------+----------------+
    |            |             |        |                |
    +------------+-------------+--------+----------------+
    |            |             |        |                |
    +------------+-------------+--------+----------------+
    | First Name | Last Name   | Gender | Language       |
    +------------+-------------+--------+----------------+
    | Tab        | Limpenny    | Male   | Kazakh         |
    +------------+-------------+--------+----------------+
    | Manda      | Hadgraft    | Female | Bislama        |
    +------------+-------------+--------+----------------+
    | Mickie     | Boreham     | Male   | Swahili        |
    +------------+-------------+--------+----------------+
    | Celinka    | Brookfield  | Female | Gagauz         |
    +------------+-------------+--------+----------------+
    | Muffin     | Bleas       | Female | Hiri Motu      |
    +------------+-------------+--------+----------------+
    | Jackelyn   | Crumb       | Female | Northern Sotho |
    +------------+-------------+--------+----------------+
    | Tessie     | Hollingsbee | Female | Fijian         |
    +------------+-------------+--------+----------------+
    | Yank       | Wernham     | Male   | Tok Pisin      |
    +------------+-------------+--------+----------------+
    | Brendan    | Lello       | Male   | Fijian         |
    +------------+-------------+--------+----------------+
    | Arabel     | Rigg        | Female | Kyrgyz         |
    +------------+-------------+--------+----------------+
    | Carolann   | McElory     | Female | Pashto         |
    +------------+-------------+--------+----------------+
    | Gasparo    | Flack       | Male   | Telugu         |
    +------------+-------------+--------+----------------+
    | Eolanda    | Polendine   | Female | Kashmiri       |
    +------------+-------------+--------+----------------+
    | Brock      | McCaw       | Male   | Tsonga         |
    +------------+-------------+--------+----------------+
    | Wenda      | Espinas     | Female | Bulgarian      |
    +------------+-------------+--------+----------------+
    | Zachary    | Banane      | Male   | Persian        |
    +------------+-------------+--------+----------------+
    | Sallyanne  | Mengue      | Female | Latvian        |
    +------------+-------------+--------+----------------+
    | Elizabet   | Hoofe       | Female | Tswana         |
    +------------+-------------+--------+----------------+
    | Alastair   | Hutchence   | Male   | Ndebele        |
    +------------+-------------+--------+----------------+
    | Minor      | Worland     | Male   | Dutch          |
    +------------+-------------+--------+----------------+

The new output now only contains four fields, with custom labels at the top, and
now we have two empty rows above just like we intended.
