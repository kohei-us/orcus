
#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/orcus_ods.hpp>

#include <filesystem>
#include <iostream>

namespace ss = orcus::spreadsheet;

class my_empty_import_factory : public ss::iface::import_factory
{
public:
    virtual ss::iface::import_sheet* append_sheet(ss::sheet_t sheet_index, std::string_view name) override
    {
        std::cout << "append_sheet: sheet index: " << sheet_index << "; sheet name: " << name << std::endl;
        return nullptr;
    }

    virtual ss::iface::import_sheet* get_sheet(std::string_view name) override
    {
        std::cout << "get_sheet: sheet name: " << name << std::endl;
        return nullptr;
    }

    virtual ss::iface::import_sheet* get_sheet(ss::sheet_t sheet_index) override
    {
        std::cout << "get_sheet: sheet index: " << sheet_index << std::endl;
        return nullptr;
    }

    virtual void finalize() override {}
};

int main()
{
    std::filesystem::path input_dir = std::getenv("INPUTDIR");
    auto filepath = input_dir / "multi-sheets.ods";

    my_empty_import_factory factory;
    orcus::orcus_ods loader(&factory);
    loader.read_file(filepath.native());

    return EXIT_SUCCESS;
}
