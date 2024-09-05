/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include "xls_filter_utils.hpp"

#include <iostream>

namespace ss = orcus::spreadsheet;

using orcus::detail::xls_filter_value_parser;

namespace {

struct input_type
{
    ss::auto_filter_op_t op;
    std::string_view value;
};

struct check
{
    input_type before;
    xls_filter_value_parser::result_type after;
};

}

void test_xls_filter_value_parser()
{
    ORCUS_TEST_FUNC_SCOPE;

    const check checks[] =
    {
        {
            { ss::auto_filter_op_t::equal, "foo" },       // input
            { ss::auto_filter_op_t::equal, "foo", false } // expected output
        },
        {
            { ss::auto_filter_op_t::not_equal, "some-value" },
            { ss::auto_filter_op_t::not_equal, "some-value", false }
        },
        {
            { ss::auto_filter_op_t::equal, "*" },
            { ss::auto_filter_op_t::equal, ".*", true }
        },
        {
            { ss::auto_filter_op_t::equal, "*end-value" },
            { ss::auto_filter_op_t::end_with, "end-value", false }
        },
        {
            { ss::auto_filter_op_t::not_equal, "*end-value" },
            { ss::auto_filter_op_t::not_end_with, "end-value", false }
        },
        {
            { ss::auto_filter_op_t::equal, "start-value*" },
            { ss::auto_filter_op_t::begin_with, "start-value", false }
        },
        {
            { ss::auto_filter_op_t::not_equal, "start-value*" },
            { ss::auto_filter_op_t::not_begin_with, "start-value", false }
        },
        {
            { ss::auto_filter_op_t::not_equal, "start-value*" },
            { ss::auto_filter_op_t::not_begin_with, "start-value", false }
        },
        {
            { ss::auto_filter_op_t::equal, "begin-~*-end" },
            { ss::auto_filter_op_t::equal, "begin-*-end", false }
        },
        {
            { ss::auto_filter_op_t::equal, "~*" },
            { ss::auto_filter_op_t::equal, "*", false }
        },
        {
            { ss::auto_filter_op_t::equal, "~?" },
            { ss::auto_filter_op_t::equal, "?", false }
        },
        {
            { ss::auto_filter_op_t::equal, "~*~*~*" },
            { ss::auto_filter_op_t::equal, "***", false }
        },
        {
            { ss::auto_filter_op_t::equal, "~?~?~?" },
            { ss::auto_filter_op_t::equal, "???", false }
        },
        {
            { ss::auto_filter_op_t::equal, "~~*~*~*~" },
            { ss::auto_filter_op_t::equal, "~***~", false }
        },
        {
            { ss::auto_filter_op_t::equal, "~~?~?~?~" },
            { ss::auto_filter_op_t::equal, "~???~", false }
        },
        {
            { ss::auto_filter_op_t::equal, "~*~*~**" },
            { ss::auto_filter_op_t::begin_with, "***", false }
        },
        {
            { ss::auto_filter_op_t::equal, "~?~?~?*" },
            { ss::auto_filter_op_t::begin_with, "???", false }
        },
        {
            { ss::auto_filter_op_t::equal, "*~*~*~*" },
            { ss::auto_filter_op_t::end_with, "***", false }
        },
        {
            { ss::auto_filter_op_t::equal, "*~?~?~?" },
            { ss::auto_filter_op_t::end_with, "???", false }
        },
        {
            { ss::auto_filter_op_t::equal, "*~*~*~**" },
            { ss::auto_filter_op_t::contain, "***", false }
        },
        {
            { ss::auto_filter_op_t::equal, "*~?~?~?*" },
            { ss::auto_filter_op_t::contain, "???", false }
        },
        {
            { ss::auto_filter_op_t::not_equal, "*~*~*~**" },
            { ss::auto_filter_op_t::not_contain, "***", false }
        },
        {
            { ss::auto_filter_op_t::not_equal, "*~?~?~?*" },
            { ss::auto_filter_op_t::not_contain, "???", false }
        },
        {
            { ss::auto_filter_op_t::equal, "head*tail" },
            { ss::auto_filter_op_t::equal, "head.*tail", true }
        },
        {
            { ss::auto_filter_op_t::equal, "head?tail" },
            { ss::auto_filter_op_t::equal, "head.tail", true }
        },
        {
            { ss::auto_filter_op_t::equal, "head*and*tail" },
            { ss::auto_filter_op_t::equal, "head.*and.*tail", true }
        },
        {
            { ss::auto_filter_op_t::equal, "head?and?tail" },
            { ss::auto_filter_op_t::equal, "head.and.tail", true }
        },
        {
            { ss::auto_filter_op_t::equal, "head**tail" },
            { ss::auto_filter_op_t::equal, "head.*.*tail", true }
        },
        {
            { ss::auto_filter_op_t::equal, "head??tail" },
            { ss::auto_filter_op_t::equal, "head..tail", true }
        },
    };

    xls_filter_value_parser parser;

    for (const auto& [before, after] : checks)
    {
        auto res = parser.parse(before.op, before.value);
        std::cout
            << "- input:\n"
            << "    op: " << before.op << "\n"
            << "    value: " << before.value << "\n"
            << "  expected output:\n"
            << "    op: " << after.op << "\n"
            << "    value: " << after.value << "\n"
            << "    regex: " << after.regex << "\n"
            << "  actual output:\n"
            << "    op: " << res.op << "\n"
            << "    value: " << res.value << "\n"
            << "    regex: " << res.regex << std::endl;

        assert(res.op == after.op);
        assert(res.value == after.value);
        assert(res.regex == after.regex);
    }
}

int main()
{
    test_xls_filter_value_parser();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
