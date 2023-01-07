
Spreadsheet document
====================

This section contains the API's related to the spreadsheet document storage, which
is provided by the ``liborcus-spreadsheet`` part of this library.


Document types
--------------

.. doxygenstruct:: orcus::spreadsheet::color_t
.. doxygenstruct:: orcus::spreadsheet::format_run
.. doxygentypedef:: orcus::spreadsheet::format_runs_t


Document
--------

.. doxygenclass:: orcus::spreadsheet::document
   :members:


Sheet
-----

.. doxygenclass:: orcus::spreadsheet::sheet
   :members:


Pivot table
-----------

.. doxygenstruct:: orcus::spreadsheet::pivot_cache_record_value_t
   :members:

.. doxygenstruct:: orcus::spreadsheet::pivot_cache_item_t
   :members:

.. doxygenstruct:: orcus::spreadsheet::pivot_cache_group_data_t
   :members:

.. doxygenstruct:: orcus::spreadsheet::pivot_cache_field_t
   :members:

.. doxygenclass:: orcus::spreadsheet::pivot_cache
   :members:

.. doxygenclass:: orcus::spreadsheet::pivot_collection
   :members:


Import factory
--------------

.. doxygenclass:: orcus::spreadsheet::import_factory
   :members:
