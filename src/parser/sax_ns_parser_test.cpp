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
    size_t len = std::strlen(test_code);
    orcus::sax_ns_handler hdl;
    orcus::xmlns_repository repo;
    orcus::xmlns_context cxt = repo.create_context();

    orcus::sax_ns_parser<orcus::sax_ns_handler> parser(test_code, len, cxt, hdl);
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
    size_t len = strlen(test_code);

    const orcus::xmlns_id_t predefined[] = { default_ns, nullptr };

    orcus::xmlns_repository repo;
    repo.add_predefined_values(predefined);

    orcus::xmlns_context cxt = repo.create_context();

    _handler hdl;
    hdl.default_ns_expected = default_ns;

    orcus::sax_ns_parser<_handler> parser(test_code, len, cxt, hdl);
    parser.parse();
}

int main()
{
    test_handler();
    test_default_attr_ns();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
