
#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/orcus_ods.hpp>

#include <iostream>

using namespace std;
using namespace orcus::spreadsheet;
using orcus::orcus_ods;

class my_empty_import_factory : public iface::import_factory
{
public:
    virtual iface::import_sheet* append_sheet(sheet_t sheet_index, std::string_view name) override
    {
        cout << "append_sheet: sheet index: " << sheet_index << "; sheet name: " << name << endl;
        return nullptr;
    }

    virtual iface::import_sheet* get_sheet(std::string_view name) override
    {
        cout << "get_sheet: sheet name: " << name << endl;
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
    loader.read_file(SRCDIR"/doc_example/files/multi-sheets.ods");

    return EXIT_SUCCESS;
}
