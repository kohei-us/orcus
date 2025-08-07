/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include "filesystem_env.hpp"

#include <orcus/orcus_json.hpp>
#include <orcus/stream.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/factory.hpp>
#include <orcus/exception.hpp>
#include <orcus/parser_global.hpp>
#include <orcus/format_detection.hpp>

#include <iostream>
#include <vector>
#include <cassert>
#include <sstream>

using namespace orcus;

namespace {

const std::vector<fs::path> mapped_test_dirs =
{
    SRCDIR"/test/json-mapped/array-of-arrays-basic",
    SRCDIR"/test/json-mapped/array-of-arrays-header",
    SRCDIR"/test/json-mapped/array-of-objects-basic",
    SRCDIR"/test/json-mapped/array-of-objects-header",
    SRCDIR"/test/json-mapped/nested-repeats",
    SRCDIR"/test/json-mapped/nested-repeats-2",
};

} // anonymous namespace

void test_mapped_json_import()
{
    ORCUS_TEST_FUNC_SCOPE;

    for (const auto& base_dir : mapped_test_dirs)
    {
        fs::path data_file = base_dir / "input.json";
        fs::path map_file = base_dir / "map.json";
        fs::path check_file = base_dir / "check.txt";

        std::cout << "reading " << data_file.string() << std::endl;
        file_content content(data_file.string());
        file_content map_content(map_file.string());
        file_content check_content(check_file.string());

        spreadsheet::range_size_t ss{1048576, 16384};
        spreadsheet::document doc{ss};
        spreadsheet::import_factory import_fact(doc);

        orcus_json app(&import_fact);
        app.read_map_definition(map_content.str());
        app.read_stream(content.str());

        std::ostringstream os;
        doc.dump_check(os);

        std::string actual_strm = os.str();
        std::string_view actual(actual_strm);
        std::string_view expected = check_content.str();
        actual = trim(actual);
        expected = trim(expected);
        assert(actual == expected);
    }
}

void test_mapped_json_import_auto_mapping()
{
    ORCUS_TEST_FUNC_SCOPE;

    const fs::path base_dir{SRCDIR"/test/json-mapped/auto-mapping/"};

    for (const auto& entry : fs::directory_iterator(base_dir))
    {
        if (!entry.is_directory())
            continue;

        auto test_dir = entry.path();
        std::cout << test_dir << std::endl;

        fs::path input_file = test_dir / "input.json";
        fs::path check_file = test_dir / "check.txt";

        file_content content(input_file.string());
        file_content check_content(check_file.string());

        spreadsheet::range_size_t ss{1048576, 16384};
        spreadsheet::document doc{ss};
        spreadsheet::import_factory import_fact(doc);

        auto filter = create_filter(format_t::json, &import_fact);
        assert(filter);
        assert(filter->get_name() == "json");

        filter->read_stream(content.str());

        std::ostringstream os;
        doc.dump_check(os);
        auto check_actual = os.str();
        assert(check_actual == check_content.str());
    }
}

void test_invalid_map_definition()
{
    ORCUS_TEST_FUNC_SCOPE;

    spreadsheet::range_size_t ss{1048576, 16384};
    spreadsheet::document doc{ss};
    spreadsheet::import_factory import_fact(doc);

    orcus_json app(&import_fact);
    try
    {
        app.read_map_definition("asdfdasf");
        assert(false); // We were expecting an exception, but didn't get one.
    }
    catch (const invalid_map_error&)
    {
        // Success!
    }
}

int main()
{
    test_mapped_json_import();
    test_mapped_json_import_auto_mapping();
    test_invalid_map_definition();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

