
#include <orcus/stream.hpp>
#include <orcus/orcus_json.hpp>

#include <orcus/spreadsheet/factory.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/sheet.hpp>

#include <iostream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

void detect_and_read(const orcus::file_content& input)
{
    //!code-start: simple-setup
    orcus::spreadsheet::range_size_t ssize{100, 20};
    orcus::spreadsheet::document doc{ssize};
    orcus::spreadsheet::import_factory factory(doc);

    orcus::orcus_json filter{&factory};
    //!code-end: simple-setup

    //!code-start: simple-detect
    filter.detect_map_definition(input.str());
    filter.read_stream(input.str());
    //!code-end: simple-detect

    //!code-start: simple-dump
    const auto* sheet = doc.get_sheet(0);
    if (!sheet)
        throw std::runtime_error("failed to fetch the first sheet");

    sheet->dump_flat(std::cout);
    //!code-end: simple-dump
}

void write_and_read_map_def(const orcus::file_content& input)
{
    orcus::spreadsheet::range_size_t ssize{100, 20};
    orcus::spreadsheet::document doc{ssize};
    orcus::spreadsheet::import_factory factory(doc);

    orcus::orcus_json filter{&factory};

    //!code-start: write-map-def
    std::ostringstream os;
    filter.write_map_definition(input.str(), os);

    // print the map definition to stdout for inspection
    auto map_def = os.str();
    std::cout << map_def << std::endl;
    //!code-end: write-map-def

    //!code-start: read-map-def
    filter.read_map_definition(map_def);
    filter.read_stream(input.str());
    //!code-end: read-map-def

    const auto* sheet = doc.get_sheet(0);
    if (!sheet)
        throw std::runtime_error("failed to fetch the first sheet");

    sheet->dump_flat(std::cout);
}

int main() try
{
    auto inputpath = fs::path{INPUTDIR} / "planets.json";
    orcus::file_content input{inputpath};

    detect_and_read(input);
    write_and_read_map_def(input);

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}
