
.. highlight:: cpp

Use a user-defined custom document class
========================================

In this section we will demonstrate how you can use orcus to populate your own
custom document model by implementing your own set of interface classes and
passing it to the orcus import filter.  The first example code shown below is
the *absolute* minimum that you need to implement in order for the orcus
filter to function properly:

.. literalinclude:: ../../doc_example/spreadsheet_doc_2.cpp
   :language: C++

Just like the example we used in the previous section, we are also loading a
document saved in the Open Document Spreadsheet format via
:cpp:class:`~orcus::orcus_ods`.  The document being loaded is named
multi-sheets.ods, and contains three sheets which are are named **'1st
Sheet'**, **'2nd Sheet'**, and **'3rd Sheet'** in this exact order.  When you
compile and execute the above code, you should get the following output:

.. code-block:: text

    append_sheet: sheet index: 0; sheet name: 1st Sheet
    append_sheet: sheet index: 1; sheet name: 2nd Sheet
    append_sheet: sheet index: 2; sheet name: 3rd Sheet

One primary role the import factory plays is to provide the orcus import
filter with the ability to create and insert a new sheet to the document.  As
illustrated in the above code, it also provides access to existing sheets by
its name or its position.  Every import factory implementation must be a
derived class of the :cpp:class:`orcus::spreadsheet::iface::import_factory`
interface base class.  At a minimum, it must implement

* the :cpp:func:`~orcus::spreadsheet::iface::import_factory::append_sheet`
  method which inserts a new sheet and return access to it,

* two variants of the :cpp:func:`~orcus::spreadsheet::iface::import_factory::get_sheet`
  method which returns access to an existing sheet, and

* the :cpp:func:`~orcus::spreadsheet::iface::import_factory::finalize` method
  which gets called exactly once at the very end of the import, to give the
  implementation a chance to perform post-import tasks.

in order for the code to be buildable.  Now, since all of the sheet accessor
methods return null pointers in this code, the import filter has no way of
populating the sheet data.  To actually receive the sheet data from the import
filter, you must have these methods return valid pointers to sheet accessors.
The next example shows how that can be done.


Implement sheet interface
-------------------------

In this section we will expand on the code in the previous section to
implement the sheet accessor interface, in order to receive cell values
in each individual sheet.  In this example, we will define a structure
to hold a cell value, and store them in a 2-dimensional array for each
sheet.  First, let's define the cell value structure:

.. literalinclude:: ../../doc_example/spreadsheet_doc_2_sheets_no_string_pool.cpp
   :language: C++
   :start-after: //!code-start: cell_value
   :end-before: //!code-end: cell_value

As we will be handling only three cell types i.e. empty, numeric, or string
cell type, this structure will work just fine.  We will also define a namespace
alias called ``ss`` for convenience.  This will be used in later code.

Next, we'll define a sheet class called ``my_sheet`` that stores the cell values
in a 2-dimensional array, and implements all required interfaces as a child class
of :cpp:class:`~orcus::spreadsheet::iface::import_sheet`.

At a minimum, the sheet accessor class must implement the following virtual
methods to satisfy the interface requirements of
:cpp:class:`~orcus::spreadsheet::iface::import_sheet`.

* :cpp:func:`~orcus::spreadsheet::iface::import_sheet::set_auto` - This is a
  setter method for a cell whose type is undetermined.  The implementor must
  determine the value type of this cell, from the raw string value of the
  cell.  This method is used when loading a CSV document, for instance.

* :cpp:func:`~orcus::spreadsheet::iface::import_sheet::set_string` - This is a
  setter method for a cell that stores a string value.  All cell string values
  are expectd to be pooled for the entire document, and this method only
  receives a string index into a centrally-managed string table.  The document
  model is expected to implement a central string table that can translate an
  index into its actual string value.

* :cpp:func:`~orcus::spreadsheet::iface::import_sheet::set_value` - This is a
  setter method for a cell that stores a numeric value.

