.. _ns-orcus:

namespace orcus
===============

Enum
----

character_set_t
^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::character_set_t

dump_format_t
^^^^^^^^^^^^^
.. doxygenenum:: orcus::dump_format_t

format_t
^^^^^^^^
.. doxygenenum:: orcus::format_t

length_unit_t
^^^^^^^^^^^^^
.. doxygenenum:: orcus::length_unit_t

string_escape_char_t
^^^^^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::string_escape_char_t


Type aliases
------------

css_properties_t
^^^^^^^^^^^^^^^^
.. doxygentypedef:: orcus::css_properties_t

css_pseudo_element_properties_t
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygentypedef:: orcus::css_pseudo_element_properties_t

xml_token_attrs_t
^^^^^^^^^^^^^^^^^
.. doxygentypedef:: orcus::xml_token_attrs_t

xml_token_t
^^^^^^^^^^^
.. doxygentypedef:: orcus::xml_token_t

xmlns_id_t
^^^^^^^^^^
.. doxygentypedef:: orcus::xmlns_id_t


Constants
---------

INDEX_NOT_FOUND
^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::INDEX_NOT_FOUND

XMLNS_UNKNOWN_ID
^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::XMLNS_UNKNOWN_ID

XML_UNKNOWN_TOKEN
^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::XML_UNKNOWN_TOKEN


Functions
---------

calc_logical_string_length
^^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::calc_logical_string_length(std::string_view s)

convert
^^^^^^^
.. doxygenfunction:: orcus::convert(double value, length_unit_t unit_from, length_unit_t unit_to)

create_filter
^^^^^^^^^^^^^
.. doxygenfunction:: orcus::create_filter(format_t type, spreadsheet::iface::import_factory *factory)

create_parse_error_output
^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::create_parse_error_output(std::string_view strm, std::ptrdiff_t offset)

decode_from_base64
^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::decode_from_base64(std::string_view base64)

detect
^^^^^^
.. doxygenfunction:: orcus::detect(std::string_view strm)

encode_to_base64
^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::encode_to_base64(const std::vector< uint8_t > &input)

get_dump_format_entries
^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::get_dump_format_entries()

get_string_escape_char_type
^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::get_string_escape_char_type(char c)

get_version_major
^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::get_version_major()

get_version_micro
^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::get_version_micro()

get_version_minor
^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::get_version_minor()

is_alpha
^^^^^^^^
.. doxygenfunction:: orcus::is_alpha(char c)

is_blank
^^^^^^^^
.. doxygenfunction:: orcus::is_blank(char c)

is_in
^^^^^
.. doxygenfunction:: orcus::is_in(char c, std::string_view allowed)

is_numeric
^^^^^^^^^^
.. doxygenfunction:: orcus::is_numeric(char c)

locate_first_different_char
^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::locate_first_different_char(std::string_view left, std::string_view right)

locate_line_with_offset
^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::locate_line_with_offset(std::string_view strm, std::ptrdiff_t offset)

parse_double_quoted_string
^^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::parse_double_quoted_string(const char *&p, std::size_t max_length, cell_buffer &buffer)

parse_integer
^^^^^^^^^^^^^
.. doxygenfunction:: orcus::parse_integer(const char *p, const char *p_end, long &value)

parse_numeric
^^^^^^^^^^^^^
.. doxygenfunction:: orcus::parse_numeric(const char *p, const char *p_end, double &value)

parse_single_quoted_string
^^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::parse_single_quoted_string(const char *&p, std::size_t max_length, cell_buffer &buffer)

parse_to_closing_double_quote
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::parse_to_closing_double_quote(const char *p, std::size_t max_length)

parse_to_closing_single_quote
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::parse_to_closing_single_quote(const char *p, std::size_t max_length)

to_bool
^^^^^^^
.. doxygenfunction:: orcus::to_bool(std::string_view s)

to_character_set
^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::to_character_set(std::string_view s)

to_double
^^^^^^^^^
.. doxygenfunction:: orcus::to_double(std::string_view s, const char **p_parse_ended=nullptr)

to_double_checked
^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::to_double_checked(std::string_view s)

to_dump_format_enum
^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::to_dump_format_enum(std::string_view s)

to_length
^^^^^^^^^
.. doxygenfunction:: orcus::to_length(std::string_view str)

