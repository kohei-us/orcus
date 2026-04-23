/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include <orcus/xml_encode.hpp>

#include <sstream>

namespace {

std::string encode(std::string_view val, orcus::xml_encode_context_t cxt)
{
    std::ostringstream os;
    orcus::write_content_encoded(os, val, cxt);
    return os.str();
}

} // anonymous namespace

void test_encode_text()
{
    ORCUS_TEST_FUNC_SCOPE;

    struct test_case
    {
        std::string_view input;
        std::string_view expected;
    };

    constexpr test_case tests[] = {
        // special characters that must be encoded
        { "&",              "&amp;" },
        { "<",              "&lt;"  },
        { ">",              "&gt;"  },
        // quote characters are not encoded in text content
        { R"(")",           R"(")" },
        { "'",              "'"    },
        // no special characters - passthrough
        { "",               ""      },
        { "hello",          "hello" },
        // mixed
        { R"(a<b>&"c')",    R"(a&lt;b&gt;&amp;"c')" },
    };

    for (const auto& t : tests)
        assert(encode(t.input, orcus::xml_encode_context_t::text) == t.expected);
}

void test_encode_attr_double_quoted()
{
    ORCUS_TEST_FUNC_SCOPE;

    struct test_case
    {
        std::string_view input;
        std::string_view expected;
    };

    constexpr test_case tests[] = {
        // special characters that must be encoded
        { "&",              "&amp;"  },
        { "<",              "&lt;"   },
        { ">",              "&gt;"   },
        { R"(")",           "&quot;" },
        // single quote is not encoded in double-quoted attributes
        { "'",              "'" },
        // no special characters - passthrough
        { "",               ""      },
        { "hello",          "hello" },
        // mixed
        { R"(a="b"&c)",     "a=&quot;b&quot;&amp;c" },
    };

    for (const auto& t : tests)
        assert(encode(t.input, orcus::xml_encode_context_t::attr_double_quoted) == t.expected);
}

void test_encode_attr_single_quoted()
{
    ORCUS_TEST_FUNC_SCOPE;

    struct test_case
    {
        std::string_view input;
        std::string_view expected;
    };

    constexpr test_case tests[] = {
        // special characters that must be encoded
        { "&",                          "&amp;"  },
        { "<",                          "&lt;"   },
        { ">",                          "&gt;"   },
        { "'",                          "&apos;" },
        // double quote is not encoded in single-quoted attributes
        { R"(")",                       R"(")" },
        // no special characters - passthrough
        { "",                           ""      },
        { "hello",                      "hello" },
        // mixed
        { R"(it's a "test" & more)",    R"(it&apos;s a "test" &amp; more)" },
    };

    for (const auto& t : tests)
        assert(encode(t.input, orcus::xml_encode_context_t::attr_single_quoted) == t.expected);
}

int main()
{
    test_encode_text();
    test_encode_attr_double_quoted();
    test_encode_attr_single_quoted();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
