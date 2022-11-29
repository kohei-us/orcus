/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include "orcus/sax_token_parser.hpp"
#include "orcus/tokens.hpp"
#include "orcus/xml_namespace.hpp"

#include <cstring>

using namespace std;
using namespace orcus;

void test_handler()
{
    const char* test_code = "<?xml version=\"1.0\"?><root/>";

    orcus::sax_token_handler hdl;
    orcus::tokens token_map(nullptr, 0);
    orcus::xmlns_repository repo;
    orcus::xmlns_context cxt = repo.create_context();
    orcus::sax_token_parser<orcus::sax_token_handler> parser(test_code, token_map, cxt, hdl);
    parser.parse();
}

void test_sax_token_parser_1()
{
    // Test XML content.
    const char* content = "<?xml version=\"1.0\"?><root><andy/><bruce/><charlie/><david/><edward/><frank/></root>";

    // Array of tokens to define for this test.
    const char* token_names[] = {
        "??",       // 0
        "andy",     // 1
        "bruce",    // 2
        "charlie",  // 3
        "david",    // 4
        "edward"    // 5
    };

    size_t token_count = std::size(token_names);

    // Token constants.
    const xml_token_t op_andy    = 1;
    const xml_token_t op_bruce   = 2;
    const xml_token_t op_charlie = 3;
    const xml_token_t op_david   = 4;
    const xml_token_t op_edward  = 5;

    struct check
    {
        const char* raw_name;
        xml_token_t token;
        bool start_element;
    };

    // Expected outcome.
    const check checks[] = {
        { "root",    XML_UNKNOWN_TOKEN, true  }, // name not on the master token list.
        { "andy",    op_andy,           true  },
        { "andy",    op_andy,           false },
        { "bruce",   op_bruce,          true  },
        { "bruce",   op_bruce,          false },
        { "charlie", op_charlie,        true  },
        { "charlie", op_charlie,        false },
        { "david",   op_david,          true  },
        { "david",   op_david,          false },
        { "edward",  op_edward,         true  },
        { "edward",  op_edward,         false },
        { "frank",   XML_UNKNOWN_TOKEN, true  }, // name not on the master token list.
        { "frank",   XML_UNKNOWN_TOKEN, false }, // name not on the master token list.
        { "root",    XML_UNKNOWN_TOKEN, false }, // name not on the master token list.
    };

    class handler
    {
        const check* mp_head;
        const check* mp_check;
    public:
        handler(const check* p) : mp_head(p), mp_check(p) {}

        void declaration(const orcus::xml_declaration_t&) {}

        void start_element(const orcus::xml_token_element_t& elem)
        {
            assert(std::string_view(mp_check->raw_name) == elem.raw_name);
            assert(mp_check->token == elem.name);
            assert(mp_check->start_element);
            ++mp_check;
        }

        void end_element(const orcus::xml_token_element_t& elem)
        {
            assert(std::string_view(mp_check->raw_name) == elem.raw_name);
            assert(mp_check->token == elem.name);
            assert(!mp_check->start_element);
            ++mp_check;
        }

        void characters(std::string_view /*val*/, bool /*transient*/) {}

        size_t get_token_count() const
        {
            return std::distance(mp_head, mp_check);
        }
    };

    handler hdl(checks);
    tokens token_map(token_names, token_count);
    xmlns_repository ns_repo;
    xmlns_context ns_cxt = ns_repo.create_context();
    sax_token_parser<handler> parser(content, token_map, ns_cxt, hdl);
    parser.parse();

    assert(hdl.get_token_count() == std::size(checks));
}

void test_unicode_string()
{
    const char* content1 = "<?xml version=\"1.0\"?><root>&#x0021;</root>";
    const char* content2 = "<?xml version=\"1.0\"?><root>&#x00B6;</root>";
    const char* content3 = "<?xml version=\"1.0\"?><root>&#x20B9;</root>";

    class handler
    {
        std::string_view str;
    public:
        handler(std::string_view _str):
            str(_str)
            {}

        void declaration(const orcus::xml_declaration_t&) {}

        void start_element(const orcus::xml_token_element_t& /*elem*/)
        {
        }

        void end_element(const orcus::xml_token_element_t& /*elem*/)
        {
        }

        void characters(std::string_view val, bool /*transient*/)
        {
            std::cout << "charachters:" << std::endl;
            std::cout << val << std::endl;
            assert(val == str);
        }
    };

    const char* token_names[] = {
        "???",
    };
    size_t token_count = std::size(token_names);

    tokens token_map(token_names, token_count);
    xmlns_repository ns_repo;
    xmlns_context ns_cxt = ns_repo.create_context();
    handler hdl(u8"\u0021");
    sax_token_parser<handler> parser1(content1, token_map, ns_cxt, hdl);
    parser1.parse();
    hdl = handler(u8"\u00B6");
    sax_token_parser<handler> parser2(content2, token_map, ns_cxt, hdl);
    parser2.parse();
    hdl = handler(u8"\u20B9");
    sax_token_parser<handler> parser3(content3, token_map, ns_cxt, hdl);
    parser3.parse();
}

void test_declaration()
{
    class handler
    {
        xml_declaration_t& m_decl;
    public:
        handler(xml_declaration_t& decl) : m_decl(decl) {}

        void declaration(const xml_declaration_t& decl)
        {
            m_decl = decl;
        }

        void start_element(const xml_token_element_t&) {}
        void end_element(const xml_token_element_t&) {}
        void characters(std::string_view, bool) {}
    };

    std::vector<const char*> token_names = {};
    tokens token_map(token_names.data(), token_names.size());
    xmlns_repository ns_repo;
    xmlns_context ns_cxt = ns_repo.create_context();

    struct check
    {
        std::string content;
        xml_declaration_t decl;
    };

    std::vector<check> checks =
    {
        {
            "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"no\"?><root/>",
            { 1, 0, character_set_t::utf_8, false }
        },
        {
            "<?xml version=\"1.1\" encoding=\"windows-1253\" standalone=\"yes\"?><root/>",
            { 1, 1, character_set_t::windows_1253, true }
        },
        {
            "<?xml version=\"2.0\" encoding=\"US-ASCII\" standalone=\"yes\"?><root/>",
            { 2, 0, character_set_t::us_ascii, true }
        },
    };

    for (const check& c : checks)
    {
        xml_declaration_t decl;
        handler hdl(decl);
        sax_token_parser<handler> parser(c.content, token_map, ns_cxt, hdl);
        parser.parse();

        assert(decl == c.decl);
    }
}

int main()
{
    test_handler();
    test_sax_token_parser_1();
    test_unicode_string();
    test_declaration();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
