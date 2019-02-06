
#include <orcus/dom_tree.hpp>
#include <orcus/stream.hpp>
#include <orcus/xml_namespace.hpp>
#include <orcus/pstring.hpp>
#include <cassert>

using namespace orcus::dom;

orcus::dom_tree load_dom_tree(const char* filepath)
{
    orcus::xmlns_repository repo;
    orcus::xmlns_context cxt = repo.create_context();
    orcus::dom_tree tree(cxt);

    std::string content = orcus::load_file_content(filepath);
    tree.load(content);
    return tree;
}

void test_declaration()
{
    orcus::dom_tree tree = load_dom_tree(SRCDIR"/test/xml/osm/street-in-aizu.osm");

    const_node decl = tree.declaration("xml");
    assert(decl.type() == node_t::declaration);
    assert(decl.child_count() == 2);

    const_node attr = decl.child(0);
    assert(attr.type() == node_t::attribute);
    assert(attr.name().ns == orcus::XMLNS_UNKNOWN_ID);
    assert(attr.name().name == "version");
    assert(attr.value() == "1.0");

    attr = decl.child(1);
    assert(attr.type() == node_t::attribute);
    assert(attr.name().ns == orcus::XMLNS_UNKNOWN_ID);
    assert(attr.name().name == "encoding");
    assert(attr.value() == "UTF-8");
}

void test_attributes()
{
    orcus::dom_tree tree = load_dom_tree(SRCDIR"/test/xml/osm/street-in-aizu.osm");

    const_node root = tree.root();
    assert(root.type() == node_t::element);
    assert(root.attribute("version") == "0.6");
    assert(root.attribute("generator") == "CGImap 0.6.1 (1984 thorn-02.openstreetmap.org)");
    assert(root.attribute("copyright") == "OpenStreetMap and contributors");
    assert(root.attribute("attribution") == "http://www.openstreetmap.org/copyright");
    assert(root.attribute("license") == "http://opendatacommons.org/licenses/odbl/1-0/");
    assert(root.attribute("no-such-attribute").empty());
}

int main()
{
    test_declaration();
    test_attributes();

    return EXIT_SUCCESS;
}
