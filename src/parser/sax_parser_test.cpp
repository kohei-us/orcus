/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include <orcus/sax_parser.hpp>
#include <cstring>

using namespace std;

void test_handler()
{
    const char* test_code = "<?xml version=\"1.0\"?><root/>";

    orcus::sax_handler hdl;
    orcus::sax_parser<orcus::sax_handler> parser(test_code, hdl);
    parser.parse();
}

void test_attr_equal_with_whitespace()
{
    struct _handler : public orcus::sax_handler {};

    const char* content =
        "<?xml version=\"1.0\"?>"
        "<root attr1='some value' attr2 = \"some value\"/>"
    ;

    _handler hdl;
    orcus::sax_parser<_handler> parser(content, hdl);
    parser.parse();
}

void test_attr_with_encoded_chars_single_quotes()
{
    struct _handler : public orcus::sax_handler
    {
        void attribute(const orcus::sax::parser_attribute& attr)
        {
            if (attr.name == "attr1")
                assert(attr.value == "'some value'");
        }
    };

    const char* content =
        "<?xml version=\"1.0\"?>"
        "<root attr1='&apos;some value&apos;'/>"
    ;

    _handler hdl;
    orcus::sax_parser<_handler> parser(content, hdl);
    parser.parse();
}

int main()
{
    test_handler();
    test_attr_equal_with_whitespace();
    test_attr_with_encoded_chars_single_quotes();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
