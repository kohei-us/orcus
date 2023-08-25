/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xpath_parser.hpp"
#include "orcus/xml_namespace.hpp"

#include <iostream>
#include <cassert>

using namespace orcus;

void test_elements()
{
    xmlns_repository repo;
    xmlns_context cxt = repo.create_context();

    std::string_view path("/A/B/C");

    xpath_parser parser(cxt, path.data(), path.size(), XMLNS_UNKNOWN_ID);
    auto token = parser.next();
    assert(token.name == "A");
    assert(!token.attribute);

    token = parser.next();
    assert(token.name == "B");
    assert(!token.attribute);

    token = parser.next();
    assert(token.name == "C");
    assert(!token.attribute);

    token = parser.next();
    assert(token.name.empty());
}

void test_attributes()
{
    xmlns_repository repo;
    xmlns_context cxt = repo.create_context();

    std::string_view path("/A/B/C/@foo");

    xpath_parser parser(cxt, path.data(), path.size(), XMLNS_UNKNOWN_ID);
    auto token = parser.next();
    assert(token.name == "A");
    assert(!token.attribute);

    token = parser.next();
    assert(token.name == "B");
    assert(!token.attribute);

    token = parser.next();
    assert(token.name == "C");
    assert(!token.attribute);

    token = parser.next();
    assert(token.name == "foo");
    assert(token.attribute);
}

int main()
{
    test_elements();
    test_attributes();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

