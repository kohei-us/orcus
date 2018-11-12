
.. highlight:: cpp

Use a user-defined custom document class
========================================

In this section we will demonstrate how you can use orcus to populate your own
custom document model by implementing your own set of interface classes and
passing it to the orcus import filter.  The first example code shown below is
the *absolute* minimum that you need to implement in order for the orcus
filter to function properly::

    #include <orcus/spreadsheet/import_interface.hpp>
    #include <orcus/orcus_ods.hpp>

    #include <iostream>

    using namespace std;
    using namespace orcus::spreadsheet;
    using orcus::orcus_ods;

    class my_empty_import_factory : public iface::import_factory
    {
    public:
        virtual ~my_empty_import_factory() override {}

        virtual iface::import_sheet* append_sheet(
            sheet_t sheet_index, const char* sheet_name, size_t sheet_name_length) override
        {
            cout << "append_sheet: sheet index: " << sheet_index
                 << "; sheet name: " << string(sheet_name, sheet_name_length)
                 << endl;
            return nullptr;
        }

        virtual iface::import_sheet* get_sheet(
            const char* sheet_name, size_t sheet_name_length) override
        {
            cout << "get_sheet: sheet name: "
                 << string(sheet_name, sheet_name_length) << endl;
            return nullptr;
        }

        virtual iface::import_sheet* get_sheet(sheet_t sheet_index) override
        {
            cout << "get_sheet: sheet index: " << sheet_index << endl;
            return nullptr;
        }

        virtual void finalize() override {}
    };

    int main()
    {
        my_empty_import_factory factory;
        orcus_ods loader(&factory);
        loader.read_file("/path/to/multi-sheets.ods");

        return EXIT_SUCCESS;
    }

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


Implement sheet accessors
-------------------------

In this section we will expand on the code in the previous section to
implement sheet accessors, in order to receive cell values in each individual
sheet.  In this example, we will define a structure to hold a cell value, and
store them in a 2-dimensional array for each sheet.  First, let's define the
cell value structure::

    enum class cell_value_type { empty, numeric, string };

    struct cell_value
    {
        cell_value_type type;

        union
        {
            size_t s;
            double f;
        };

        cell_value() : type(cell_value_type::empty) {}
    };

As we will be handling only three cell types i.e.  empty, numeric, or string
cell type, this structure will work just fine.  Next, we'll define a sheet
class called ``my_sheet`` that stores the cell values in a 2-dimensional
array, and implements all required interfaces as a child class of
:cpp:class:`~orcus::spreadsheet::iface::import_sheet`.

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

Here is the actual code for class ``my_sheet``::

    class my_sheet : public iface::import_sheet
    {
        cell_value m_cells[100][1000];
        range_size_t m_sheet_size;
        sheet_t m_sheet_index;

    public:
        my_sheet(sheet_t sheet_index) :
            m_sheet_index(sheet_index)
        {
            m_sheet_size.rows = 1000;
            m_sheet_size.columns = 100;
        }

        virtual void set_auto(row_t row, col_t col, const char* p, size_t n) override
        {
            // TODO : implement this.
        }

        virtual void set_string(row_t row, col_t col, size_t sindex) override
        {
            cout << "(sheet: " << m_sheet_index << "; row: " << row << "; col: " << col << "): string index = " << sindex << endl;

            m_cells[col][row].type = cell_value_type::string;
            m_cells[col][row].s = sindex;
        }

        virtual void set_value(row_t row, col_t col, double value) override
        {
            cout << "(sheet: " << m_sheet_index << "; row: " << row << "; col: " << col << "): value = " << value << endl;

            m_cells[col][row].type = cell_value_type::numeric;
            m_cells[col][row].f = value;
        }

        virtual void set_bool(row_t row, col_t col, bool value) override
        {
            // TODO : implement this.
        }

        virtual void set_date_time(
            row_t row, col_t col, int year, int month, int day, int hour, int minute, double second) override
        {
            // TODO : implement this.
        }

        virtual void set_format(row_t row, col_t col, size_t xf_index) override
        {
            // TODO : implement this.
        }

        virtual void set_format(
            row_t row_start, col_t col_start, row_t row_end, col_t col_end, size_t xf_index) override
        {
            // TODO : implement this.
        }

        virtual range_size_t get_sheet_size() const override
        {
            return m_sheet_size;
        }
    };

Note that this class receives its sheet index value from the caller upon
instantiation.  A sheet index is a 0-based value and represents its position
within the sheet collection.

Finally, we will modify the ``my_import_factory`` class to store and manage a
collection of ``my_sheet`` instances and to return the pointer value to a
correct sheet accessor instance as needed.

