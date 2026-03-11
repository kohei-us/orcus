#include <orcus/spreadsheet/factory.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/orcus_ods.hpp>

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;


void print_doc(const orcus::spreadsheet::document& doc)
{
    for (std::size_t i = 0; i < doc.get_sheet_count(); ++i)
    {
        std::cout << "- sheet " << i << std::endl;
        auto name = doc.get_sheet_name(i);
        std::cout << "  sheet name: " << name << std::endl;
    }
}

int main() try
{
    const char* testdir = std::getenv("TESTDIR");

    if (!testdir)
    {
        std::cerr << "TESTDIR not defined" << std::endl;
        return EXIT_FAILURE;
    }

    auto filepath = fs::path{testdir} / "ods" / "autofilter" / "text-comparisons.ods";

    orcus::spreadsheet::range_size_t ssize{1048576, 16384};
    orcus::spreadsheet::document doc{ssize};
    orcus::spreadsheet::import_factory factory(doc);

    //!code-start: direct
    orcus::orcus_ods filter{&factory};
    filter.read_file(filepath);
    //!code-end: direct

    print_doc(doc);

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}

