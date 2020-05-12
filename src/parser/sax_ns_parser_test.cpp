/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/sax_ns_parser.hpp>
#include <orcus/xml_namespace.hpp>

void test_handler()
{
    const char* test_code = "<?xml version=\"1.0\"?><root/>";
    size_t len = strlen(test_code);
    orcus::sax_ns_handler hdl;
    orcus::xmlns_repository repo;
    orcus::xmlns_context cxt = repo.create_context();

    orcus::sax_ns_parser<orcus::sax_ns_handler> parser(test_code, len, cxt, hdl);
    parser.parse();
}

int main()
{
    test_handler();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