* :cpp:func:`~orcus::spreadsheet::iface::import_sheet::set_bool` - This is a
  setter method for a cell that stores a boolean value.  Note that not all
  format types use this method, as some formats store boolean values as
  numeric values.

* :cpp:func:`~orcus::spreadsheet::iface::import_sheet::set_date_time` - This
  is a setter method for a cell that stores a date time value.  As with
  boolean value type, some format types may not use this method as they store
  date time values as numeric values, typically as days since epoch.

* :cpp:func:`~orcus::spreadsheet::iface::import_sheet::set_format` - This is a
  setter method for applying cell formats.  Just like the string values, cell
  format properties are expected to be stored in a document-wide cell format
  properties table, and this method only receives an index into the table.

* :cpp:func:`~orcus::spreadsheet::iface::import_sheet::get_sheet_size` - This
  method is expected to return the dimension of the sheet which the loader may
  need in some operations.

For now, we'll only implement
:cpp:func:`~orcus::spreadsheet::iface::import_sheet::set_string`,
:cpp:func:`~orcus::spreadsheet::iface::import_sheet::set_value`, and
:cpp:func:`~orcus::spreadsheet::iface::import_sheet::get_sheet_size`, and
leave the rest empty.

Here is the actual code for class ``my_sheet``:

.. literalinclude:: ../../doc_example/spreadsheet_doc_2_sheets_no_string_pool.cpp
   :language: C++
   :start-after: //!code-start: my_sheet
   :end-before: //!code-end: my_sheet

Note that this class receives its sheet index value from the caller upon
instantiation.  A sheet index is a 0-based value and represents its position
within the sheet collection.

Finally, we will modify the ``my_import_factory`` class to store and manage a
collection of ``my_sheet`` instances and to return the pointer value to a
correct sheet accessor instance as needed.

.. literalinclude:: ../../doc_example/spreadsheet_doc_2_sheets_no_string_pool.cpp
   :language: C++
   :start-after: //!code-start: my_import_factory
   :end-before: //!code-end: my_import_factory

Let's put it all together and run this code:

.. literalinclude:: ../../doc_example/spreadsheet_doc_2_sheets_no_string_pool.cpp
   :language: C++

We'll be loading the same document we loaded in the previous example, but this
time we will receive its cell values.  Let's go through each sheet one at a
time.

Data on the first sheet looks like this:

.. figure:: /_static/images/overview/multi-sheets-sheet1.png

It consists of 4 columns, with each column having a header row followed by
exactly ten rows of data.  The first and forth columns contain numeric data,
while the second and third columns contain string data.

When you run the above code to load this sheet, you'll get the following output:

