.. _ns-orcus-css:

namespace orcus::css
====================

Enum
----

combinator_t
^^^^^^^^^^^^
.. doxygenenum:: orcus::css::combinator_t

property_function_t
^^^^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::css::property_function_t

property_value_t
^^^^^^^^^^^^^^^^
.. doxygenenum:: orcus::css::property_value_t


Type aliases
------------

pseudo_class_t
^^^^^^^^^^^^^^
.. doxygentypedef:: orcus::css::pseudo_class_t

pseudo_element_t
^^^^^^^^^^^^^^^^
.. doxygentypedef:: orcus::css::pseudo_element_t


Constants
---------

pseudo_class_active
^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_active

pseudo_class_checked
^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_checked

pseudo_class_default
^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_default

pseudo_class_dir
^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_dir

pseudo_class_disabled
^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_disabled

pseudo_class_empty
^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_empty

pseudo_class_enabled
^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_enabled

pseudo_class_first
^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_first

pseudo_class_first_child
^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_first_child

pseudo_class_first_of_type
^^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_first_of_type

pseudo_class_focus
^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_focus

pseudo_class_fullscreen
^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_fullscreen

pseudo_class_hover
^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_hover

pseudo_class_in_range
^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_in_range

pseudo_class_indeterminate
^^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_indeterminate

pseudo_class_invalid
^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_invalid

pseudo_class_lang
^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_lang

pseudo_class_last_child
^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_last_child

pseudo_class_last_of_type
^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_last_of_type

pseudo_class_left
^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_left

pseudo_class_link
^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_link

pseudo_class_not
^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_not

pseudo_class_nth_child
^^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_nth_child

pseudo_class_nth_last_child
^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_nth_last_child

pseudo_class_nth_last_of_type
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_nth_last_of_type

pseudo_class_nth_of_type
^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_nth_of_type

pseudo_class_only_child
^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_only_child

pseudo_class_only_of_type
^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_only_of_type

pseudo_class_optional
^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_optional

pseudo_class_out_of_range
^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_out_of_range

pseudo_class_read_only
^^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_read_only

pseudo_class_read_write
^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_read_write

pseudo_class_required
^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_required

pseudo_class_right
^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_right

pseudo_class_root
^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_root

pseudo_class_scope
^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_scope

pseudo_class_target
^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_target

pseudo_class_valid
^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_valid

pseudo_class_visited
^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_class_visited

pseudo_element_after
^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_element_after

pseudo_element_backdrop
^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_element_backdrop

pseudo_element_before
^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_element_before

pseudo_element_first_letter
^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_element_first_letter

pseudo_element_first_line
^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_element_first_line

pseudo_element_selection
^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenvariable:: orcus::css::pseudo_element_selection


Functions
---------

pseudo_class_to_string
^^^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::css::pseudo_class_to_string(pseudo_class_t val)

to_property_function
^^^^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::css::to_property_function(std::string_view s)

to_pseudo_class
^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::css::to_pseudo_class(std::string_view s)

to_pseudo_element
^^^^^^^^^^^^^^^^^
.. doxygenfunction:: orcus::css::to_pseudo_element(std::string_view s)


Struct
------

hsla_color_t
^^^^^^^^^^^^
.. doxygenstruct:: orcus::css::hsla_color_t
   :members:

rgba_color_t
^^^^^^^^^^^^
.. doxygenstruct:: orcus::css::rgba_color_t
   :members:


Classes
-------

parser_base
^^^^^^^^^^^
.. doxygenclass:: orcus::css::parser_base
   :members:

