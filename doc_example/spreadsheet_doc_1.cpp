
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/factory.hpp>
#include <orcus/orcus_ods.hpp>

#include <ixion/model_context.hpp>
#include <iostream>

using namespace orcus;

int main()
{
    // Instantiate a document, and wrap it with a factory.
    spreadsheet::range_size_t ss{1048576, 16384};
    spreadsheet::document doc{ss};
    spreadsheet::import_factory factory{doc};

    // Pass the factory to the document loader, and read the content from a file
    // to populate the document.
    orcus_ods loader(&factory);
    loader.read_file(SRCDIR"/doc_example/files/document.ods");

    // Now that the document is fully populated, access its content.
    const ixion::model_context& model = doc.get_model_context();

    // Read the header row and print its content.

    ixion::abs_address_t pos(0, 0, 0); // Set the cell position to A1.
    ixion::string_id_t str_id = model.get_string_identifier(pos);

    const std::string* s = model.get_string(str_id);
    assert(s);
    std::cout << "A1: " << *s << std::endl;

    pos.column = 1; // Move to B1
    str_id = model.get_string_identifier(pos);
    s = model.get_string(str_id);
    assert(s);
    std::cout << "B1: " << *s << std::endl;

    pos.column = 2; // Move to C1
    str_id = model.get_string_identifier(pos);
    s = model.get_string(str_id);
    assert(s);
    std::cout << "C1: " << *s << std::endl;

    return EXIT_SUCCESS;
}