.. code-block:: text

    (sheet: 0; row: 0; col: 0): string index = 0
    (sheet: 0; row: 0; col: 1): string index = 0
    (sheet: 0; row: 0; col: 2): string index = 0
    (sheet: 0; row: 0; col: 3): string index = 0
    (sheet: 0; row: 1; col: 0): value = 1
    (sheet: 0; row: 1; col: 1): string index = 0
    (sheet: 0; row: 1; col: 2): string index = 0
    (sheet: 0; row: 1; col: 3): value = 35
    (sheet: 0; row: 2; col: 0): value = 2
    (sheet: 0; row: 2; col: 1): string index = 0
    (sheet: 0; row: 2; col: 2): string index = 0
    (sheet: 0; row: 2; col: 3): value = 56
    (sheet: 0; row: 3; col: 0): value = 3
    (sheet: 0; row: 3; col: 1): string index = 0
    (sheet: 0; row: 3; col: 2): string index = 0
    (sheet: 0; row: 3; col: 3): value = 6
    (sheet: 0; row: 4; col: 0): value = 4
    (sheet: 0; row: 4; col: 1): string index = 0
    (sheet: 0; row: 4; col: 2): string index = 0
    (sheet: 0; row: 4; col: 3): value = 65
    (sheet: 0; row: 5; col: 0): value = 5
    (sheet: 0; row: 5; col: 1): string index = 0
    (sheet: 0; row: 5; col: 2): string index = 0
    (sheet: 0; row: 5; col: 3): value = 88
    (sheet: 0; row: 6; col: 0): value = 6
    (sheet: 0; row: 6; col: 1): string index = 0
    (sheet: 0; row: 6; col: 2): string index = 0
    (sheet: 0; row: 6; col: 3): value = 90
    (sheet: 0; row: 7; col: 0): value = 7
    (sheet: 0; row: 7; col: 1): string index = 0
    (sheet: 0; row: 7; col: 2): string index = 0
    (sheet: 0; row: 7; col: 3): value = 80
    (sheet: 0; row: 8; col: 0): value = 8
    (sheet: 0; row: 8; col: 1): string index = 0
    (sheet: 0; row: 8; col: 2): string index = 0
    (sheet: 0; row: 8; col: 3): value = 66
    (sheet: 0; row: 9; col: 0): value = 9
    (sheet: 0; row: 9; col: 1): string index = 0
    (sheet: 0; row: 9; col: 2): string index = 0
    (sheet: 0; row: 9; col: 3): value = 14
    (sheet: 0; row: 10; col: 0): value = 10
    (sheet: 0; row: 10; col: 1): string index = 0
    (sheet: 0; row: 10; col: 2): string index = 0
    (sheet: 0; row: 10; col: 3): value = 23

There is a couple of things worth pointing out.  First, the cell data
flows left to right first then top to bottom second.  Second, for this
particular sheet and for this particular format, implementing just the
two setter methods, namely
:cpp:func:`~orcus::spreadsheet::iface::import_sheet::set_string` and
:cpp:func:`~orcus::spreadsheet::iface::import_sheet::set_value` are
enough to receive all cell values.  However, we are getting a string
index value of 0 for all string cells.  This is because orcus expects
the backend document model to implement the shared strings interface
which is responsible for providing correct string indices to the import
filter, and we have not yet implemented one.  Let's fix that.


Implement shared strings interface
----------------------------------

The first thing to do is define some types:

.. literalinclude:: ../../doc_example/spreadsheet_doc_2_sheets_with_string_pool.cpp
   :language: C++
   :start-after: //!code-start: types
   :end-before: //!code-end: types

Here, we define ``ss_type`` to be the authoritative store for the shared
string values.  The string values will be stored as std::string type, and we
use std::deque here to avoid re-allocation of internal buffers as the size
of the container grows.

Another type we define is ``ss_hash_type``, which will be the hash map type
for storing string-to-index mapping entries.  Here, we are using std::string_view
instead of std::string so that we can simply reference the string values stored in
the first container.

The shared string interface is designed to handle both unformatted and
formatted string values.  The following two methods:

* :cpp:func:`~orcus::spreadsheet::iface::import_shared_strings::add`
* :cpp:func:`~orcus::spreadsheet::iface::import_shared_strings::append`

are for unformatted string values.  The
:cpp:func:`~orcus::spreadsheet::iface::import_shared_strings::add` method is
used when passing a string value that may or may not already exist in the
shared string pool.  The
:cpp:func:`~orcus::spreadsheet::iface::import_shared_strings::append` method,
on the other hand, is used only when the string value being passed is a
brand-new string not yet stored in the string pool.  When implementing the
:cpp:func:`~orcus::spreadsheet::iface::import_shared_strings::append` method,
you may skip checking for the existance of the string value in the pool before
inserting it.  Both of these methods are expected to return a positive integer
value as the index of the string being passed.

The following eight methods:

* :cpp:func:`~orcus::spreadsheet::iface::import_shared_strings::set_segment_bold`
* :cpp:func:`~orcus::spreadsheet::iface::import_shared_strings::set_segment_font`
* :cpp:func:`~orcus::spreadsheet::iface::import_shared_strings::set_segment_font_color`
* :cpp:func:`~orcus::spreadsheet::iface::import_shared_strings::set_segment_font_name`
* :cpp:func:`~orcus::spreadsheet::iface::import_shared_strings::set_segment_font_size`
* :cpp:func:`~orcus::spreadsheet::iface::import_shared_strings::set_segment_italic`
* :cpp:func:`~orcus::spreadsheet::iface::import_shared_strings::append_segment`
* :cpp:func:`~orcus::spreadsheet::iface::import_shared_strings::commit_segments`

