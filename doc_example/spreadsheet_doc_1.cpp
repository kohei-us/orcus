
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/factory.hpp>
#include <orcus/orcus_ods.hpp>

#include <ixion/address.hpp>
#include <ixion/model_context.hpp>

#include <iostream>
#include <cstdlib>
#ifdef HAVE_FILESYSTEM
#include <filesystem>
namespace fs = std::filesystem;
#else
#ifdef HAVE_EXPERIMENTAL_FILESYSTEM
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#endif
#endif

using namespace orcus;

int main()
{
    fs::path input_dir = std::getenv("INPUTDIR");

    //!code-start: instantiate
    spreadsheet::range_size_t ss{1048576, 16384};
    spreadsheet::document doc{ss};
    spreadsheet::import_factory factory{doc};
    //!code-end: instantiate

    //!code-start: loader
    orcus_ods loader(&factory);
    //!code-end: loader

    //!code-start: read-file
    auto filepath = input_dir / "document.ods";
    loader.read_file(filepath.native());
    //!code-end: read-file

    //!code-start: model-context
    const ixion::model_context& model = doc.get_model_context();
    //!code-end: model-context

    //!code-start: string-id
    ixion::abs_address_t pos(0, 0, 0); // Set the cell position to A1.
    ixion::string_id_t str_id = model.get_string_identifier(pos);
    //!code-end: string-id

    //!code-start: print-string
    const std::string* s = model.get_string(str_id);
    assert(s);
    std::cout << "A1: " << *s << std::endl;
    //!code-end: print-string

    //!code-start: rest
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
    //!code-end: rest

    return EXIT_SUCCESS;
}
