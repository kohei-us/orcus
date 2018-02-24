
.. highlight:: cpp

Use a user-defined custom document class
========================================

In this section we will demonstrate how you can use orcus to populate your own
custom document model by implementing your own set of interface classes and
passing it to the orcus import filter.  The first example code shown below is
the *absolute* minimum that you need to write in order for the orcus filter
to function properly::

    #include <orcus/spreadsheet/import_interface.hpp>
    #include <orcus/orcus_ods.hpp>

    #include <iostream>

    using namespace std;
    using namespace orcus::spreadsheet;
    using orcus::orcus_ods;

    class my_empty_import_factory : public iface::import_factory
    {
    public:
        ~my_empty_import_factory() {}

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
Sheet'**, **'2nd Sheet'**, and **'3rd Sheet'** in this exact order.

.. code-block:: text

    append_sheet: sheet index: 0; sheet name: 1st Sheet
    append_sheet: sheet index: 1; sheet name: 2nd Sheet
    append_sheet: sheet index: 2; sheet name: 3rd Sheet
