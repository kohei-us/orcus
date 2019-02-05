
#include <orcus/dom_tree.hpp>
#include <orcus/stream.hpp>
#include <orcus/xml_namespace.hpp>
#include <cassert>

void test_declaration()
{
    using namespace orcus::dom;

    orcus::xmlns_repository repo;
    orcus::xmlns_context cxt = repo.create_context();
    orcus::dom_tree tree(cxt);

    std::string content = orcus::load_file_content(SRCDIR"/test/xml/osm/street-in-aizu.osm");
    tree.load(content);

    const_node decl = tree.declaration("xml");
    assert(decl.type() == node_t::declaration);
    assert(decl.child_count() == 2);

    const_node attr = decl.child(0);
    assert(attr.type() == node_t::attribute);
    assert(attr.name().ns == orcus::XMLNS_UNKNOWN_ID);
    assert(attr.name().name == "version");

    attr = decl.child(1);
    assert(attr.type() == node_t::attribute);
    assert(attr.name().ns == orcus::XMLNS_UNKNOWN_ID);
    assert(attr.name().name == "encoding");
}

int main()
{
    test_declaration();

    return EXIT_SUCCESS;
}
