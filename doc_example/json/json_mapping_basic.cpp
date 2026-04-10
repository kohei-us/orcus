
//!code-start: headers
#include <orcus/stream.hpp>
#include <orcus/orcus_json.hpp>

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
    auto inputpath = fs::path{INPUTDIR} / "books.json";
    orcus::file_content input{inputpath};
    //!code-end: load-input

    //!code-start: create-doc
    orcus::spreadsheet::range_size_t ssize{200, 10};
    orcus::spreadsheet::document doc{ssize};
    orcus::spreadsheet::import_factory factory(doc);
    //!code-end: create-doc

    //!code-start: setup-orcus-json
    orcus::orcus_json filter{&factory};
    //!code-end: setup-orcus-json

    //!code-start: cell-links
    filter.set_cell_link("$['meta']['title']", "Books", 0, 0);
    filter.set_cell_link("$['meta']['owner']", "Books", 1, 0);
    filter.set_cell_link("$['meta']['last-updated']", "Books", 2, 0);
    //!code-end: cell-links

    //!code-start: read-json
    filter.append_sheet("Books");
    filter.read_stream(input.str());
    //!code-end: read-json

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
