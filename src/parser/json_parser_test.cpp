/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/json_parser.hpp>
#include <orcus/exception.hpp>

#include <cstring>
#include <string>
#include <vector>
#include <cassert>

void test_handler()
{
    const char* test_code = "{\"key1\": [1,2,3,4,5], \"key2\": 12.3}";

    orcus::json_handler hdl;
    orcus::json_parser<orcus::json_handler> parser(test_code, hdl);
    parser.parse();
}

namespace {

class string_capture_handler : public orcus::json_handler
{
public:
    std::vector<std::string> strings;

    void string(std::string_view val, bool /*transient*/)
    {
        strings.emplace_back(val);
    }
};

std::string parse_single_string(std::string_view json)
{
    string_capture_handler hdl;
    orcus::json_parser<string_capture_handler> parser(json, hdl);
    parser.parse();
    assert(hdl.strings.size() == 1);
    return hdl.strings.front();
}

bool parse_rejects(std::string_view json)
{
    string_capture_handler hdl;
    try
    {
        orcus::json_parser<string_capture_handler> parser(json, hdl);
        parser.parse();
    }
    catch (const orcus::parse_error&)
    {
        return true;
    }
    return false;
}

} // anonymous namespace

void test_unicode_escape_surrogates()
{
    // A valid UTF-16 surrogate pair (U+1F600) decodes to its 4-byte UTF-8 form.
    assert(parse_single_string("[\"\\uD83D\\uDE00\"]") == "\xF0\x9F\x98\x80");

    // The same pair after an earlier escape, so it is decoded on the buffered
    // path rather than the initial one.
    assert(parse_single_string("[\"\\t\\uD83D\\uDE00\"]") == "\x09\xF0\x9F\x98\x80");

    // A non-surrogate \u escape still works (U+00E9).
    assert(parse_single_string("[\"\\u00e9\"]") == "\xC3\xA9");

    // Lone or mismatched surrogates are rejected, not emitted as invalid UTF-8.
    assert(parse_rejects("[\"\\uD83D\"]"));        // lone high surrogate
    assert(parse_rejects("[\"\\uDE00\"]"));        // lone low surrogate
    assert(parse_rejects("[\"\\uD83Dx\"]"));       // high not followed by \u
    assert(parse_rejects("[\"\\uD83D\\u0041\"]")); // high followed by non-low-surrogate
    assert(parse_rejects("[\"\\t\\uD83D\"]"));     // lone high surrogate on the buffered path
}

int main()
{
    test_handler();
    test_unicode_escape_surrogates();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
