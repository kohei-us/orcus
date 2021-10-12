
.. highlight:: cpp

Use orcus's spreadsheet document class
======================================

If you want to use orcus' :cpp:class:`~orcus::spreadsheet::document` as your
document store, you can use the :cpp:class:`~orcus::spreadsheet::import_factory`
class that orcus provides which already implements all necessary interfaces.
The example code shown below illustrates how to do this:

.. literalinclude:: ../../doc_example/spreadsheet_doc_1.cpp
   :language: C++

This example code loads a file saved in the Open Document Spreadsheet format
stored in a directory whose path is to be defined in the environment variable
named ``INPUTDIR``.  In this example, we don't check for the validity of ``INPUTDIR``
for bravity's sake.

The input file consists of the following content on its first sheet.

.. figure:: /_static/images/overview/doc-content.png

While it is not clear from this screenshot, cell C2 contains the formula
**CONCATENATE(A2, " ", B2)** to concatenate the content of A2 and B2 with a
space between them.  Cells C3 through C7 also contain similar formula
expressions.

Let's walk through this code step by step.  First, we need to instantiate the
document store.  Here we are using the concrete :cpp:class:`~orcus::spreadsheet::document`
class available in orcus.  Then immediately pass this document to the
:cpp:class:`~orcus::spreadsheet::import_factory` instance also from orcus::

    // Instantiate a document, and wrap it with a factory.
    spreadsheet::document doc;
    spreadsheet::import_factory factory(doc);

The next step is to create the loader instance and pass the factory to it::

    // Pass the factory to the document loader, and read the content from a file
    // to populate the document.
    orcus_ods loader(&factory);

In this example we are using the :cpp:class:`~orcus::orcus_ods` filter class
because the document we are loading is of Open Document Spreadsheet type, but
the process is the same for other document types, the only difference being
the name of the class.  Once the filter object is constructed, we'll simply
load the file by calling its :cpp:func:`~orcus::orcus_ods::read_file` method
and passing the path to the file as its argument::

    loader.read_file("/path/to/document.ods");

Once this call returns, the document has been fully populated.  What the rest
of the code does is access the content of the first row of the first sheet of
the document.  First, you need to get a reference to the internal cell value
store that we call *model context*::

    const ixion::model_context& model = doc.get_model_context();

Since the content of cell A1 is a string, to get the value you need to first
get the ID of the string::

    ixion::abs_address_t pos(0, 0, 0); // Set the cell position to A1.
    ixion::string_id_t str_id = model.get_string_identifier(pos);

Once you have the ID of the string, you can pass that to the model to get the
actual string value and print it to the standard output::

    const std::string* s = model.get_string(str_id);
    assert(s);
    std::cout << "A1: " << *s << std::endl;

Here we do assume that the string value exists for the given ID.  In case you
pass a string ID value to the :cpp:func:`get_string` method and there isn't a string
value associated with it, you'll get a null pointer instead.

The reason you need to take this 2-step process to get a string value is
because all the string values stored in the cells are pooled at the document
model level, and the cells themselves only store the ID values.

You may also have noticed that the types surrounding the :cpp:class:`ixion::model_context`
class are all in the :cpp:any:`ixion` namespace.  It is because orcus' own
:cpp:class:`~orcus::spreadsheet::document` class uses the formula engine from
the `ixion library <https://gitlab.com/ixion/ixion>`_ in order to calculate
the results of the formula cells inside the document, and the formula engine
requires all cell values to be stored in the :cpp:class:`ixion::model_context`
instance.

.. note:: The :cpp:class:`~orcus::spreadsheet::document` class in orcus uses
   the formula engine from the `ixion library <https://gitlab.com/ixion/ixion>`_
   to calculate the results of the formula cells stored in the document.

The rest of the code basically repeats the same process for cells B1 and C1::

    pos.column = 1; // Move to B1
    str_id = model.get_string_identifier(pos);
    s = model.get_string(str_id);
    assert(s);
    std::cout << "B1: " << *s << std::endl;

    pos.column = 2; // Move to C1
    str_id = model.get_string_identifier(pos);
    s = model.get_string(str_id);
    assert(s);
    std::cout << "C1: " << *s << std::endl;

You will see the following output when you compile and run this code:

.. code-block:: text

    A1: Number
    B1: String
    C1: Formula

Accessing the numeric cell values are a bit simpler since the values are
stored directly with the cells.  Using the document from the above example
code, the following code::

    for (spreadsheet::row_t row = 1; row <= 6; ++row)
    {
        ixion::abs_address_t pos(0, row, 0);
        double value = model.get_numeric_value(pos);
        std::cout << "A" << (pos.row+1) << ": " << value << std::endl;
    }

will access the cells from A2 through A7 and print out their numeric values.
You should see the following output when you run this code block:

.. code-block:: text

    A2: 1
    A3: 2
    A4: 3
    A5: 4
    A6: 5
    A7: 6

It's a bit more complex to handle formula cells.  Since each formula cell
contains two things: 1) the formula expression which is stored as tokens
internally, and 2) the cached result of the formula.  The following code
illustrates how to retrieve the cached formula results of cells C2 through
C7::

    for (spreadsheet::row_t row = 1; row <=6; ++row)
    {
        ixion::abs_address_t pos(0, row, 2); // Column C
        const ixion::formula_cell* fc = model.get_formula_cell(pos);
        assert(fc);

        // Get the formula cell results.
        const ixion::formula_result& result = fc->get_result_cache();

        // We already know the result is a string.
        ixion::string_id_t sid = result.get_string();
        const std::string* s = model.get_string(sid);
        assert(s);
        std::cout << "C" << (pos.row+1) << ": " << *s << std::endl;
    }

For each cell, this code first accesses the stored formula cell instance, get
a reference to its cached result, then obtain its string result value to print
it out to the standard output.  Running this block of code will yield the
following output:

.. code-block:: text

    C2: 1 Andy
    C3: 2 Bruce
    C4: 3 Charlie
    C5: 4 David
    C6: 5 Edward
    C7: 6 Frank

.. warning:: In production code, you should probabaly check the formula cell
             pointer which may be null in case the cell at the specified
             position is not a formula cell.
