/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include <orcus/sax_ns_parser.hpp>
#include <orcus/dom_tree.hpp>
#include <orcus/xml_namespace.hpp>
#include <orcus/stream.hpp>

#include <cstdlib>
#include <cassert>
#include <iostream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

using namespace orcus;

class sax_handler_encoded_attrs
{
    std::vector<sax::parser_attribute> m_attrs;

public:
    void doctype(const sax::doctype_declaration&) {}

    void start_declaration(std::string_view) {}

    void end_declaration(std::string_view)
    {
        m_attrs.clear();
    }

    void start_element(const sax::parser_element&) {}

    void end_element(const sax::parser_element&) {}

    void characters(std::string_view, bool) {}

    void attribute(const sax::parser_attribute& attr)
    {
        m_attrs.push_back(attr);
    }

    bool check(const std::vector<std::string>& expected) const
    {
        if (m_attrs.size() != expected.size())
        {
            std::cerr << "unexpected attribute count." << std::endl;
            return false;
        }

        for (size_t i = 0, n = m_attrs.size(); i < n; ++i)
        {
            if (m_attrs[i].value != expected[i].c_str())
            {
                std::cerr << "expected attribute value: " << expected[i]
                    << "  actual attribute value: " << m_attrs[i].value
                    << std::endl;
                return false;
            }
        }

        return true;
    }
};

const char* sax_parser_test_dirs[] = {
    SRCDIR"/test/xml/simple/",
    SRCDIR"/test/xml/encoded-char/",
    SRCDIR"/test/xml/default-ns/",
    SRCDIR"/test/xml/ns-alias-1/",
    SRCDIR"/test/xml/bom/",
    SRCDIR"/test/xml/custom-decl-1/",
    SRCDIR"/test/xml/cdata-1/",
    SRCDIR"/test/xml/single-quote/",
    SRCDIR"/test/xml/no-decl-1/",
    SRCDIR"/test/xml/underscore-identifier/",
    SRCDIR"/test/xml/self-closing-root/",
    SRCDIR"/test/xml/utf8-1/",
    SRCDIR"/test/xml/utf8-2/",
};

const char* sax_parser_parse_only_test_dirs[] = {
    SRCDIR"/test/xml/parse-only/rss/"
};

void parse_file(dom::document_tree& tree, const char* filepath)
{
    std::cout << "testing " << filepath << std::endl;
    auto content = test::to_file_content(filepath);
    assert(!content.empty());

    tree.load(content.str());
}

bool compare_vs_expected(std::string_view actual, std::string_view expected)
{
    actual = trim(actual);
    expected = trim(expected);

    if (actual != expected)
    {
        auto pos = locate_first_different_char(actual, expected);
        std::cout << create_parse_error_output(actual, pos) << std::endl;
        return false;
    }

    return true;
}

void test_xml_sax_parser()
{
    ORCUS_TEST_FUNC_SCOPE;

    size_t n = sizeof(sax_parser_test_dirs)/sizeof(sax_parser_test_dirs[0]);
    for (size_t i = 0; i < n; ++i)
    {
        const char* dir = sax_parser_test_dirs[i];
        fs::path dir_path(dir);
        fs::path file = dir_path / "input.xml";

        std::cout << "testing " << file << std::endl;

        auto content = test::to_file_content(file);
        assert(!content.empty());

        xmlns_repository repo;
        xmlns_context cxt = repo.create_context();
        dom::document_tree tree(cxt);
        tree.load(content.str());

        // Get the compact form of the content.
        std::ostringstream os;
        tree.dump_compact(os);
        std::string compact = os.str();

        // Load the check form.
        file = dir_path / "check.txt";
        file_content check(file);
        std::string_view psource(compact);
        std::string_view pcheck = check.str();

        // They must be equal, minus preceding or trailing spaces (if any).
        assert(trim(psource) == trim(pcheck));
    }
}

void test_xml_sax_parser_read_only()
{
    ORCUS_TEST_FUNC_SCOPE;

    size_t n = sizeof(sax_parser_parse_only_test_dirs)/sizeof(sax_parser_parse_only_test_dirs[0]);
    for (size_t i = 0; i < n; ++i)
    {
        const char* dir = sax_parser_parse_only_test_dirs[i];
        std::string dir_path(dir);
        std::string file = dir_path;
        file.append("input.xml");

        xmlns_repository repo;
        xmlns_context cxt = repo.create_context();
        dom::document_tree tree(cxt);
        parse_file(tree, file.c_str());
    }
}

