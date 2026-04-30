/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/sax_ns_parser.hpp>
#include <orcus/xml_namespace.hpp>

#include <cstring>

void test_handler()
{
    const char* test_code = "<?xml version=\"1.0\"?><root/>";
    orcus::sax_ns_handler hdl;
    orcus::xmlns_repository repo;
    orcus::xmlns_context cxt = repo.create_context();

    orcus::sax_ns_parser<orcus::sax_ns_handler> parser(test_code, cxt, hdl);
    parser.parse();
}

/**
 * Test for unqualified attribute NOT belonging to the default namespace,
 * according to
 * https://stackoverflow.com/questions/3312390/xml-default-namespaces-for-unqualified-attribute-names
 */
void test_default_attr_ns()
{
    const orcus::xmlns_id_t default_ns = "test:foo";

    struct _handler : public orcus::sax_ns_handler
    {
        orcus::xmlns_id_t default_ns_expected;

        void start_element(const orcus::sax_ns_parser_element& elem)
        {
            // All elements should belong to the default namespace.
            assert(elem.ns == default_ns_expected);
        }

        void attribute(std::string_view /*name*/, std::string_view /*val*/) {}

        void attribute(const orcus::sax_ns_parser_attribute& attr)
        {
            // Attribute's namespace should be empty.
            assert(attr.ns == orcus::XMLNS_UNKNOWN_ID);
            assert(attr.name == "attr");
            assert(attr.value == "1");
        }
    };

    const char* test_code = "<?xml version=\"1.0\"?><root xmlns='test:foo'><elem attr='1'/></root>";

    const orcus::xmlns_id_t predefined[] = { default_ns, nullptr };

    orcus::xmlns_repository repo;
    repo.add_predefined_values(predefined);

    orcus::xmlns_context cxt = repo.create_context();

    _handler hdl;
    hdl.default_ns_expected = default_ns;

    orcus::sax_ns_parser<_handler> parser(test_code, cxt, hdl);
    parser.parse();
}

/**
 * Test that an explicit redeclaration of the builtin 'xml' namespace alias
 * (which is permitted by the XML spec when the URI matches) is handled without
 * throwing and that attributes prefixed with 'xml:' resolve to the correct
 * namespace identifier.
 */
void test_builtin_xml_ns()
{
    struct _handler : public orcus::sax_ns_handler
    {
        orcus::xmlns_id_t xml_ns_id = nullptr;
        bool ns_decl_seen = false;

        void namespace_declaration(std::string_view alias, orcus::xmlns_id_t ns_id)
        {
            assert(alias == orcus::XML_BUILTIN_NS_ALIAS);
            assert(ns_id == orcus::XML_BUILTIN_NS_URI);
            ns_decl_seen = true;
        }

        void attribute(std::string_view /*name*/, std::string_view /*val*/) {}

        void attribute(const orcus::sax_ns_parser_attribute& attr)
        {
            assert(attr.ns_alias == orcus::XML_BUILTIN_NS_ALIAS);
            assert(attr.ns == orcus::XML_BUILTIN_NS_URI);
            assert(attr.name == "lang");
            assert(attr.value == "en");
        }
    };

    // Explicitly redeclare xmlns:xml — legal per the XML spec when the URI matches.
    const char* test_code =
        "<?xml version=\"1.0\"?>"
        "<root xmlns:xml=\"http://www.w3.org/XML/1998/namespace\" xml:lang=\"en\"/>";

    orcus::xmlns_repository repo;
    orcus::xmlns_context cxt = repo.create_context();

    _handler hdl;
    orcus::sax_ns_parser<_handler> parser(test_code, cxt, hdl);
    parser.parse();

    assert(hdl.ns_decl_seen);
}

int main()
{
    test_handler();
    test_default_attr_ns();
    test_builtin_xml_ns();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
