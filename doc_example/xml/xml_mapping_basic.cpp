#include <orcus/xml_namespace.hpp>
#include <orcus/stream.hpp>
#include <orcus/orcus_xml.hpp>

#include <orcus/spreadsheet/factory.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/sheet.hpp>

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main() try
{
    auto inputpath = fs::path{INPUTDIR} / "cities.xml";
    orcus::file_content input{inputpath};

    orcus::spreadsheet::range_size_t ssize{200, 10};
    orcus::spreadsheet::document doc{ssize};
    orcus::spreadsheet::import_factory factory(doc);

    orcus::xmlns_repository repo;
    orcus::orcus_xml filter{repo, &factory};
    filter.set_cell_link("/cities/header/@date-generated", "Cities", 0, 0);
    filter.set_cell_link("/cities/header/title", "Cities", 1, 0);
    filter.set_cell_link("/cities/header/source", "Cities", 2, 0);
    filter.append_sheet("Cities");
    filter.read_stream(input.str());

    const auto* sheet = doc.get_sheet(0);
    if (!sheet)
    {
        std::cerr << "failed to fetch the first sheet" << std::endl;
        return EXIT_FAILURE;
    }

    sheet->dump_flat(std::cout);

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}

