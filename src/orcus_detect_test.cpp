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
#include <orcus/orcus_json.hpp>
#include <orcus/orcus_xml.hpp>
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

std::vector<fs::path> collection_target_dirs(const fs::path& base_dir, std::string_view skip_dir)
{
    std::vector<fs::path> target_dirs;

    for (const auto& entry : fs::directory_iterator(base_dir))
    {
        if (!entry.is_directory())
            continue;

        auto p = entry.path();
        if (p.filename() == skip_dir)
            continue;

        target_dirs.push_back(std::move(p));
    }

    return target_dirs;
}

std::vector<fs::path> collection_target_files(
    const std::vector<fs::path>& target_dirs, std::string_view ext)
{
    std::vector<fs::path> target_files;

    for (const auto& target_dir : target_dirs)
    {
        for (const auto& entry : fs::recursive_directory_iterator(target_dir))
        {
            if (!entry.is_regular_file())
                continue;

            auto p = entry.path();
            if (p.extension() != ext)
                continue;

            target_files.push_back(std::move(p));
        }
    }

    return target_files;
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

void test_json_detect_positive()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto json_base_dir = test_base_dir / "json";

    std::vector<fs::path> target_dirs = collection_target_dirs(json_base_dir, "validation");
    assert(!target_dirs.empty());

    std::vector<fs::path> targets = collection_target_files(target_dirs, ".json");
    assert(!targets.empty());

    for (const auto& target : targets)
    {
        std::cout << target << std::endl;
        auto fc = orcus::test::to_file_content(target.string());
        auto strm = fc.str();
        bool valid = orcus::orcus_json::detect(
            reinterpret_cast<const unsigned char*>(strm.data()), strm.size());

        assert(valid);
    }
}

void test_json_detect_negative()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::vector<fs::path> targets = {
        "css/test.css",
        "css/basic6.css",
        "csv/split-sheet/input.csv",
        "csv/simple-numbers/input.csv",
        "gnumeric/text-alignment/input.gnumeric",
        "ods/raw-values-1/input.ods",
        "xls-xml/empty-rows/input.xml",
        "yaml/basic2/input.yaml",
    };

    for (const auto& target : targets)
    {
        auto p = test_base_dir / target;
        std::cout << p << std::endl;
        assert(fs::is_regular_file(p));

        auto fc = orcus::test::to_file_content(p.native());
        auto strm = fc.str();
        bool valid = orcus::orcus_json::detect(
            reinterpret_cast<const unsigned char*>(strm.data()), strm.size());

        assert(!valid);
    }
}

void test_xml_detect_positive()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto json_base_dir = test_base_dir / "xml";

    std::vector<fs::path> target_dirs = collection_target_dirs(json_base_dir, "invalids");
    assert(!target_dirs.empty());

    std::vector<fs::path> target_files = collection_target_files(target_dirs, ".xml");
    assert(!target_files.empty());

    for (const auto& p : target_files)
    {
        std::cout << p << std::endl;
        auto fc = orcus::test::to_file_content(p.string());
        auto strm = fc.str();
        bool valid = orcus::orcus_xml::detect(
            reinterpret_cast<const unsigned char*>(strm.data()), strm.size());

        assert(valid);
    }
}

void test_xml_detect_negative()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::vector<fs::path> targets = {
        "css/basic6.css",
        "csv/split-sheet/input.csv",
        "csv/simple-numbers/input.csv",
        "gnumeric/text-alignment/input.gnumeric",
        "ods/raw-values-1/input.ods",
        "xlsx/raw-values-1/input.xlsx",
        "yaml/basic2/input.yaml",
        "json/swagger/input.json",
    };

    for (const auto& target : targets)
    {
        auto p = test_base_dir / target;
        std::cout << p << std::endl;
        assert(fs::is_regular_file(p));

        auto fc = orcus::test::to_file_content(p.native());
        auto strm = fc.str();
        bool valid = orcus::orcus_xml::detect(
            reinterpret_cast<const unsigned char*>(strm.data()), strm.size());

        assert(!valid);
    }
}

int main()
{
    orcus::bootstrap_program();
    test_format_detection();
    test_json_detect_positive();
    test_json_detect_negative();
    test_xml_detect_positive();
    test_xml_detect_negative();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
