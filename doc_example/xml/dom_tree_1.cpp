
//!code-start: headers
#include <orcus/dom_tree.hpp>
#include <orcus/xml_namespace.hpp>
#include <orcus/stream.hpp>

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;
//!code-end: headers

using namespace orcus;

void run_load_and_navigate()
{
    std::cout << "--- load and navigate ---" << std::endl;

    //!code-start: load
    // the repository creates namespace identifiers for the stored names and must
    // outlive the tree
    xmlns_repository repo;
    dom::document_tree tree(repo);

    auto inputpath = fs::path{INPUTDIR} / "library.xml";
    file_content input{inputpath};
    tree.load(input.str());
    //!code-end: load

    //!code-start: navigate-root
    // root() hands back a read-only handle into storage owned by the tree
    dom::const_node root = tree.root();
    std::cout << "root: " << root.name().name << std::endl;
    std::cout << "  name attribute: " << root.attribute("name") << std::endl;
    std::cout << "  child count: " << root.child_count() << std::endl;
    //!code-end: navigate-root

    //!code-start: navigate-children
    // child_count() counts only child elements, so text and whitespace between
    // the elements are not included
    for (std::size_t i = 0; i < root.child_count(); ++i)
    {
        dom::const_node child = root.child(i);
        std::cout << "  child " << i << ": " << child.name().name
            << " id=" << child.attribute("id")
            << " title='" << child.attribute("title") << "'" << std::endl;
    }
    //!code-end: navigate-children
}

void run_build_and_serialize()
{
    std::cout << "--- build and serialize ---" << std::endl;

    //!code-start: build
    xmlns_repository repo;
    dom::document_tree tree(repo);

    // set_root() installs a fresh root element and returns a mutable handle
    dom::node root = tree.set_root({"message"});
    root.set_attribute("lang", "en");

    dom::node greeting = root.append_element({"greeting"});
    greeting.append_content("Hello, world!");
    //!code-end: build

    //!code-start: serialize
    // dump() serializes the tree; the indent is the number of spaces per level
    std::cout << tree.dump(2) << std::endl;
    //!code-end: serialize
}

int main() try
{
    run_load_and_navigate();
    run_build_and_serialize();

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}