::

    class my_import_factory : public iface::import_factory
    {
        vector<unique_ptr<my_sheet>> m_sheets;

    public:
        virtual ~my_import_factory() {}

        virtual iface::import_sheet* append_sheet(
            sheet_t sheet_index, const char* sheet_name, size_t sheet_name_length) override
        {
            m_sheets.push_back(make_unique<my_sheet>(m_sheets.size()));
            return m_sheets.back().get();
        }

        virtual iface::import_sheet* get_sheet(
            const char* sheet_name, size_t sheet_name_length) override
        {
            // TODO : implement this.
            return nullptr;
        }

        virtual iface::import_sheet* get_sheet(sheet_t sheet_index) override
        {
            sheet_t sheet_count = m_sheets.size();
            return sheet_index < sheet_count ? m_sheets[sheet_index].get() : nullptr;
        }

        virtual void finalize() override {}
    };

Let's put it all together and run this code::

    #include <orcus/spreadsheet/import_interface.hpp>
    #include <orcus/orcus_ods.hpp>

    #include <iostream>
    #include <memory>

    using namespace std;
    using namespace orcus::spreadsheet;
    using orcus::orcus_ods;

    enum class cell_value_type { empty, numeric, string };

    struct cell_value
    {
        cell_value_type type;

        union
        {
            size_t s;
            double f;
        };

        cell_value() : type(cell_value_type::empty) {}
    };

    class my_sheet : public iface::import_sheet
    {
        cell_value m_cells[100][1000];
        range_size_t m_sheet_size;
        sheet_t m_sheet_index;

    public:
        my_sheet(sheet_t sheet_index) :
            m_sheet_index(sheet_index)
        {
            m_sheet_size.rows = 1000;
            m_sheet_size.columns = 100;
        }

        virtual void set_auto(row_t row, col_t col, const char* p, size_t n) override
        {
            // TODO : implement this.
        }

        virtual void set_string(row_t row, col_t col, size_t sindex) override
        {
            cout << "(sheet: " << m_sheet_index << "; row: " << row << "; col: " << col << "): string index = " << sindex << endl;

            m_cells[col][row].type = cell_value_type::string;
            m_cells[col][row].s = sindex;
        }

        virtual void set_value(row_t row, col_t col, double value) override
        {
            cout << "(sheet: " << m_sheet_index << "; row: " << row << "; col: " << col << "): value = " << value << endl;

            m_cells[col][row].type = cell_value_type::numeric;
            m_cells[col][row].f = value;
        }

        virtual void set_bool(row_t row, col_t col, bool value) override
        {
            // TODO : implement this.
        }

        virtual void set_date_time(
            row_t row, col_t col, int year, int month, int day, int hour, int minute, double second) override
        {
            // TODO : implement this.
        }

        virtual void set_format(row_t row, col_t col, size_t xf_index) override
        {
            // TODO : implement this.
        }

        virtual void set_format(
            row_t row_start, col_t col_start, row_t row_end, col_t col_end, size_t xf_index) override
        {
            // TODO : implement this.
        }

        virtual range_size_t get_sheet_size() const override
        {
            return m_sheet_size;
        }
    };

    class my_import_factory : public iface::import_factory
    {
        vector<unique_ptr<my_sheet>> m_sheets;

    public:
        virtual ~my_import_factory() {}

        virtual iface::import_sheet* append_sheet(
            sheet_t sheet_index, const char* sheet_name, size_t sheet_name_length) override
        {
            m_sheets.push_back(make_unique<my_sheet>(m_sheets.size()));
            return m_sheets.back().get();
        }

        virtual iface::import_sheet* get_sheet(
            const char* sheet_name, size_t sheet_name_length) override
        {
            // TODO : implement this.
            return nullptr;
        }

        virtual iface::import_sheet* get_sheet(sheet_t sheet_index) override
        {
            sheet_t sheet_count = m_sheets.size();
            return sheet_index < sheet_count ? m_sheets[sheet_index].get() : nullptr;
        }

        virtual void finalize() override {}
    };

    int main()
    {
        my_import_factory factory;
        orcus_ods loader(&factory);
        loader.read_file("/path/to/multi-sheets.ods");

        return EXIT_SUCCESS;
    }

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

There is a couple of things worth pointing out.  First, the cell data flows
left to right first then top to bottom second, starting from A1 - the row
position of 0 and the column position of 0, all the way to D11 - row position
of 10 and the column position of 3.  Second, for this particular sheet,
implementing just the two setter methods, namely
:cpp:func:`~orcus::spreadsheet::iface::import_sheet::set_string` and
:cpp:func:`~orcus::spreadsheet::iface::import_sheet::set_value` are enough to
receive all cell values.  However, we are getting a string index value of 0
for all string cells.  This is because orcus expects the custom document model
to implement the relevant string loader interface for the pooled string
values, and we have not yet implemented one.  Let's fix that.



