/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/sax_ns_parser.hpp"
#include "orcus/dom_tree.hpp"
#include "orcus/xml_namespace.hpp"
#include "orcus/stream.hpp"

#include <cstdlib>
#include <cassert>
#include <iostream>
#include <sstream>

using namespace orcus;
using namespace std;

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

    bool check(const vector<string>& expected) const
    {
        if (m_attrs.size() != expected.size())
        {
            cerr << "unexpected attribute count." << endl;
            return false;
        }

        for (size_t i = 0, n = m_attrs.size(); i < n; ++i)
        {
            if (m_attrs[i].value != expected[i].c_str())
            {
                cerr << "expected attribute value: " << expected[i] << "  actual attribute value: " << m_attrs[i].value << endl;
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

void parse_file(dom::document_tree& tree, const char* filepath, std::string& /*strm*/)
{
    cout << "testing " << filepath << endl;
    file_content content(filepath);
    assert(!content.empty());

    tree.load(content.str());
}

void test_xml_sax_parser()
{
    string strm;
    size_t n = sizeof(sax_parser_test_dirs)/sizeof(sax_parser_test_dirs[0]);
    for (size_t i = 0; i < n; ++i)
    {
        const char* dir = sax_parser_test_dirs[i];
        string dir_path(dir);
        string file = dir_path;
        file.append("input.xml");

        xmlns_repository repo;
        xmlns_context cxt = repo.create_context();
        dom::document_tree tree(cxt);
        parse_file(tree, file.c_str(), strm);

        // Get the compact form of the content.
        ostringstream os;
        tree.dump_compact(os);
        string content = os.str();

        // Load the check form.
        file = dir_path;
        file.append("check.txt");
        file_content check(file.data());
        std::string_view psource(content);
        std::string_view pcheck = check.str();

        // They must be equal, minus preceding or trailing spaces (if any).
        assert(trim(psource) == trim(pcheck));
    }
}

void test_xml_sax_parser_read_only()
{
    string strm;
    size_t n = sizeof(sax_parser_parse_only_test_dirs)/sizeof(sax_parser_parse_only_test_dirs[0]);
    for (size_t i = 0; i < n; ++i)
    {
        const char* dir = sax_parser_parse_only_test_dirs[i];
        string dir_path(dir);
        string file = dir_path;
        file.append("input.xml");

        xmlns_repository repo;
        xmlns_context cxt = repo.create_context();
        dom::document_tree tree(cxt);
        parse_file(tree, file.c_str(), strm);
    }
}

void test_xml_declarations()
{
    string strm;
    const char* file_path = SRCDIR"/test/xml/custom-decl-1/input.xml";
    xmlns_repository repo;
    xmlns_context cxt = repo.create_context();
    dom::document_tree dom(cxt);
    parse_file(dom, file_path, strm);

    // Make sure we parse the custom declaration correctly.
    dom::const_node decl = dom.declaration("mso-application");
    assert(decl.type() == dom::node_t::declaration);
    assert(decl.attribute("progid") == "Excel.Sheet");
}

void test_xml_dtd()
{
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
        string strm;
        xmlns_context cxt = repo.create_context();
        dom::document_tree dom(cxt);
        parse_file(dom, file_path, strm);
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
    const char* filepath = SRCDIR"/test/xml/encoded-attrs/test1.xml";

    cout << "testing " << filepath << endl;
    file_content content(filepath);
    assert(!content.empty());

    sax_handler_encoded_attrs hdl;
    sax_parser<sax_handler_encoded_attrs> parser(content.str(), hdl);
    parser.parse();

    vector<string> expected;
    expected.push_back("1 & 2");
    expected.push_back("3 & 4");
    expected.push_back("5 & 6");
    assert(hdl.check(expected));
}

int main()
{
    test_xml_sax_parser();
    test_xml_sax_parser_read_only();
    test_xml_declarations();
    test_xml_dtd();
    test_xml_encoded_attrs();

    return EXIT_SUCCESS;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
