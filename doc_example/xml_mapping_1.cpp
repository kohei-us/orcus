
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

    // empty alias is for default namespace.  You can either use nullptr or an
    // empty string.
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

void run_xmlns_stacked()
{
    xmlns_repository ns_repo;
    xmlns_context ns_cxt = ns_repo.create_context();

    // Push a first default namespace.
    xmlns_id_t ns_default_1 = ns_cxt.push(nullptr, "http://original");

    // Push a nested deffault namespace.  This overwrites the original.
    xmlns_id_t current_default_ns = ns_cxt.push(nullptr, "http://nested");
    cout << "same as original: " << (current_default_ns == ns_default_1) << endl;

    // Pop the current default namespace.  After this the original namespace
    // becomes the default namespace again.
    ns_cxt.pop(nullptr);

    // Get the current default namespace identifier.
    current_default_ns = ns_cxt.get(nullptr);
    cout << "same as original: " << (current_default_ns == ns_default_1) << endl;
}

void run_xmlns_same_ns_different_aliases()
{
    xmlns_repository ns_repo;

    // Same namespace URI may be associated with different aliases in different
    // contexts.

    xmlns_id_t alias_1, alias_2;
    {
        xmlns_context ns_cxt = ns_repo.create_context();
        alias_1 = ns_cxt.push("foo", "http://some-namespace");
    }

    {
        xmlns_context ns_cxt = ns_repo.create_context();
        alias_2 = ns_cxt.push("bar", "http://some-namespace");
    }

    cout << (alias_1 == alias_2 ? "same" : "different") << endl;
}

void run_xmlns_different_ns_same_alias()
{
    xmlns_repository ns_repo;

    // Same alias may be associated with different namespace URI's in different
    // contexts.

    xmlns_id_t alias_1, alias_2;
    {
        xmlns_context ns_cxt = ns_repo.create_context();
        alias_1 = ns_cxt.push("foo", "http://namespace-1");
    }

    {
        xmlns_context ns_cxt = ns_repo.create_context();
        alias_2 = ns_cxt.push("foo", "http://namespace-2");
    }

    cout << (alias_1 == alias_2 ? "same" : "different") << endl;
}

int main(int argc, char** argv)
{
    run_xmlns_example();
    run_xmlns_stacked();
    run_xmlns_same_ns_different_aliases();
    run_xmlns_different_ns_same_alias();

    return EXIT_SUCCESS;
}
