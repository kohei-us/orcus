
Types and Interfaces
====================


Global Interface
----------------

.. doxygenclass:: orcus::iface::import_filter
   :members:

.. doxygenclass:: orcus::iface::document_dumper
   :members:


Spreadsheet Interface
---------------------

import_array_formula
^^^^^^^^^^^^^^^^^^^^

.. doxygenclass:: orcus::spreadsheet::iface::import_array_formula
   :members:

import_auto_filter
^^^^^^^^^^^^^^^^^^

.. doxygenclass:: orcus::spreadsheet::iface::import_auto_filter
   :members:

import_conditional_format
^^^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenclass:: orcus::spreadsheet::iface::import_conditional_format
   :members:

import_data_table
^^^^^^^^^^^^^^^^^

.. doxygenclass:: orcus::spreadsheet::iface::import_data_table
   :members:

import_factory
^^^^^^^^^^^^^^

.. doxygenclass:: orcus::spreadsheet::iface::import_factory
   :members:

import_formula
^^^^^^^^^^^^^^

.. doxygenclass:: orcus::spreadsheet::iface::import_formula
   :members:

import_global_settings
^^^^^^^^^^^^^^^^^^^^^^

.. doxygenclass:: orcus::spreadsheet::iface::import_global_settings
   :members:

import_named_expression
^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenclass:: orcus::spreadsheet::iface::import_named_expression
   :members:

import_pivot_cache_definition
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenclass:: orcus::spreadsheet::iface::import_pivot_cache_definition
   :members:

import_pivot_cache_records
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenclass:: orcus::spreadsheet::iface::import_pivot_cache_records
   :members:

import_reference_resolver
^^^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenclass:: orcus::spreadsheet::iface::import_reference_resolver
   :members:

import_shared_strings
^^^^^^^^^^^^^^^^^^^^^

.. doxygenclass:: orcus::spreadsheet::iface::import_shared_strings
   :members:

import_sheet
^^^^^^^^^^^^

.. doxygenclass:: orcus::spreadsheet::iface::import_sheet
   :members:

import_sheet_properties
^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenclass:: orcus::spreadsheet::iface::import_sheet_properties
   :members:

import_sheet_view
^^^^^^^^^^^^^^^^^

.. doxygenclass:: orcus::spreadsheet::iface::import_sheet_view
   :members:

import_styles
^^^^^^^^^^^^^

.. doxygenclass:: orcus::spreadsheet::iface::import_styles
   :members:

import_table
^^^^^^^^^^^^

.. doxygenclass:: orcus::spreadsheet::iface::import_table
   :members:

export_factory
^^^^^^^^^^^^^^

.. doxygenclass:: orcus::spreadsheet::iface::export_factory
   :members:

export_sheet
^^^^^^^^^^^^

.. doxygenclass:: orcus::spreadsheet::iface::export_sheet
   :members:


Spreadsheet Types
-----------------

Types
^^^^^

.. doxygentypedef:: orcus::spreadsheet::row_t
.. doxygentypedef:: orcus::spreadsheet::col_t
.. doxygentypedef:: orcus::spreadsheet::sheet_t
.. doxygentypedef:: orcus::spreadsheet::color_elem_t
.. doxygentypedef:: orcus::spreadsheet::col_width_t
.. doxygentypedef:: orcus::spreadsheet::row_height_t
.. doxygentypedef:: orcus::spreadsheet::pivot_cache_id_t


Structs
^^^^^^^

.. doxygenstruct:: orcus::spreadsheet::underline_attrs_t
   :members:

.. doxygenstruct:: orcus::spreadsheet::address_t
   :members:

.. doxygenstruct:: orcus::spreadsheet::range_size_t
   :members:

.. doxygenstruct:: orcus::spreadsheet::range_t
   :members:

.. doxygenstruct:: orcus::spreadsheet::color_rgb_t
   :members:


Enums
^^^^^

.. doxygenenum:: orcus::spreadsheet::error_value_t
.. doxygenenum:: orcus::spreadsheet::border_direction_t
.. doxygenenum:: orcus::spreadsheet::border_style_t
.. doxygenenum:: orcus::spreadsheet::fill_pattern_t
.. doxygenenum:: orcus::spreadsheet::strikethrough_style_t
.. doxygenenum:: orcus::spreadsheet::strikethrough_type_t
.. doxygenenum:: orcus::spreadsheet::strikethrough_width_t
.. doxygenenum:: orcus::spreadsheet::strikethrough_text_t
.. doxygenenum:: orcus::spreadsheet::formula_grammar_t
.. doxygenenum:: orcus::spreadsheet::formula_t
.. doxygenenum:: orcus::spreadsheet::underline_t
.. doxygenenum:: orcus::spreadsheet::underline_width_t
.. doxygenenum:: orcus::spreadsheet::underline_mode_t
.. doxygenenum:: orcus::spreadsheet::underline_type_t
.. doxygenenum:: orcus::spreadsheet::hor_alignment_t
.. doxygenenum:: orcus::spreadsheet::ver_alignment_t
.. doxygenenum:: orcus::spreadsheet::data_table_type_t
.. doxygenenum:: orcus::spreadsheet::totals_row_function_t
.. doxygenenum:: orcus::spreadsheet::conditional_format_t
.. doxygenenum:: orcus::spreadsheet::condition_operator_t
.. doxygenenum:: orcus::spreadsheet::condition_type_t
.. doxygenenum:: orcus::spreadsheet::condition_date_t
.. doxygenenum:: orcus::spreadsheet::databar_axis_t
.. doxygenenum:: orcus::spreadsheet::pivot_cache_group_by_t


Spreadsheet Functions
---------------------

.. doxygenfunction:: orcus::spreadsheet::get_default_column_width
.. doxygenfunction:: orcus::spreadsheet::get_default_row_height
.. doxygenfunction:: orcus::spreadsheet::to_totals_row_function_enum
.. doxygenfunction:: orcus::spreadsheet::to_pivot_cache_group_by_enum
.. doxygenfunction:: orcus::spreadsheet::to_error_value_enum
.. doxygenfunction:: orcus::spreadsheet::to_color_rgb