void test_xml_declarations()
{
    ORCUS_TEST_FUNC_SCOPE;

    const char* file_path = SRCDIR"/test/xml/custom-decl-1/input.xml";
    xmlns_repository repo;
    xmlns_context cxt = repo.create_context();
    dom::document_tree dom(cxt);
    parse_file(dom, file_path);

    // Make sure we parse the custom declaration correctly.
    dom::const_node decl = dom.declaration("mso-application");
    assert(decl.type() == dom::node_t::declaration);
    assert(decl.attribute("progid") == "Excel.Sheet");
}

void test_xml_dtd()
{
    ORCUS_TEST_FUNC_SCOPE;

    struct {
        const char* file_path;
        sax::doctype_declaration::keyword_type keyword;
        const char* root_element;
        const char* fpi;
        const char* uri;
    } tests[] = {
        { SRCDIR"/test/xml/doctype/html.xml", sax::doctype_declaration::keyword_type::dtd_public,
          "html", "-//W3C//DTD XHTML 1.0 Transitional//EN", "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd" }
    };

    xmlns_repository repo;
    size_t n = sizeof(tests)/sizeof(tests[0]);

    for (size_t i = 0; i < n; ++i)
    {
        const char* file_path = tests[i].file_path;
        xmlns_context cxt = repo.create_context();
        dom::document_tree dom(cxt);
        parse_file(dom, file_path);
        const sax::doctype_declaration* dtd = dom.get_doctype();
        assert(dtd);
        assert(dtd->keyword == tests[i].keyword);
        assert(dtd->root_element == tests[i].root_element);
        assert(dtd->fpi == tests[i].fpi);
        if (tests[i].uri)
        {
            assert(dtd->uri == tests[i].uri);
        }
    }
}

void test_xml_encoded_attrs()
{
    ORCUS_TEST_FUNC_SCOPE;

    const char* filepath = SRCDIR"/test/xml/encoded-attrs/test1.xml";

    std::cout << "testing " << filepath << std::endl;
    auto content = test::to_file_content(filepath);
    assert(!content.empty());

    sax_handler_encoded_attrs hdl;
    sax_parser<sax_handler_encoded_attrs> parser(content.str(), hdl);
    parser.parse();

    std::vector<std::string> expected;
    expected.push_back("1 & 2");
    expected.push_back("3 & 4");
    expected.push_back("5 & 6");
    assert(hdl.check(expected));
}

void test_xml_lint()
{
    ORCUS_TEST_FUNC_SCOPE;

    const fs::path test_dirs[] = {
        SRCDIR"/test/xml-lint/namespace-basic",
        SRCDIR"/test/xml-lint/namespace-multi",
        SRCDIR"/test/xml-lint/namespace-xml-1",
        SRCDIR"/test/xml-lint/attributes",
        SRCDIR"/test/xml-lint/nested",
    };

    orcus::xmlns_repository repo;

    for (const auto& test_dir : test_dirs)
    {
        std::cout << test_dir << std::endl;
        auto content = test::to_file_content(test_dir / "input.xml");
        assert(!content.empty());

        orcus::xmlns_context cxt = repo.create_context();
        orcus::dom::document_tree tree{cxt};
        tree.load(content.str());

        // Test dump with different indent values
        constexpr std::string_view prefix = "output-indent";
        for (const auto& entry : fs::directory_iterator(test_dir))
        {
            if (entry.path().extension() != ".xml")
                continue;

            const std::string stem = entry.path().stem().string();
            if (stem.find(prefix) != 0)
                continue;

            // Extract indent value from filename like "output-indent2.xml"
            std::size_t indent = std::stoi(stem.substr(prefix.length()));

            auto expected = test::to_file_content(entry.path());
            assert(!expected.empty());

            auto actual = tree.dump(indent);
            bool res = compare_vs_expected(actual, expected.str());
            assert(res);
        }
    }
}

int main()
{
    test_xml_sax_parser();
    test_xml_sax_parser_read_only();
    test_xml_declarations();
    test_xml_dtd();
    test_xml_encoded_attrs();
    test_xml_lint();

    return EXIT_SUCCESS;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
