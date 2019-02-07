
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
    assert(decl.attribute("version") == "1.0");
    assert(decl.attribute("encoding") == "UTF-8");
}

void test_attributes()
{
    orcus::dom_tree tree = load_dom_tree(SRCDIR"/test/xml/osm/street-in-aizu.osm");

    const_node root = tree.root();
    assert(root.name() == entity_name("osm"));
    assert(root.type() == node_t::element);
    assert(root.attribute("version") == "0.6");
    assert(root.attribute("generator") == "CGImap 0.6.1 (1984 thorn-02.openstreetmap.org)");
    assert(root.attribute("copyright") == "OpenStreetMap and contributors");
    assert(root.attribute("attribution") == "http://www.openstreetmap.org/copyright");
    assert(root.attribute("license") == "http://opendatacommons.org/licenses/odbl/1-0/");
    assert(root.attribute("no-such-attribute").empty());
    assert(root.attribute_count() == 5);
}

void test_element_hierarchy()
{
    orcus::dom_tree tree = load_dom_tree(SRCDIR"/test/xml/osm/street-in-aizu.osm");

    const_node root = tree.root();
    assert(root.name() == entity_name("osm"));
    assert(root.child_count() > 0);

    const_node elem = root.child(0);
    assert(elem != root);
    assert(elem.type() == node_t::element);
    assert(elem.name() == entity_name("bounds"));
    assert(elem.attribute("minlat") == "37.4793300");
    assert(elem.attribute("minlon") == "139.9158300");
    assert(elem.attribute("maxlat") == "37.4798000");
    assert(elem.attribute("maxlon") == "139.9162300");
    assert(elem.attribute_count() == 4);
    assert(elem.child_count() == 0);
    assert(elem.parent() == root);

    // TODO : continue with this.
}

int main()
{
    test_declaration();
    test_attributes();
    test_element_hierarchy();

    return EXIT_SUCCESS;
}
