/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include "orcus/threaded_sax_token_parser.hpp"
#include "orcus/tokens.hpp"
#include "orcus/xml_namespace.hpp"
#include "orcus/parser_base.hpp"
#include "orcus/stream.hpp"

#include <cstring>

using namespace std;
using namespace orcus;

void test_sax_token_parser_1()
{
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

    tokens token_map(token_names, token_count);
    xmlns_repository ns_repo;
    xmlns_context ns_cxt = ns_repo.create_context();

    {
        // Test XML content.
        const char* content = "<?xml version=\"1.0\"?><root><andy/><bruce/><charlie/><david/><edward/><frank/></root>";
        size_t content_size = strlen(content);

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
        threaded_sax_token_parser<handler> parser(content, content_size, token_map, ns_cxt, hdl, 1, 100);
        parser.parse();

        assert(hdl.get_token_count() == std::size(checks));
    }

    {
        // This content intentially contains invalid XML part at offset 28.
        const char* content = "<?xml version=\"1.0\"?><root><<andy/><bruce/><charlie/><david/><edward/><frank/></root>";
        size_t content_size = strlen(content);

        class handler
        {
        public:
            handler() {}

            void start_element(const orcus::xml_token_element_t& /*elem*/) {}

            void end_element(const orcus::xml_token_element_t& /*elem*/) {}

            void characters(std::string_view /*val*/, bool /*transient*/) {}
        };

        try
        {
            handler hdl;
            threaded_sax_token_parser<handler> parser(content, content_size, token_map, ns_cxt, hdl, 1, 100);
            parser.parse();
            assert(!"An exception was expected, but one was not thrown.");
        }
        catch (const malformed_xml_error& e)
        {
            assert(e.offset() == 28u);
        }
        catch (const std::exception&)
        {
            assert(!"Wrong exception was thrown!");
        }
    }

    {
        // Test XML content.
        const char* content = "<?xml version=\"1.0\"?><root><andy/><bruce/><charlie/><david/><edward/><frank/></root>";
        size_t content_size = strlen(content);

        class mock_exception : public std::exception {};

        class handler
        {
        public:
            handler() {}

            void start_element(const orcus::xml_token_element_t& /*elem*/) {}

            void end_element(const orcus::xml_token_element_t& /*elem*/)
            {
                throw mock_exception();
            }

            void characters(std::string_view /*val*/, bool /*transient*/) {}
        };

        handler hdl;
        threaded_sax_token_parser<handler> parser(content, content_size, token_map, ns_cxt, hdl, 1, 100);

        try
        {
            parser.parse();
            assert(!"A mock exception was expected but not thrown.");
        }
        catch (const mock_exception&)
        {
            // expected.
        }
    }
}

int main()
{
    test_sax_token_parser_1();
    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
