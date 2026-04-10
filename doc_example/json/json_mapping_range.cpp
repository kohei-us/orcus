
#include <orcus/stream.hpp>
#include <orcus/orcus_json.hpp>

#include <orcus/spreadsheet/factory.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/sheet.hpp>

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main() try
{
    auto inputpath = fs::path{INPUTDIR} / "books.json";
    orcus::file_content input{inputpath};
    orcus::spreadsheet::range_size_t ssize{200, 10};
    orcus::spreadsheet::document doc{ssize};
    orcus::spreadsheet::import_factory factory(doc);

    orcus::orcus_json filter{&factory};

    //!code-start: range
    filter.start_range("Books", 0, 0, true);
    filter.append_field_link("$['books'][]['title']", "Title");
    filter.append_field_link("$['books'][]['author']", "Author");
    filter.append_field_link("$['books'][]['year']", "Year");
    filter.append_field_link("$['books'][]['genre']", "Genre");
    filter.append_field_link("$['books'][]['rating']", "Rating");
    filter.set_range_row_group("$['books']");
    filter.commit_range();
    //!code-end: range

    filter.append_sheet("Books");
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
