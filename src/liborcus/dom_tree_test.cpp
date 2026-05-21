
#include <orcus/dom_tree.hpp>
#include <orcus/stream.hpp>
#include <orcus/xml_namespace.hpp>
#include <cassert>
#include <iostream>

using namespace orcus::dom;

struct doctree
{
    orcus::xmlns_repository repo;
    orcus::xmlns_context cxt;
    orcus::dom::document_tree tree;

    doctree(std::string_view content) : repo(), cxt(repo.create_context()), tree(cxt)
    {
        tree.load(content);
    }
};

std::unique_ptr<doctree> load_document_tree(std::string_view content)
{
    std::unique_ptr<doctree> ret = std::make_unique<doctree>(content);
    return ret;
}

std::unique_ptr<doctree> load_document_tree_from_file(const char* filepath)
{
    orcus::file_content content(filepath);
    return load_document_tree(content.str());
}

void test_encoded_attr()
{
    std::string content = "<?xml version=\"1.0\"?><root attr=\"&amp;;\"/>";
    auto doctree = load_document_tree(content);
    const_node root = doctree->tree.root();
    doctree->tree.dump_compact(std::cout);
    std::cout << __FILE__ << "#" << __LINE__ << " (:test_encoded_attr): " << root.attribute("attr") << std::endl;
    assert(root.attribute("attr") == "&;");
}

void test_declaration()
{
    auto doctree = load_document_tree_from_file(SRCDIR"/test/xml/osm/street-in-aizu.osm");

    const_node decl = doctree->tree.declaration("xml");
    assert(decl.type() == node_t::declaration);
    assert(decl.attribute("version") == "1.0");
    assert(decl.attribute("encoding") == "UTF-8");

    const_node copied = decl;
    assert(copied == decl);
}

void test_attributes()
{
    auto doctree = load_document_tree_from_file(SRCDIR"/test/xml/osm/street-in-aizu.osm");

    const_node root = doctree->tree.root();
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
    auto doctree = load_document_tree_from_file(SRCDIR"/test/xml/osm/street-in-aizu.osm");

    const_node root = doctree->tree.root();
    assert(root.name() == entity_name("osm"));
    assert(root.child_count() > 0);
    assert(root.parent().type() == node_t::unset);

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

    const_node copied_elem = elem;
    assert(copied_elem == elem);

    elem = root.child(5);
    assert(elem.name() == entity_name("node"));
    assert(elem.attribute("user") == "jun_meguro");
    assert(elem.child_count() == 1);
    elem = elem.child(0);
    assert(elem.name() == entity_name("tag"));
    assert(elem.attribute("k") == "highway");
    assert(elem.attribute("v") == "crossing");

    // Make sure the number of child elements are accurate.
    size_t n_elems = root.child_count();
    for (size_t i = 0; i < n_elems; ++i)
    {
        auto child = root.child(i);
        assert(child.type() == node_t::element);
    }
}

void test_comments()
{
    {
        // block layout - comment between element siblings
        std::string_view input = R"(<?xml version="1.0"?><root><a/><!--c--><b/></root>)";
        auto dt = load_document_tree(input);
        std::string output = dt->tree.dump(2);
        assert(output.find("<!--c-->") != std::string::npos);

        // <!--c--> must sit on its own indented line between <a/> and <b/>
        // because the parent has child elements
        assert(output.find("  <!--c-->\n") != std::string::npos);
    }

    {
        // inline layout - comment between text segments
        std::string_view input = R"(<?xml version="1.0"?><root>before<!--c-->after</root>)";
        auto dt = load_document_tree(input);
        std::string output = dt->tree.dump(2);

        // these parts stay together because the parent has no child elements
        assert(output.find("before<!--c-->after") != std::string::npos);
    }

    {
        // sole-child comment
        std::string_view input = R"(<?xml version="1.0"?><root><!--only--></root>)";
        auto dt = load_document_tree(input);
        std::string output = dt->tree.dump(2);

        // like the previous case, the parent has no child elements -> no reformatting
        assert(output.find("<root><!--only--></root>") != std::string::npos);
    }

    {
        // prolog comment between declaration and root
        std::string_view input = R"(<?xml version="1.0"?><!--prolog--><root/>)";
        auto dt = load_document_tree(input);
        std::string output = dt->tree.dump(2);
        assert(output.find("<!--prolog-->") != std::string::npos);

        // prolog comment must sit after the <?xml ?> declaration and before <root/>
        assert(output.find("?>") < output.find("<!--prolog-->"));
        assert(output.find("<!--prolog-->") < output.find("<root"));
    }

    {
        // epilog comment after root
        std::string_view input = R"(<?xml version="1.0"?><root/><!--epilog-->)";
        auto dt = load_document_tree(input);
        std::string output = dt->tree.dump(2);
        assert(output.find("<!--epilog-->") != std::string::npos);

        // epilog comment must sit after the root element
        assert(output.find("<root/>") < output.find("<!--epilog-->"));
    }
}

int main()
{
    test_encoded_attr();
    test_declaration();
    test_attributes();
    test_element_hierarchy();
    test_comments();

    return EXIT_SUCCESS;
}
