
Extract a subtree from a document tree
======================================

In this page, we are going to show how to use the
:cpp:class:`~orcus::json::subtree` class to extract a subtree structure from an
existing document tree using `JSONPath
<https://datatracker.ietf.org/doc/html/rfc9535>`_.  The :cpp:class:`~orcus::json::subtree` class
takes as the argumnets to its constructor:

* an existing document tree instance of :cpp:class:`~orcus::json::document_tree` type, and
* a JSONPath expression

in order to reference a subtree within the document tree.  Once the subtree is extracted,
you can use its :cpp:func:`~orcus::json::subtree::dump()` function to dump its
content as a JSON string.

First, let's include the headers we need in this example code:

.. literalinclude:: ../../../doc_example/json_subtree_1.cpp
   :language: C++
   :start-after: //!code-start: headers
   :end-before: //!code-end: headers

Both :cpp:class:`~orcus::json::document_tree` and
:cpp:class:`~orcus::json::subtree` classes are provided by the
``json_document_tree.hpp`` header, while the ``config.hpp`` header is to access
the :cpp:class:`orcus::json_config` struct type.

The following is the input JSON string we will be using in this example:

.. literalinclude:: ../../../doc_example/json_subtree_1.cpp
   :language: C++
   :start-after: //!code-start: input
   :end-before: //!code-end: input

It is defined as a raw string literal to make the value more human-readable.

First,  let's load this JSON string into an in-memory tree:

.. literalinclude:: ../../../doc_example/json_subtree_1.cpp
   :language: C++
   :start-after: //!code-start: load doc
   :end-before: //!code-end: load doc
   :dedent: 4

We can pass the input string defined above as its first argument.  The
:cpp:func:`~orcus::json::document_tree::load()` function also requires a
:cpp:struct:`~orcus::json_config` instance as its second argument to specify
some configuration parameters, but since we are not doing anything out of the
ordinary, a default-constructed one will suffice.


With the source JSON document loaded into memory, let's use the
:cpp:class:`orcus::json::subtree` class to extract the subtree whose root path
is located at the path ``$.profile.address`` of the original document:


.. literalinclude:: ../../../doc_example/json_subtree_1.cpp
   :language: C++
   :start-after: //!code-start: subtree 1
   :end-before: //!code-end: subtree 1
   :dedent: 4

Executing this code will generate the following output:

.. code-block:: text

   {
     "street": "123 Elm Street",
     "city": "Springfield",
     "state": "IL",
     "zipCode": "62704"
   }

One thing to note is that a :cpp:class:`~orcus::json::subtree` instance can only
reference the original document stored in
:cpp:class:`~orcus::json::document_tree`.  The user therefore must ensure that
the referencing instance will *not* outlive the original.  Accessing the
subtree instance after the original document has been destroyed causes an
undefined behavior.

.. note::

   You must ensure that the subtree instance will *not* outlive the original document
   tree instance.  Accessing the subtree instance after the original document tree
   instance has been destroyed causes an undefined behavior.

Let's use another example. This time, we will extract the subtree whose root path
is located at ``$.purchaseHistory[1].items[0]``:

.. literalinclude:: ../../../doc_example/json_subtree_1.cpp
   :language: C++
   :start-after: //!code-start: subtree 2
   :end-before: //!code-end: subtree 2
   :dedent: 4

This path includes object keys as well as array positions.  Executing this code
will generate the following output:

.. code-block:: text

   {
     "productId": "P125",
     "name": "Noise Cancelling Headphones",
     "quantity": 1,
     "price": 119.99
   }

It's important to note that, currently, :cpp:class:`~orcus::json::subtree` only
supports a small subset of the JSONPath specification, and does not fully
support expressions involving slicing or filtering.  It does, however, support
wildcards as the following example demonstrates:

.. literalinclude:: ../../../doc_example/json_subtree_1.cpp
   :language: C++
   :start-after: //!code-start: subtree 3
   :end-before: //!code-end: subtree 3
   :dedent: 4

Executing this code will generate the following output:

.. code-block:: text

   [
     [
       {
         "productId": "P123",
         "name": "Wireless Mouse",
         "quantity": 1,
         "price": 49.99
       },
       {
         "productId": "P124",
         "name": "Mechanical Keyboard",
         "quantity": 1,
         "price": 200
       }
     ],
     [
       {
         "productId": "P125",
         "name": "Noise Cancelling Headphones",
         "quantity": 1,
         "price": 119.99
       }
     ]
   ]

It extracted the ``items`` subtrees from both elements of the
``purchaseHistory`` array, and sequentially put them into a newly-created array
in order of occurrence.
