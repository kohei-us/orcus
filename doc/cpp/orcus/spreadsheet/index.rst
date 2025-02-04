.. _ns-orcus-spreadsheet:

namespace orcus::spreadsheet
============================

Enum
----

auto_filter_node_op_t
^^^^^^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::auto_filter_node_op_t

auto_filter_op_t
^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::auto_filter_op_t

border_direction_t
^^^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::border_direction_t

border_style_t
^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::border_style_t

condition_date_t
^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::condition_date_t

condition_operator_t
^^^^^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::condition_operator_t

condition_type_t
^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::condition_type_t

conditional_format_t
^^^^^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::conditional_format_t

data_table_type_t
^^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::data_table_type_t

databar_axis_t
^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::databar_axis_t

error_value_t
^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::error_value_t

fill_pattern_t
^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::fill_pattern_t

formula_error_policy_t
^^^^^^^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::formula_error_policy_t

formula_grammar_t
^^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::formula_grammar_t

formula_ref_context_t
^^^^^^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::formula_ref_context_t

formula_t
^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::formula_t

hor_alignment_t
^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::hor_alignment_t

pane_state_t
^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::pane_state_t

pivot_cache_group_by_t
^^^^^^^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::pivot_cache_group_by_t

sheet_pane_t
^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::sheet_pane_t

strikethrough_style_t
^^^^^^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::strikethrough_style_t

strikethrough_text_t
^^^^^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::strikethrough_text_t

strikethrough_type_t
^^^^^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::strikethrough_type_t

strikethrough_width_t
^^^^^^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::strikethrough_width_t

totals_row_function_t
^^^^^^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::totals_row_function_t

underline_count_t
^^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::underline_count_t

underline_spacing_t
^^^^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::underline_spacing_t

underline_style_t
^^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::underline_style_t

underline_thickness_t
^^^^^^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::underline_thickness_t

ver_alignment_t
^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::ver_alignment_t

xf_category_t
^^^^^^^^^^^^^
.. doxygenenum:: orcus::spreadsheet::xf_category_t


Type aliases
------------

col_t
^^^^^
.. doxygentypedef:: orcus::spreadsheet::col_t

col_width_t
^^^^^^^^^^^
.. doxygentypedef:: orcus::spreadsheet::col_width_t

color_elem_t
^^^^^^^^^^^^
.. doxygentypedef:: orcus::spreadsheet::color_elem_t

format_runs_t
^^^^^^^^^^^^^
.. doxygentypedef:: orcus::spreadsheet::format_runs_t

pivot_cache_id_t
^^^^^^^^^^^^^^^^
.. doxygentypedef:: orcus::spreadsheet::pivot_cache_id_t

pivot_cache_indices_t
^^^^^^^^^^^^^^^^^^^^^
.. doxygentypedef:: orcus::spreadsheet::pivot_cache_indices_t

pivot_cache_items_t
^^^^^^^^^^^^^^^^^^^
.. doxygentypedef:: orcus::spreadsheet::pivot_cache_items_t

pivot_cache_record_t
^^^^^^^^^^^^^^^^^^^^
.. doxygentypedef:: orcus::spreadsheet::pivot_cache_record_t

row_height_t
^^^^^^^^^^^^
.. doxygentypedef:: orcus::spreadsheet::row_height_t

row_t
^^^^^
.. doxygentypedef:: orcus::spreadsheet::row_t

sheet_t
^^^^^^^
.. doxygentypedef:: orcus::spreadsheet::sheet_t

string_id_t
^^^^^^^^^^^
.. doxygentypedef:: orcus::spreadsheet::string_id_t


Functions
---------

get_default_column_width
^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::spreadsheet::get_default_column_width()

get_default_row_height
^^^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::spreadsheet::get_default_row_height()

to_color_rgb
^^^^^^^^^^^^
.. doxygenfunction:: orcus::spreadsheet::to_color_rgb(std::string_view s)

to_color_rgb_from_name
^^^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::spreadsheet::to_color_rgb_from_name(std::string_view s)

to_error_value_enum
^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::spreadsheet::to_error_value_enum(std::string_view s)

to_formula_error_policy
^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::spreadsheet::to_formula_error_policy(std::string_view s)

to_pivot_cache_group_by_enum
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::spreadsheet::to_pivot_cache_group_by_enum(std::string_view s)

to_rc_address
^^^^^^^^^^^^^
.. doxygenfunction:: orcus::spreadsheet::to_rc_address(const src_address_t &r)

