
.. highlight:: cpp

JSON
====

The JSON part of orcus consists of a low-level parser class that handles
parsing of JSON strings, and a high-level document class that stores parsed
JSON structures as a node tree.

There are two approaches to processing JSON strings using the orcus library.
One approach is to utilize the :cpp:class:`~orcus::json::document_tree` class
to load and populate the JSON structure tree via its
:cpp:func:`~orcus::json::document_tree::load()` method and traverse the tree
through its :cpp:func:`~orcus::json::document_tree::get_document_root()` method.
This approach is ideal if you want a quick way to parse and access the content
of a JSON document with minimal effort.

Another approach is to use the low-level :cpp:class:`~orcus::json_parser`
class directly by providing your own handler class to receive callbacks from
the parser.  This method requires a bit more effort on your part to provide
and populate your own data structure, but if you already have a data structure
to store the content of JSON, then this approach is ideal.  The
:cpp:class:`~orcus::json::document_tree` class internally uses
:cpp:class:`~orcus::json_parser` to parse JSON contents.


Populating a document tree from JSON string
-------------------------------------------

The following code snippet shows an example of how to populate an instance of
:cpp:class:`~orcus::json::document_tree` from a JSON string, and navigate its
content tree afterward.

.. literalinclude:: ../../doc_example/json_doc_1.cpp
   :language: C++

You'll see the following output when executing this code:

.. code-block:: text

    name: John Doe
    occupation: Software Engineer
    score:
      - 89
      - 67
      - 90


Using the low-level parser
--------------------------

The following code snippet shows how to use the low-level :cpp:class:`~orcus::json_parser`
class by providing an own handler class and passing it as a template argument:

.. literalinclude:: ../../doc_example/json_parser_1.cpp
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


Building a document tree directly
---------------------------------

You can also create and populate a JSON document tree directly without needing
to parse a JSON string.  This approach is ideal if you want to create a JSON
tree from scratch and export it as a string.  The following series of code
snippets demonstrate how to exactly build JSON document trees directly and
export their contents as JSON strings.

The first example shows how to initialize the tree with a simple array:

.. literalinclude:: ../../doc_example/json_doc_2.cpp
   :language: C++
   :start-after: //!code-start: root list
   :end-before: //!code-end: root list

You can simply specify the content of the array via initialization list and
assign it to the document.  The :cpp:func:`~orcus::json::document_tree::dump()`
method then turns the content into a single string instance, which looks like
the following:

.. code-block:: text

    [
        1,
        2,
        "string value",
        false,
        null
    ]

If you need to build a array of arrays, do like the following:

.. literalinclude:: ../../doc_example/json_doc_2.cpp
   :language: C++
   :start-after: //!code-start: list nested
   :end-before: //!code-end: list nested

This will create an array of two nested child arrays with three values each.
Dumping the content of the tree as a JSON string will produce something like
the following:

.. code-block:: text

    [
        [
            true,
            false,
            null
        ],
        [
            1.1,
            2.2,
            "text"
        ]
    ]

Creating an object can be done by nesting one of more key-value pairs, each of
which is surrounded by a pair of curly braces, inside another pair of curly
braces.  For example, the following code:

.. literalinclude:: ../../doc_example/json_doc_2.cpp
   :language: C++
   :start-after: //!code-start: list object
   :end-before: //!code-end: list object

produces the following output:

.. code-block:: text

    {
        "key1": 1.2,
        "key2": "some text"
    }

indicating that the tree consists of a single object having two key-value
pairs.

You may notice that this syntax is identical to the syntax for
creating an array of arrays as shown above.  In fact, in order for this to be
an object, each of the inner sequences must have exactly two values, and its
first value must be a string value.  Failing that, it will be interpreted as
an array of arrays.

As with arrays, nesting of objects is also supported.  The following code:

.. literalinclude:: ../../doc_example/json_doc_2.cpp
   :language: C++
   :start-after: //!code-start: list object 2
   :end-before: //!code-end: list object 2

creates a root object having two key-value pairs one of which contains
another object having three key-value pairs, as evident in the following output
generated by this code:

.. code-block:: text

    {
        "parent1": {
            "child1": true,
            "child2": false,
            "child3": 123.4
        },
        "parent2": "not-nested"
    }

