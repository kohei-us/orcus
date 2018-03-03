
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

TBD
