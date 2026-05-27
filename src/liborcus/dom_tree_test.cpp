
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

    const_node decl = doctree->tree.declaration();
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

void test_mutable_build_from_scratch()
{
    orcus::xmlns_repository repo;
    orcus::xmlns_context cxt = repo.create_context();
    document_tree tree(cxt);

    auto root = tree.set_root({"book"});
    root.set_attribute("id", "42");
    root.set_attribute("lang", "en");

    auto title = root.append_element({"title"});
    title.append_content("Hello");

    auto body = root.append_element({"body"});
    body.append_content("World");
    body.append_comment(" note ");

    std::string output = tree.dump(2);

    // attributes appear on the root, both children present, comment preserved
    assert(output.find(R"(<book)") != std::string::npos);
    assert(output.find(R"(id="42")") != std::string::npos);
    assert(output.find(R"(lang="en")") != std::string::npos);
    assert(output.find("<title>Hello</title>") != std::string::npos);
    assert(output.find("World") != std::string::npos);
    assert(output.find("<!-- note -->") != std::string::npos);

    // round-trip: re-parse the dumped output and verify structure survives
    auto rt = load_document_tree(output);
    const_node rt_root = rt->tree.root();
    assert(rt_root.name() == entity_name("book"));
    assert(rt_root.attribute("id") == "42");
    assert(rt_root.attribute("lang") == "en");
    assert(rt_root.child_count() == 2);
    assert(rt_root.child(0).name() == entity_name("title"));
    assert(rt_root.child(1).name() == entity_name("body"));
}

void test_mutable_attribute_insert_update()
{
    orcus::xmlns_repository repo;
    orcus::xmlns_context cxt = repo.create_context();
    document_tree tree(cxt);

    auto root = tree.set_root({"root"});
    root.set_attribute("a", "1");
    root.set_attribute("b", "2");
    assert(root.attribute_count() == 2);
    assert(root.attribute("a") == "1");
    assert(root.attribute("b") == "2");

    // updating the same attribute must not grow the count
    root.set_attribute("a", "updated");
    assert(root.attribute_count() == 2);
    assert(root.attribute("a") == "updated");
    assert(root.attribute("b") == "2");
}

void test_mutable_declaration_pi_and_comments()
{
    orcus::xmlns_repository repo;
    orcus::xmlns_context cxt = repo.create_context();
    document_tree tree(cxt);

    auto decl = tree.set_declaration();
    decl.set_attribute("version", "1.0");
    decl.set_attribute("encoding", "UTF-8");

    auto pi = tree.add_processing_instruction("xml-stylesheet");
    pi.set_attribute("type", "text/xsl");
    pi.set_attribute("href", "style.xsl");

    tree.append_prolog_comment(" before root ");
    tree.set_root({"root"});
    tree.append_epilog_comment(" after root ");

    std::string output = tree.dump(2);

    assert(output.find(R"(<?xml version="1.0" encoding="UTF-8"?>)") != std::string::npos);
    assert(output.find("xml-stylesheet") != std::string::npos);
    assert(output.find(R"(type="text/xsl")") != std::string::npos);
    assert(output.find("<!-- before root -->") != std::string::npos);
    assert(output.find("<!-- after root -->") != std::string::npos);

    // round-trip: dumped output must parse and expose the same pieces
    auto rt = load_document_tree(output);
    const_node rt_decl = rt->tree.declaration();
    assert(rt_decl.type() == node_t::declaration);
    assert(rt_decl.attribute("version") == "1.0");
    assert(rt_decl.attribute("encoding") == "UTF-8");

    const_node rt_pi = rt->tree.processing_instruction("xml-stylesheet");
    assert(rt_pi.type() == node_t::processing_instruction);
    assert(rt_pi.attribute("href") == "style.xsl");
}

void test_mutable_namespaces()
{
    orcus::xmlns_repository repo;
    orcus::xmlns_context cxt = repo.create_context();
    document_tree tree(cxt);

    orcus::xmlns_id_t ns_p = cxt.push("p", "http://example.com/p");
    orcus::xmlns_id_t ns_q = cxt.push("q", "http://example.com/q");

    auto root = tree.set_root({ns_p, "root"});
    root.declare_namespace("p", ns_p);
    root.declare_namespace("q", ns_q);

    auto child = root.append_element({ns_q, "child"});
    child.set_attribute(entity_name{ns_q, "id"}, "x");

    std::string output = tree.dump(0);

    assert(output.find(R"(<p:root)") != std::string::npos);
    assert(output.find(R"(xmlns:p="http://example.com/p")") != std::string::npos);
    assert(output.find(R"(xmlns:q="http://example.com/q")") != std::string::npos);
    assert(output.find(R"(<q:child)") != std::string::npos);
    assert(output.find(R"(q:id="x")") != std::string::npos);

    // default-namespace case
    document_tree tree2(cxt);
    orcus::xmlns_id_t ns_default = cxt.push("", "http://example.com/default");
    auto root2 = tree2.set_root({ns_default, "root"});
    root2.declare_namespace("", ns_default);
    std::string output2 = tree2.dump(0);
    assert(output2.find(R"(xmlns="http://example.com/default")") != std::string::npos);
}

void test_mutable_non_element_throws()
{
    orcus::xmlns_repository repo;
    orcus::xmlns_context cxt = repo.create_context();
    document_tree tree(cxt);

    auto pi = tree.add_processing_instruction("target");

    bool threw = false;
    try
    {
        pi.append_element({"child"});
    }
    catch (const std::invalid_argument&)
    {
        threw = true;
    }
    assert(threw);

    threw = false;
    try
    {
        pi.append_content("text"); // append_content also requires element
    }
    catch (const std::invalid_argument&)
    {
        threw = true;
    }
    assert(threw);

    threw = false;
    try
    {
        // the entity_name set_attribute overload is element-only
        pi.set_attribute(entity_name{"version"}, "1.0");
    }
    catch (const std::invalid_argument&)
    {
        threw = true;
    }
    assert(threw);
}

void test_mutable_reset_then_load()
{
    orcus::xmlns_repository repo;
    orcus::xmlns_context cxt = repo.create_context();
    document_tree tree(cxt);

    // build something first
    tree.set_root({"original"}).append_element({"child"});
    assert(tree.root().name() == entity_name("original"));

    // load() should replace the previous root
    tree.load(R"(<?xml version="1.0"?><fresh><a/></fresh>)");
    assert(tree.root().name() == entity_name("fresh"));
    assert(tree.root().child_count() == 1);
}

int main()
{
    test_encoded_attr();
    test_declaration();
    test_attributes();
    test_element_hierarchy();
    test_comments();
    test_mutable_build_from_scratch();
    test_mutable_attribute_insert_update();
    test_mutable_declaration_pi_and_comments();
    test_mutable_namespaces();
    test_mutable_non_element_throws();
    test_mutable_reset_then_load();

    return EXIT_SUCCESS;
}
