
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
recurring ``<record>`` elements each of which contains multiple fields.
