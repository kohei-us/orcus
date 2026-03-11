//!code-start: header
#include <orcus/format_detection.hpp>  // for create_filter()

#include <orcus/spreadsheet/factory.hpp>
#include <orcus/spreadsheet/document.hpp>

#include <filesystem>

namespace fs = std::filesystem;
//!code-end: header

#include <iostream>

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
    //!code-start: testdir
    const char* testdir = std::getenv("TESTDIR");
    //!code-end: testdir

    if (!testdir)
    {
        std::cerr << "TESTDIR not defined" << std::endl;
        return EXIT_FAILURE;
    }

    //!code-start: filepath
    auto filepath = fs::path{testdir} / "ods" / "autofilter" / "text-comparisons.ods";
    //!code-end: filepath

    //!code-start: doc-and-factory
    orcus::spreadsheet::range_size_t ssize{1048576, 16384}; // sheet size
    orcus::spreadsheet::document doc{ssize};
    orcus::spreadsheet::import_factory factory(doc);
    //!code-end: doc-and-factory

    //!code-start: create-filter
    auto filter = orcus::create_filter(orcus::format_t::ods, &factory);
    filter->read_file(filepath);
    //!code-end: create-filter

    print_doc(doc);

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}

