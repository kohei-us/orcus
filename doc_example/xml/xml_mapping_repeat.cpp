
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

    //!code-start: range
    filter.start_range("Cities", 0, 0);
    filter.append_field_link("/cities/city/@name", "City");
    filter.append_field_link("/cities/city/country", "Country");
    filter.append_field_link("/cities/city/population", "Population");
    filter.append_field_link("/cities/city/fact", "Fact");
    filter.append_field_link("/cities/city/landmark", "Popular Spot");
    filter.set_range_row_group("/cities/city");
    filter.commit_range();
    //!code-end: range

    filter.append_sheet("Cities");
    filter.read_stream(input.str());

    const auto* sheet = doc.get_sheet(0);
    if (!sheet)
        throw std::runtime_error("failed to fetch the first sheet");

    sheet->dump_flat(std::cout);

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}

