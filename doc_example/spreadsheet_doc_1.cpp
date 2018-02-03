
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/factory.hpp>
#include <orcus/orcus_xlsx.hpp>

using namespace orcus;

int main()
{
    spreadsheet::document doc;
    spreadsheet::import_factory factory(doc);

    orcus_xlsx loader(&factory);

    // TODO : continue on...

    return EXIT_SUCCESS;
}