to_rc_range
^^^^^^^^^^^
.. doxygenfunction:: orcus::spreadsheet::to_rc_range(const src_range_t &r)

to_totals_row_function_enum
^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::spreadsheet::to_totals_row_function_enum(std::string_view s)


Struct
------

address_t
^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::address_t
   :members:

auto_filter_t
^^^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::auto_filter_t
   :members:

border_attrs_t
^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::border_attrs_t
   :members:

border_t
^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::border_t
   :members:

cell_format_t
^^^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::cell_format_t
   :members:

cell_style_t
^^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::cell_style_t
   :members:

color_rgb_t
^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::color_rgb_t
   :members:

color_t
^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::color_t
   :members:

document_config
^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::document_config
   :members:

fill_t
^^^^^^
.. doxygenstruct:: orcus::spreadsheet::fill_t
   :members:

font_t
^^^^^^
.. doxygenstruct:: orcus::spreadsheet::font_t
   :members:

format_run_t
^^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::format_run_t
   :members:

frozen_pane_t
^^^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::frozen_pane_t
   :members:

import_factory_config
^^^^^^^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::import_factory_config
   :members:

number_format_t
^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::number_format_t
   :members:

pivot_cache_field_t
^^^^^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::pivot_cache_field_t
   :members:

pivot_cache_group_data_t
^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::pivot_cache_group_data_t
   :members:

pivot_cache_item_t
^^^^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::pivot_cache_item_t
   :members:

pivot_cache_record_value_t
^^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::pivot_cache_record_value_t
   :members:

protection_t
^^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::protection_t
   :members:

range_size_t
^^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::range_size_t
   :members:

range_t
^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::range_t
   :members:

split_pane_t
^^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::split_pane_t
   :members:

src_address_t
^^^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::src_address_t
   :members:

src_range_t
^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::src_range_t
   :members:

strikethrough_t
^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::strikethrough_t
   :members:

table_column_t
^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::table_column_t
   :members:

table_style_t
^^^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::table_style_t
   :members:

table_t
^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::table_t
   :members:

underline_t
^^^^^^^^^^^
.. doxygenstruct:: orcus::spreadsheet::underline_t
   :members:


Classes
-------

document
^^^^^^^^
.. doxygenclass:: orcus::spreadsheet::document
   :members:

export_factory
^^^^^^^^^^^^^^
.. doxygenclass:: orcus::spreadsheet::export_factory
   :members:

filter_item_set_t
^^^^^^^^^^^^^^^^^
.. doxygenclass:: orcus::spreadsheet::filter_item_set_t
   :members:

filter_item_t
^^^^^^^^^^^^^
.. doxygenclass:: orcus::spreadsheet::filter_item_t
   :members:

filter_node_t
^^^^^^^^^^^^^
.. doxygenclass:: orcus::spreadsheet::filter_node_t
   :members:

filter_value_t
^^^^^^^^^^^^^^
.. doxygenclass:: orcus::spreadsheet::filter_value_t
   :members:

filterable
^^^^^^^^^^
.. doxygenclass:: orcus::spreadsheet::filterable
   :members:

import_factory
^^^^^^^^^^^^^^
.. doxygenclass:: orcus::spreadsheet::import_factory
   :members:

import_styles
^^^^^^^^^^^^^
.. doxygenclass:: orcus::spreadsheet::import_styles
   :members:

pivot_cache
^^^^^^^^^^^
.. doxygenclass:: orcus::spreadsheet::pivot_cache
   :members:

pivot_collection
^^^^^^^^^^^^^^^^
.. doxygenclass:: orcus::spreadsheet::pivot_collection
   :members:

shared_strings
^^^^^^^^^^^^^^
.. doxygenclass:: orcus::spreadsheet::shared_strings
   :members:

sheet
^^^^^
.. doxygenclass:: orcus::spreadsheet::sheet
   :members:

sheet_view
^^^^^^^^^^
.. doxygenclass:: orcus::spreadsheet::sheet_view
   :members:

styles
^^^^^^
.. doxygenclass:: orcus::spreadsheet::styles
   :members:

tables
^^^^^^
.. doxygenclass:: orcus::spreadsheet::tables
   :members:

view
^^^^
.. doxygenclass:: orcus::spreadsheet::view
   :members:


Child namespaces
----------------
.. toctree::
   :maxdepth: 1

   iface/index.rst