to_long
^^^^^^^
.. doxygenfunction:: orcus::to_long(std::string_view s, const char **p_parse_ended=nullptr)

to_long_checked
^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::to_long_checked(std::string_view s)

trim
^^^^
.. doxygenfunction:: orcus::trim(std::string_view str)


Struct
------

config
^^^^^^
.. doxygenstruct:: orcus::config
   :members:

css_chained_simple_selector_t
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::css_chained_simple_selector_t
   :members:

css_property_value_t
^^^^^^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::css_property_value_t
   :members:

css_selector_t
^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::css_selector_t
   :members:

css_simple_selector_t
^^^^^^^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::css_simple_selector_t
   :members:

date_time_t
^^^^^^^^^^^
.. doxygenstruct:: orcus::date_time_t
   :members:

json_config
^^^^^^^^^^^
.. doxygenstruct:: orcus::json_config
   :members:

length_t
^^^^^^^^
.. doxygenstruct:: orcus::length_t
   :members:

line_with_offset
^^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::line_with_offset
   :members:

parse_error_value_t
^^^^^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::parse_error_value_t
   :members:

parse_quoted_string_state
^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::parse_quoted_string_state
   :members:

sax_ns_parser_attribute
^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::sax_ns_parser_attribute
   :members:

sax_ns_parser_element
^^^^^^^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::sax_ns_parser_element
   :members:

sax_parser_default_config
^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::sax_parser_default_config
   :members:

xml_declaration_t
^^^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::xml_declaration_t
   :members:

xml_name_t
^^^^^^^^^^
.. doxygenstruct:: orcus::xml_name_t
   :members:

xml_table_range_t
^^^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::xml_table_range_t
   :members:

xml_token_attr_t
^^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::xml_token_attr_t
   :members:

xml_token_element_t
^^^^^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::xml_token_element_t
   :members:

yaml_config
^^^^^^^^^^^
.. doxygenstruct:: orcus::yaml_config
   :members:

zip_file_entry_header
^^^^^^^^^^^^^^^^^^^^^
.. doxygenstruct:: orcus::zip_file_entry_header
   :members:


Classes
-------

cell_buffer
^^^^^^^^^^^
.. doxygenclass:: orcus::cell_buffer
   :members:

css_document_tree
^^^^^^^^^^^^^^^^^
.. doxygenclass:: orcus::css_document_tree
   :members:

css_handler
^^^^^^^^^^^
.. doxygenclass:: orcus::css_handler
   :members:

css_parser
^^^^^^^^^^
.. doxygenclass:: orcus::css_parser
   :members:

csv_handler
^^^^^^^^^^^
.. doxygenclass:: orcus::csv_handler
   :members:

csv_parser
^^^^^^^^^^
.. doxygenclass:: orcus::csv_parser
   :members:

file_content
^^^^^^^^^^^^
.. doxygenclass:: orcus::file_content
   :members:

general_error
^^^^^^^^^^^^^
.. doxygenclass:: orcus::general_error
   :members:

import_ods
^^^^^^^^^^
.. doxygenclass:: orcus::import_ods
   :members:

import_xlsx
^^^^^^^^^^^
.. doxygenclass:: orcus::import_xlsx
   :members:

interface_error
^^^^^^^^^^^^^^^
.. doxygenclass:: orcus::interface_error
   :members:

invalid_arg_error
^^^^^^^^^^^^^^^^^
.. doxygenclass:: orcus::invalid_arg_error
   :members:

invalid_map_error
^^^^^^^^^^^^^^^^^
.. doxygenclass:: orcus::invalid_map_error
   :members:

json_handler
^^^^^^^^^^^^
.. doxygenclass:: orcus::json_handler
   :members:

json_parser
^^^^^^^^^^^
.. doxygenclass:: orcus::json_parser
   :members:

json_structure_error
^^^^^^^^^^^^^^^^^^^^
.. doxygenclass:: orcus::json_structure_error
   :members:

malformed_xml_error
^^^^^^^^^^^^^^^^^^^
.. doxygenclass:: orcus::malformed_xml_error
   :members:

memory_content
^^^^^^^^^^^^^^
.. doxygenclass:: orcus::memory_content
   :members:

orcus_csv
^^^^^^^^^
.. doxygenclass:: orcus::orcus_csv
   :members:

