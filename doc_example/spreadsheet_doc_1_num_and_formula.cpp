
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/factory.hpp>
#include <orcus/orcus_ods.hpp>

#include <ixion/address.hpp>
#include <ixion/model_context.hpp>
#include <ixion/formula_result.hpp>
#include <ixion/cell.hpp>

#include <iostream>
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

    // Instantiate a document, and wrap it with a factory.
    spreadsheet::range_size_t ss{1048576, 16384};
    spreadsheet::document doc{ss};
    spreadsheet::import_factory factory{doc};

    // Pass the factory to the document loader, and read the content from a file
    // to populate the document.
    orcus_ods loader(&factory);
    auto filepath = input_dir / "document.ods";
    loader.read_file(filepath.native());
    doc.recalc_formula_cells();

    // Now that the document is fully populated, access its content.
    const ixion::model_context& model = doc.get_model_context();

    //!code-start: print-numeric-cells
    for (spreadsheet::row_t row = 1; row <= 6; ++row)
    {
        ixion::abs_address_t pos(0, row, 0);
        double value = model.get_numeric_value(pos);
        std::cout << "A" << (pos.row+1) << ": " << value << std::endl;
    }
    //!code-end: print-numeric-cells

    //!code-start: print-formula-cells
    for (spreadsheet::row_t row = 1; row <=6; ++row)
    {
        ixion::abs_address_t pos(0, row, 2); // Column C
        const ixion::formula_cell* fc = model.get_formula_cell(pos);
        assert(fc);

        // Get the formula cell results.
        const ixion::formula_result& result = fc->get_result_cache(
            ixion::formula_result_wait_policy_t::throw_exception);

        // We already know the result is a string.
        const std::string& s = result.get_string();
        std::cout << "C" << (pos.row+1) << ": " << s << std::endl;
    }
    //!code-end: print-formula-cells

    return EXIT_SUCCESS;
}
