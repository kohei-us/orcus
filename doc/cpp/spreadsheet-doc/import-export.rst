
Import and export
=================

The classes in this section can be viewed as the points of entry for initiating
import or export processes.

The :cpp:class:`~orcus::spreadsheet::import_factory` class wraps
:cpp:class:`~orcus::spreadsheet::document` as its destination storage then
gets passed to an import filter class that parses the content of an input file
and populates the destination document store.

The :cpp:class:`~orcus::spreadsheet::import_styles` class works similarly to
:cpp:class:`~orcus::spreadsheet::import_factory` in that it wraps
:cpp:class:`~orcus::spreadsheet::styles` as its destination storage then gets
passed to a styles import parser in order to get the destination store
populated.  Although this class is used by
:cpp:class:`~orcus::spreadsheet::import_factory` internally, it can also be
instantiated independently to allow loading of just the styles data.

The :cpp:class:`~orcus::spreadsheet::export_factory` also works in a similar
fashion, however; the export functionality of the orcus library is currently
very limited and should be considered experimental.  It is currently only used
by :cpp:class:`~orcus::orcus_xml` to export the content of a document which
was originally imported from an XML document.

.. warning::

   The export functionality of the orcus library is highly experimental.


Import factory
--------------

.. doxygenclass:: orcus::spreadsheet::import_factory
   :members:


Import styles
-------------

.. doxygenclass:: orcus::spreadsheet::import_styles
   :members:


Export factory
--------------

.. doxygenclass:: orcus::spreadsheet::export_factory
   :members:
