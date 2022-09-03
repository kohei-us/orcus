/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include <orcus/types.hpp>

void test_character_set_t()
{
    orcus::test::stack_printer __sp__(__func__);

    using orcus::character_set_t;

    struct test_case
    {
        std::string_view input;
        character_set_t expected;
    };

    constexpr test_case tests[] = {
        { "utf-8", character_set_t::utf_8 },
        { "UTF-8", character_set_t::utf_8 },
        { "EUC-JP", character_set_t::euc_jp },
        { "GBK", character_set_t::gbk },
        { "Shift_JIS", character_set_t::shift_jis },
        { "MS_Kanji", character_set_t::shift_jis },
        { "csShiftJIS", character_set_t::shift_jis },
        { "GB2312", character_set_t::gb2312 },
    };

    for (const auto& test : tests)
    {
        assert(orcus::to_character_set(test.input) == test.expected);
    }
}

int main()
{
    test_character_set_t();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
