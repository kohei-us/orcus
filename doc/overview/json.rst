
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

::

    #include <orcus/json_document_tree.hpp>
    #include <orcus/config.hpp>
    #include <orcus/pstring.hpp>

    #include <cstdlib>
    #include <iostream>

    using namespace std;

    const char* json_string = "{"
    "   \"name\": \"John Doe\","
    "   \"occupation\": \"Software Engineer\","
    "   \"score\": [89, 67, 90]"
    "}";

    int main()
    {
        using node = orcus::json::node;

        orcus::json_config config; // Use default configuration.

        orcus::json::document_tree doc;
        doc.load(json_string, config);

        // Root is an object containing three key-value pairs.
        node root = doc.get_document_root();

        for (const orcus::pstring& key : root.keys())
        {
            node value = root.child(key);
            switch (value.type())
            {
                case orcus::json::node_t::string:
                    // string value
                    cout << key << ": " << value.string_value() << endl;
                break;
                case orcus::json::node_t::array:
                {
                    // array value
                    cout << key << ":" << endl;

                    for (size_t i = 0; i < value.child_count(); ++i)
                    {
                        node array_element = value.child(i);
                        cout << "  - " << array_element.numeric_value() << endl;
                    }
                }
                break;
                default:
                    ;
            }
        }

        return EXIT_SUCCESS;
    }

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
class by providing an own handler class and passing it as a template argument::

    #include <orcus/json_parser.hpp>
    #include <orcus/pstring.hpp>
    #include <cstring>
    #include <iostream>

    using namespace std;

    class json_parser_handler
    {
    public:
        void begin_parse()
        {
            cout << "begin parse" << endl;
        }

        void end_parse()
        {
            cout << "end parse" << endl;
        }

        void begin_array()
        {
            cout << "begin array" << endl;
        }

        void end_array()
        {
            cout << "end array" << endl;
        }

        void begin_object()
        {
            cout << "begin object" << endl;
        }

        void object_key(const char* p, size_t len, bool transient)
        {
            cout << "object key: " << orcus::pstring(p, len) << endl;
        }

        void end_object()
        {
            cout << "end object" << endl;
        }

        void boolean_true()
        {
            cout << "true" << endl;
        }

        void boolean_false()
        {
            cout << "false" << endl;
        }

        void null()
        {
            cout << "null" << endl;
        }

        void string(const char* p, size_t len, bool transient)
        {
            cout << "string: " << orcus::pstring(p, len) << endl;
        }

        void number(double val)
        {
            cout << "number: " << val << endl;
        }
    };

    int main()
    {
        const char* test_code = "{\"key1\": [1,2,3,4,5], \"key2\": 12.3}";
        size_t n = strlen(test_code);

        cout << "JSON string: " << test_code << endl;

        // Instantiate the parser with an own handler.
        json_parser_handler hdl;
        orcus::json_parser<json_parser_handler> parser(test_code, n, hdl);

        // Parse the string.
        parser.parse();

        return EXIT_SUCCESS;
    }

Executing this code will generate the following output:

.. code-block:: text

    JSON string: {"key1": [1,2,3,4,5], "key2": 12.3}
    begin parse
    begin object
    object key: key1
    begin array
    number: 1
    number: 2
    number: 3
    number: 4
    number: 5
    end array
    object key: key2
    number: 12.3
    end object
    end parse


Building a document tree directly
---------------------------------

You can also create and populate a JSON document tree directly without needing
to parse a JSON string.  This approach is ideal if you want to create a JSON
tree from scratch and export it as a string.  The following series of code
snippets demonstrate how to exactly build JSON document trees directly and
export their contents as JSON strings.

The first example shows how to initialize the tree with a simple array::

    orcus::json::document_tree doc = {
        1.0, 2.0, "string value", false, nullptr
    };

    std::cout << doc.dump() << std::endl;

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

If you need to build a array of arrays, do like the following::

    orcus::json::document_tree doc = {
        { true, false, nullptr },
        { 1.1, 2.2, "text" }
    };

    std::cout << doc.dump() << std::endl;

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
braces.  For example, the following code::

    orcus::json::document_tree doc = {
        { "key1", 1.2 },
        { "key2", "some text" },
    };

    std::cout << doc.dump() << std::endl;

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

As with arrays, nesting of objects is also supported.  The following code::

    orcus::json::document_tree doc = {
        { "parent1",
            {
                { "child1", true  },
                { "child2", false },
                { "child3", 123.4 },
            }
        },
        { "parent2", "not-nested" },
    };

    std::cout << doc.dump() << std::endl;

creates a root object having two key-value pairs one of which contains
another object having three key-value pairs, as evident in the following output
generated by the code:

.. code-block:: text

    {
        "parent1": {
            "child1": true,
            "child2": false,
            "child3": 123.4
        },
        "parent2": "not-nested"
    }