orcus_gnumeric
^^^^^^^^^^^^^^
.. doxygenclass:: orcus::orcus_gnumeric
   :members:

orcus_json
^^^^^^^^^^
.. doxygenclass:: orcus::orcus_json
   :members:

orcus_ods
^^^^^^^^^
.. doxygenclass:: orcus::orcus_ods
   :members:

orcus_parquet
^^^^^^^^^^^^^
.. doxygenclass:: orcus::orcus_parquet
   :members:

orcus_xls_xml
^^^^^^^^^^^^^
.. doxygenclass:: orcus::orcus_xls_xml
   :members:

orcus_xlsx
^^^^^^^^^^
.. doxygenclass:: orcus::orcus_xlsx
   :members:

orcus_xml
^^^^^^^^^
.. doxygenclass:: orcus::orcus_xml
   :members:

parse_error
^^^^^^^^^^^
.. doxygenclass:: orcus::parse_error
   :members:

parser_base
^^^^^^^^^^^
.. doxygenclass:: orcus::parser_base
   :members:

sax_handler
^^^^^^^^^^^
.. doxygenclass:: orcus::sax_handler
   :members:

sax_ns_handler
^^^^^^^^^^^^^^
.. doxygenclass:: orcus::sax_ns_handler
   :members:

sax_ns_parser
^^^^^^^^^^^^^
.. doxygenclass:: orcus::sax_ns_parser
   :members:

sax_parser
^^^^^^^^^^
.. doxygenclass:: orcus::sax_parser
   :members:

sax_token_handler
^^^^^^^^^^^^^^^^^
.. doxygenclass:: orcus::sax_token_handler
   :members:

sax_token_handler_wrapper_base
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenclass:: orcus::sax_token_handler_wrapper_base
   :members:

sax_token_parser
^^^^^^^^^^^^^^^^
.. doxygenclass:: orcus::sax_token_parser
   :members:

string_pool
^^^^^^^^^^^
.. doxygenclass:: orcus::string_pool
   :members:

threaded_json_parser
^^^^^^^^^^^^^^^^^^^^
.. doxygenclass:: orcus::threaded_json_parser
   :members:

threaded_sax_token_parser
^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenclass:: orcus::threaded_sax_token_parser
   :members:

tokens
^^^^^^
.. doxygenclass:: orcus::tokens
   :members:

value_error
^^^^^^^^^^^
.. doxygenclass:: orcus::value_error
   :members:

xml_structure_error
^^^^^^^^^^^^^^^^^^^
.. doxygenclass:: orcus::xml_structure_error
   :members:

xml_structure_tree
^^^^^^^^^^^^^^^^^^
.. doxygenclass:: orcus::xml_structure_tree
   :members:

xml_writer
^^^^^^^^^^
.. doxygenclass:: orcus::xml_writer
   :members:

xmlns_context
^^^^^^^^^^^^^
.. doxygenclass:: orcus::xmlns_context
   :members:

xmlns_repository
^^^^^^^^^^^^^^^^
.. doxygenclass:: orcus::xmlns_repository
   :members:

xpath_error
^^^^^^^^^^^
.. doxygenclass:: orcus::xpath_error
   :members:

yaml_handler
^^^^^^^^^^^^
.. doxygenclass:: orcus::yaml_handler
   :members:

yaml_parser
^^^^^^^^^^^
.. doxygenclass:: orcus::yaml_parser
   :members:

zip_archive
^^^^^^^^^^^
.. doxygenclass:: orcus::zip_archive
   :members:

zip_archive_stream
^^^^^^^^^^^^^^^^^^
.. doxygenclass:: orcus::zip_archive_stream
   :members:

zip_archive_stream_blob
^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenclass:: orcus::zip_archive_stream_blob
   :members:

zip_archive_stream_fd
^^^^^^^^^^^^^^^^^^^^^
.. doxygenclass:: orcus::zip_archive_stream_fd
   :members:

zip_error
^^^^^^^^^
.. doxygenclass:: orcus::zip_error
   :members:


Child namespaces
----------------
.. toctree::
   :maxdepth: 1

   css/index.rst
   csv/index.rst
   detail/index.rst
   dom/index.rst
   iface/index.rst
   json/index.rst
   sax/index.rst
   spreadsheet/index.rst
   yaml/index.rst