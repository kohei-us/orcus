/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_test_global.hpp"
#include "filesystem_env.hpp"
#include "filter_env.hpp"
#include "cli_global.hpp"

#include <orcus/stream.hpp>
#include <orcus/format_detection.hpp>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

const fs::path test_base_dir(SRCDIR"/test");

bool test_filter(std::string_view name)
{
    static const std::unordered_set<std::string_view> filter_test_dirs = {
#if GNUMERIC_ENABLED
        "gnumeric",
#endif
#if ODS_ENABLED
        "ods",
#endif
#if PARQUET_ENABLED
        "parquet",
#endif
#if XLS_XML_ENABLED
        "xls-xml",
#endif
#if XLSX_ENABLED
        "xlsx",
#endif
    };

    return filter_test_dirs.count(name) > 0;
}


orcus::format_t expected_format(std::string_view parent_dir_name)
{
    static const std::unordered_map<std::string_view, orcus::format_t> map = {
        { "xlsx", orcus::format_t::xlsx },
        { "ods", orcus::format_t::ods },
        { "xls-xml", orcus::format_t::xls_xml },
        { "gnumeric", orcus::format_t::gnumeric },
        { "csv", orcus::format_t::csv },
        { "parquet", orcus::format_t::parquet },
    };

    auto it = map.find(parent_dir_name);
    return it == map.end() ? orcus::format_t::unknown : it->second;
}

void test_format_detection()
{
    ORCUS_TEST_FUNC_SCOPE;

    for (const auto& child_dir : fs::directory_iterator(test_base_dir))
    {
        if (!child_dir.is_directory())
            continue;

        auto filter_name = child_dir.path().stem().string();
        if (!test_filter(filter_name))
            continue; // skip a non-filter test directory

        orcus::format_t expected = expected_format(filter_name);
        assert(expected != orcus::format_t::unknown);

        const auto detect_dir = child_dir.path() / "detect";
        if (!fs::is_directory(detect_dir))
            continue;

        for (const auto& input_entry : fs::directory_iterator(detect_dir))
        {
            if (!input_entry.is_regular_file())
                continue;

            auto input = input_entry.path();
            auto input_s = input.native();
            orcus::test::print_path(input_s);

            auto fc = orcus::test::to_file_content(input_s);
            orcus::format_t detected = orcus::detect(fc.str());
            assert(detected == expected);
        }
    }
}

int main()
{
    orcus::bootstrap_program();
    test_format_detection();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