There is one caveat that you need to be aware of because of this special
object creation syntax.  When you have a nested array that exactly contains
two values and the first value is a string value, you must explicitly declare
that as an array by using an :cpp:class:`~orcus::json::array` class instance.
For instance, this code:

.. literalinclude:: ../../doc_example/json_doc_2.cpp
   :language: C++
   :start-after: //!code-start: array ambiguous
   :end-before: //!code-end: array ambiguous

is intended to be an object containing an array.  However, because the supposed
inner array contains exactly two values and the first value is a string
value, which could be interpreted as a key-value pair for the outer object, it
ends up being too ambiguous and a :cpp:class:`~orcus::json::key_value_error`
exception gets thrown as a result.

To work around this ambiguity, you need to declare the inner array to be
explicit by using an :cpp:class:`~orcus::json::array` instance:

.. literalinclude:: ../../doc_example/json_doc_2.cpp
   :language: C++
   :start-after: //!code-start: array explicit
   :end-before: //!code-end: array explicit

This code now correctly generates a root object containing one key-value pair
whose value is an array:

.. code-block:: text

    {
        "array": [
            "one",
            987
        ]
    }

Similar ambiguity issue arises when you want to construct a tree consisting
only of an empty root object.  You may be tempted to write something like
this:

.. literalinclude:: ../../doc_example/json_doc_2.cpp
   :language: C++
   :start-after: //!code-start: object ambiguous a
   :end-before: //!code-end: object ambiguous a

However, this will result in leaving the tree entirely unpopulated i.e. the
tree will not even have a root node!  If you continue on and try to get a root
node from this tree, you'll get a :cpp:class:`~orcus::json::document_error`
thrown as a result.  If you inspect the error message stored in the exception:

.. literalinclude:: ../../doc_example/json_doc_2.cpp
   :language: C++
   :start-after: //!code-start: object ambiguous b
   :end-before: //!code-end: object ambiguous b

you will get

.. code-block:: text

    json::document_error: document tree is empty

giving you further proof that the tree is indeed empty!  The solution here is
to directly assign an instance of :cpp:class:`~orcus::json::object` to the
document tree, which will initialize the tree with an empty root object.  The
following code:

.. literalinclude:: ../../doc_example/json_doc_2.cpp
   :language: C++
   :start-after: //!code-start: object explicit 1
   :end-before: //!code-end: object explicit 1

will therefore generate

.. code-block:: text

    {
    }

You can also use the :cpp:class:`~orcus::json::object` class instances to
indicate empty objects anythere in the tree.  For instance, this code:

.. literalinclude:: ../../doc_example/json_doc_2.cpp
   :language: C++
   :start-after: //!code-start: object explicit 2
   :end-before: //!code-end: object explicit 2

is intended to create an array containing three empty objects as its elements,
and that's exactly what it does:

.. code-block:: text

    [
        {
        },
        {
        },
        {
        }
    ]

So far all the examples have shown how to initialize the document tree as the
tree itself is being constructed.  But our next example shows how to create
new key-value pairs to existing objects after the document tree instance has
been initialized.

.. literalinclude:: ../../doc_example/json_doc_2.cpp
   :language: C++
   :start-after: //!code-start: root object add child
   :end-before: //!code-end: root object add child

This code first initializes the tree with an empty object, then retrieves the
root empty object and assigns several key-value pairs to it.  When converting
the tree content to a string and inspecting it you'll see something like the
following:

.. code-block:: text

    {
        "child array": [
            1.1,
            1.2,
            true
        ],
        "child1": 1,
        "child3": [
            true,
            false
        ],
        "child2": "string",
        "child object": {
            "key1": 100,
            "key2": 200
        }
    }

The next example shows how to append values to an existing array after the
tree has been constructed.  Let's take a look at the code:

.. literalinclude:: ../../doc_example/json_doc_2.cpp
   :language: C++
   :start-after: //!code-start: root array add child
   :end-before: //!code-end: root array add child

Like the previous example, this code first initializes the tree but this time
with an empty array as its root, retrieves the root array, then appends
several values to it via its :cpp:func:`~orcus::json::node::push_back` method.

When you dump the content of this tree as a JSON string you'll get something
like this:

.. code-block:: text

    [
        -1.2,
        "string",
        true,
        null,
        {
            "key1": 1.1,
            "key2": 1.2
        }
    ]

