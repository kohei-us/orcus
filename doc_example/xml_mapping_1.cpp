
#include <orcus/xml_namespace.hpp>

#include <iostream>

using namespace orcus;
using namespace std;

void run_xmlns_example()
{
    // A namespace repository is a shared storage for namespace strings for
    // multiple contexts.  The client code needs to simply create an instance
    // from which to create contexts.
    xmlns_repository ns_repo;

    xmlns_context ns_cxt = ns_repo.create_context();

    // Push namespaces with their aliases as you counter them.  The push()
    // method then returns an identifier associated with the alias.

    // null alias is for default namespace.
    xmlns_id_t ns_default = ns_cxt.push(
        nullptr, "http://schemas.openxmlformats.org/spreadsheetml/2006/main");

    xmlns_id_t ns_a = ns_cxt.push(
        "a", "http://schemas.openxmlformats.org/drawingml/2006/main");

    xmlns_id_t ns_r = ns_cxt.push(
        "r", "http://schemas.openxmlformats.org/officeDocument/2006/relationships");

    // You can retrieve the data associated with alias ID's.
    for (const xmlns_id_t nsid : {ns_default, ns_a, ns_r})
    {
        pstring alias = ns_cxt.get_alias(nsid);
        cout << "Namespace alias '" << alias << "' has an index of " << ns_cxt.get_index(nsid)
            << " and a short name of '" << ns_cxt.get_short_name(nsid) << "'." << endl;
        cout << "The value of the alias '" << alias << "' is '" << ns_cxt.get(alias) << "'." << endl;
    }
}

int main(int argc, char** argv)
{
    run_xmlns_example();

    return EXIT_SUCCESS;
}
