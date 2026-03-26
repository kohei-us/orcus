
//!code-start: headers
#include <orcus/xml_namespace.hpp>
#include <orcus/stream.hpp>
#include <orcus/orcus_xml.hpp>

#include <orcus/spreadsheet/factory.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/sheet.hpp>

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;
//!code-end: headers

int main() try
{
    //!code-start: load-input
    auto inputpath = fs::path{INPUTDIR} / "cities.xml";
    orcus::file_content input{inputpath};
    //!code-end: load-input

    //!code-start: create-doc
    orcus::spreadsheet::range_size_t ssize{200, 10};
    orcus::spreadsheet::document doc{ssize};
    orcus::spreadsheet::import_factory factory(doc);
    //!code-end: create-doc

    //!code-start: setup-orcus-xml
    orcus::xmlns_repository repo;
    orcus::orcus_xml filter{repo, &factory};
    //!code-end: setup-orcus-xml

    //!code-start: cell-links
    filter.set_cell_link("/cities/header/@date-generated", "Cities", 0, 0);
    filter.set_cell_link("/cities/header/title", "Cities", 1, 0);
    filter.set_cell_link("/cities/header/source", "Cities", 2, 0);
    //!code-end: cell-links

    //!code-start: read-xml
    filter.append_sheet("Cities");
    filter.read_stream(input.str());
    //!code-end: read-xml

    //!code-start: dump-content
    const auto* sheet = doc.get_sheet(0);
    if (!sheet)
        throw std::runtime_error("failed to fetch the first sheet");

    sheet->dump_flat(std::cout);
    //!code-end: dump-content

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}