are for receiving formatted string values.  Conceptually, a formatted string
consists of a series of multiple string segments, where each segment may have
different formatting attributes applied to it.  These ``set_segment_*``
methods are used to set the individual formatting attributes for the current
string segment, and the string value for the current segment is passed through
the
:cpp:func:`~orcus::spreadsheet::iface::import_shared_strings::append_segment`
call.  The order in which the ``set_segment_*`` methods are called is not
specified, and not all of them may be called, but they are guaranteed to be
called before the
:cpp:func:`~orcus::spreadsheet::iface::import_shared_strings::append_segment`
method gets called.  The implementation should keep a buffer to store the
formatting attributes for the current segment and apply each attribute to the
buffer as one of the ``set_segment_*`` methods gets called.  When the
:cpp:func:`~orcus::spreadsheet::iface::import_shared_strings::append_segment`
gets called, the implementation should apply the formatting attirbute set
currently in the buffer to the current segment, and reset the buffer for the
next segment.  When all of the string segments and their formatting attributes
are passed,
:cpp:func:`~orcus::spreadsheet::iface::import_shared_strings::commit_segments`
gets called, signaling the implementation that now it's time to commit the
string to the document model.

As we are going to ignore the formatting attributes in our current example,
the following code will do:

.. literalinclude:: ../../doc_example/spreadsheet_doc_2_sheets_with_string_pool.cpp
   :language: C++
   :start-after: //!code-start: my_shared_strings
   :end-before: //!code-end: my_shared_strings

Note that some import filters may use the
:cpp:func:`~orcus::spreadsheet::iface::import_shared_strings::append_segment`
and
:cpp:func:`~orcus::spreadsheet::iface::import_shared_strings::commit_segments`
combination even for unformatted strings.  Because of this, you still need to
implement these two methods even if raw string values are all you care about.

Note also that the container storing the string values is a reference.  The
source container will be owned by ``my_import_factory`` who will also be the
owner of the ``my_shared_strings`` instance.  Shown below is the modified
version of ``my_import_factory`` that provides the shared string interface:

.. literalinclude:: ../../doc_example/spreadsheet_doc_2_sheets_with_string_pool.cpp
   :language: C++
   :start-after: //!code-start: my_import_factory
   :end-before: //!code-end: my_import_factory

The shared string store is also passed to each sheet instance, and we'll use
that to fetch the string values from their respective string indices.

Let's put this all together:

.. literalinclude:: ../../doc_example/spreadsheet_doc_2_sheets_with_string_pool.cpp
   :language: C++

The sheet class is largely unchanged except for one thing; it now takes a
reference to the string pool and print the actual string value alongside the
string index associated with it.  When you execute this code, you'll see the
following output when loading the same sheet:

.. code-block:: text

    (sheet: 0; row: 0; col: 0): string index = 0 (ID)
    (sheet: 0; row: 0; col: 1): string index = 1 (First Name)
    (sheet: 0; row: 0; col: 2): string index = 2 (Last Name)
    (sheet: 0; row: 0; col: 3): string index = 3 (Age)
    (sheet: 0; row: 1; col: 0): value = 1
    (sheet: 0; row: 1; col: 1): string index = 5 (Thia)
    (sheet: 0; row: 1; col: 2): string index = 6 (Beauly)
    (sheet: 0; row: 1; col: 3): value = 35
    (sheet: 0; row: 2; col: 0): value = 2
    (sheet: 0; row: 2; col: 1): string index = 9 (Pepito)
    (sheet: 0; row: 2; col: 2): string index = 10 (Resun)
    (sheet: 0; row: 2; col: 3): value = 56
    (sheet: 0; row: 3; col: 0): value = 3
    (sheet: 0; row: 3; col: 1): string index = 13 (Emera)
    (sheet: 0; row: 3; col: 2): string index = 14 (Gravey)
    (sheet: 0; row: 3; col: 3): value = 6
    (sheet: 0; row: 4; col: 0): value = 4
    (sheet: 0; row: 4; col: 1): string index = 17 (Erinn)
    (sheet: 0; row: 4; col: 2): string index = 18 (Flucks)
    (sheet: 0; row: 4; col: 3): value = 65
    (sheet: 0; row: 5; col: 0): value = 5
    (sheet: 0; row: 5; col: 1): string index = 21 (Giusto)
    (sheet: 0; row: 5; col: 2): string index = 22 (Bambury)
    (sheet: 0; row: 5; col: 3): value = 88
    (sheet: 0; row: 6; col: 0): value = 6
    (sheet: 0; row: 6; col: 1): string index = 25 (Neall)
    (sheet: 0; row: 6; col: 2): string index = 26 (Scorton)
    (sheet: 0; row: 6; col: 3): value = 90
    (sheet: 0; row: 7; col: 0): value = 7
    (sheet: 0; row: 7; col: 1): string index = 29 (Ervin)
    (sheet: 0; row: 7; col: 2): string index = 30 (Foreman)
    (sheet: 0; row: 7; col: 3): value = 80
    (sheet: 0; row: 8; col: 0): value = 8
    (sheet: 0; row: 8; col: 1): string index = 33 (Shoshana)
    (sheet: 0; row: 8; col: 2): string index = 34 (Bohea)
    (sheet: 0; row: 8; col: 3): value = 66
    (sheet: 0; row: 9; col: 0): value = 9
    (sheet: 0; row: 9; col: 1): string index = 37 (Gladys)
    (sheet: 0; row: 9; col: 2): string index = 38 (Somner)
    (sheet: 0; row: 9; col: 3): value = 14
    (sheet: 0; row: 10; col: 0): value = 10
    (sheet: 0; row: 10; col: 1): string index = 41 (Ephraim)
    (sheet: 0; row: 10; col: 2): string index = 42 (Russell)
    (sheet: 0; row: 10; col: 3): value = 23

The string indices now increment nicely, and their respective string values
look correct.

Now, let's turn our attention to the second sheet, which contains formulas.
First, here is what the second sheet looks like:

.. figure:: /_static/images/overview/multi-sheets-sheet2.png

It contains a simple table extending from A1 to C9.  It consists of three
columns and the first row is a header row.  Cells in the the first and second
columns contain simple numbers and the third column contains formulas that
simply add the two numbers to the left of the same row.  When loading this
sheet using the last code we used above, you'll see the following output:

.. code-block:: text

    (sheet: 1; row: 0; col: 0): string index = 44 (X)
    (sheet: 1; row: 0; col: 1): string index = 45 (Y)
    (sheet: 1; row: 0; col: 2): string index = 46 (X + Y)
    (sheet: 1; row: 1; col: 0): value = 18
    (sheet: 1; row: 1; col: 1): value = 79
    (sheet: 1; row: 2; col: 0): value = 48
    (sheet: 1; row: 2; col: 1): value = 55
    (sheet: 1; row: 3; col: 0): value = 99
    (sheet: 1; row: 3; col: 1): value = 35
    (sheet: 1; row: 4; col: 0): value = 41
    (sheet: 1; row: 4; col: 1): value = 69
    (sheet: 1; row: 5; col: 0): value = 5
    (sheet: 1; row: 5; col: 1): value = 18
    (sheet: 1; row: 6; col: 0): value = 46
    (sheet: 1; row: 6; col: 1): value = 69
    (sheet: 1; row: 7; col: 0): value = 36
    (sheet: 1; row: 7; col: 1): value = 67
    (sheet: 1; row: 8; col: 0): value = 78
    (sheet: 1; row: 8; col: 1): value = 2

Everything looks fine except that the formula cells in C2:C9 are not loaded at
all.  This is because, in order to receive formula cell data, you must
implement the required :cpp:class:`~orcus::spreadsheet::iface::import_formula`
interface, which we will cover in the next section.


Implement formula interface
---------------------------

In this section we will extend the code from the previous section in order to
receive and process formula cell values from the sheet.  We will need to make
quite a few changes.  Let's go over this one thing at a time.  First, we are
adding a new cell value type ``formula``:

.. literalinclude:: ../../doc_example/spreadsheet_doc_2_sheets_with_formula.cpp
   :language: C++
   :start-after: //!code-start: cell_value_type
   :end-before: //!code-end: cell_value_type

which should not come as a surprise.

We are not making any change to the ``cell_value`` struct itself, but we are
re-using its ``index`` member for a formula cell value such that, if the cell
stores a formula, the index will refer to its actual formula data which will
be stored in a separate data store, much like how strings are stored
externally and referenced by their indices in the ``cell_value`` instances.

We are also adding a brand-new class called ``cell_grid``, to add an extra
layer over the raw cell value array:

.. literalinclude:: ../../doc_example/spreadsheet_doc_2_sheets_with_formula.cpp
   :language: C++
   :start-after: //!code-start: cell_grid
   :end-before: //!code-end: cell_grid

Each sheet instance will own one instance of ``cell_grid``, and the formula
interface class instance will hold a reference to it and use it to insert
formula cell values into it.  The same sheet instance will also hold a formula
value store, and pass its reference to the formula interface class.

The formula interface class must implement the following methods:

* :cpp:func:`~orcus::spreadsheet::iface::import_formula::set_position`
* :cpp:func:`~orcus::spreadsheet::iface::import_formula::set_formula`
* :cpp:func:`~orcus::spreadsheet::iface::import_formula::set_shared_formula_index`
* :cpp:func:`~orcus::spreadsheet::iface::import_formula::set_result_string`
* :cpp:func:`~orcus::spreadsheet::iface::import_formula::set_result_value`
* :cpp:func:`~orcus::spreadsheet::iface::import_formula::set_result_empty`
* :cpp:func:`~orcus::spreadsheet::iface::import_formula::set_result_bool`
* :cpp:func:`~orcus::spreadsheet::iface::import_formula::commit`

Depending on the type of a formula cell, and depending on the format of the
document, some methods may not be called.  The
:cpp:func:`~orcus::spreadsheet::iface::import_formula::set_position` method
always gets called regardless of the formula cell type, to specify the
position of the formula cell.  The
:cpp:func:`~orcus::spreadsheet::iface::import_formula::set_formula` gets
called for a formula cell that does not share its formula expression with any
other formula cells, or a formula cell that shares its formula expression with
a group of other formuls cells and is the primary cell of that group.  If it's
the primary cell of a grouped formula cells, the
:cpp:func:`~orcus::spreadsheet::iface::import_formula::set_shared_formula_index`
method also gets called to receive the identifier value of that group.  All
formula cells belonging to the same group receives the same identifier value
via
:cpp:func:`~orcus::spreadsheet::iface::import_formula::set_shared_formula_index`,
but only the primary cell of a group receives the formula expression string
via :cpp:func:`~orcus::spreadsheet::iface::import_formula::set_formula`.  The
rest of the methods -
:cpp:func:`~orcus::spreadsheet::iface::import_formula::set_result_string`,
:cpp:func:`~orcus::spreadsheet::iface::import_formula::set_result_value`,
:cpp:func:`~orcus::spreadsheet::iface::import_formula::set_result_empty` and
:cpp:func:`~orcus::spreadsheet::iface::import_formula::set_result_bool` - are
called to deliver the cached formula cell value when applicable.

The :cpp:func:`~orcus::spreadsheet::iface::import_formula::commit` method gets
called at the very end to let the implementation commit the formula cell data
to the backend document store.

Without further ado, here is the formula interface implementation that we will
use:

.. literalinclude:: ../../doc_example/spreadsheet_doc_2_sheets_with_formula.cpp
   :language: C++
   :start-after: //!code-start: my_formula
   :end-before: //!code-end: my_formula

and here is the defintion of the ``formula`` struct that stores a formula expression
string as well as its grammer type:

.. literalinclude:: ../../doc_example/spreadsheet_doc_2_sheets_with_formula.cpp
   :language: C++
   :start-after: //!code-start: formula
   :end-before: //!code-end: formula

Note that since we are loading a OpenDocument Spereadsheet file (.ods) which
does not support shared formulas, we do not need to handle the
:cpp:func:`~orcus::spreadsheet::iface::import_formula::set_shared_formula_index`
method.  Likewise, we are leaving the ``set_result_*`` methods unhandled for
now.

This interface class also stores references to ``cell_grid`` and
``std::vector<formula>`` instances, both of which are passed from the parent
sheet instance.

We also need to make a few changes to the sheet interface class to provide a formula interface
and add a formula value store:

.. literalinclude:: ../../doc_example/spreadsheet_doc_2_sheets_with_formula.cpp
   :language: C++
   :start-after: //!code-start: my_sheet
   :end-before: //!code-end: my_sheet

We've added the
:cpp:func:`~orcus::spreadsheet::iface::import_sheet::get_formula` method which
returns a pointer to the ``my_formula`` class instance defined above.  The
rest of the code is unchanged.

Now let's see what happens when loading the same sheet from the previous
section:

.. code-block:: text

    (sheet: 1; row: 0; col: 0): string index = 44 (X)
    (sheet: 1; row: 0; col: 1): string index = 45 (Y)
    (sheet: 1; row: 0; col: 2): string index = 46 (X + Y)
    (sheet: 1; row: 1; col: 0): value = 18
    (sheet: 1; row: 1; col: 1): value = 79
    (sheet: 1; row: 2; col: 0): value = 48
    (sheet: 1; row: 2; col: 1): value = 55
    (sheet: 1; row: 3; col: 0): value = 99
    (sheet: 1; row: 3; col: 1): value = 35
    (sheet: 1; row: 4; col: 0): value = 41
    (sheet: 1; row: 4; col: 1): value = 69
    (sheet: 1; row: 5; col: 0): value = 5
    (sheet: 1; row: 5; col: 1): value = 18
    (sheet: 1; row: 6; col: 0): value = 46
    (sheet: 1; row: 6; col: 1): value = 69
    (sheet: 1; row: 7; col: 0): value = 36
    (sheet: 1; row: 7; col: 1): value = 67
    (sheet: 1; row: 8; col: 0): value = 78
    (sheet: 1; row: 8; col: 1): value = 2
    (sheet: 1; row: 1; col: 2): formula = [.A2]+[.B2] (ods)
    (sheet: 1; row: 2; col: 2): formula = [.A3]+[.B3] (ods)
    (sheet: 1; row: 3; col: 2): formula = [.A4]+[.B4] (ods)
    (sheet: 1; row: 4; col: 2): formula = [.A5]+[.B5] (ods)
    (sheet: 1; row: 5; col: 2): formula = [.A6]+[.B6] (ods)
    (sheet: 1; row: 6; col: 2): formula = [.A7]+[.B7] (ods)
    (sheet: 1; row: 7; col: 2): formula = [.A8]+[.B8] (ods)
    (sheet: 1; row: 8; col: 2): formula = [.A9]+[.B9] (ods)

Looks like we are getting the formula cell values this time around.

One thing to note is that the formula expression strings you see here follow
the syntax defined in the OpenFormula specifications, which is the formula syntax
used in the OpenDocument Spreadsheet format.


Implement more interfaces
-------------------------

This section has covered only a part of the available spreadsheet interfaces
you can implement in your code.  Refer to the :ref:`spreadsheet-interfaces`
section to see the complete list of interfaces.
