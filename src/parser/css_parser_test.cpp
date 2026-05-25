/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/css_parser.hpp>
#include <orcus/exception.hpp>

#include <cassert>
#include <cstring>
#include <string>

void test_handler()
{
    const char* test_code = "p { background-color: white; }";

    orcus::css_handler hdl;
    orcus::css_parser<orcus::css_handler> parser(test_code, hdl);
    parser.parse();
}

void test_truncated_percent_in_hsl()
{
    // The view ends right after a digit inside hsl().  parse_percent used
    // to read *mp_char without checking has_char(); with a backing buffer
    // that holds a valid '%' at that position the buggy parser would
    // silently continue and reach the hsl() handler callback.  The fixed
    // code throws on EOF.
    const char backing[] = "a{color:hsl(0,1%,100%,1.0)}";
    std::string_view view(backing, 15); // "a{color:hsl(0,1"

    struct hsl_recorder : public orcus::css_handler
    {
        bool hsl_called = false;
        void hsl(uint8_t, uint8_t, uint8_t) { hsl_called = true; }
    };

    hsl_recorder hdl;
    orcus::css_parser<hsl_recorder> parser(view, hdl);

    bool threw = false;
    try
    {
        parser.parse();
    }
    catch (const orcus::parse_error&)
    {
        threw = true;
    }
    assert(threw);
    assert(!hdl.hsl_called);
}

int main()
{
    test_handler();
    test_truncated_percent_in_hsl();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
