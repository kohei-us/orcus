
Mapping XML to spreadsheet
==========================

In this tutorial, we will go over how to use the ``orcus-xml`` command to map an
XML content into a spreadsheet document.  We will be using :download:`this sample XML
document <example.xml>` throughout this tutorial.

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
record content is structured.  You can also run ``orcus-xml`` in the structure
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

