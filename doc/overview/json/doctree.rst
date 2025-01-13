
Populating a document tree from JSON string
===========================================

The following code snippet shows an example of how to populate an instance of
:cpp:class:`~orcus::json::document_tree` from a JSON string, and navigate its
content tree afterward.

.. literalinclude:: ../../../doc_example/json_doc_1.cpp
   :language: C++

You'll see the following output when executing this code:

.. code-block:: text

    name: John Doe
    occupation: Software Engineer
    score:
      - 89
      - 67
      - 90
