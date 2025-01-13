
Using the low-level parser
==========================

The following code snippet shows how to use the low-level :cpp:class:`~orcus::json_parser`
class by providing an own handler class and passing it as a template argument:

.. literalinclude:: ../../../doc_example/json_parser_1.cpp
   :language: C++

The parser constructor expects the char array, its length, and the handler
instance.  The base handler class :cpp:class:`~orcus::json_handler` implements
all required handler methods.  By inheriting from it, you only need to
implement the handler methods you need.  In this example, we are only
implementing the :cpp:func:`~orcus::json_handler::object_key`,
:cpp:func:`~orcus::json_handler::string`, and :cpp:func:`~orcus::json_handler::number`
methods to process object key values, string values and numeric values,
respectively.  Refer to the :cpp:class:`~orcus::json_handler` class definition
for all available handler methods.

Executing this code will generate the following output:

.. code-block:: text

    JSON string: {"key1": [1,2,3,4,5], "key2": 12.3}
    object key: key1
    number: 1
    number: 2
    number: 3
    number: 4
    number: 5
    object key: key2
    number: 12.3
