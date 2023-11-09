/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/orcus_xml.hpp>
#include <orcus/sax_ns_parser.hpp>
#include <orcus/xml_namespace.hpp>
#include <orcus/stream.hpp>
#include <orcus/dom_tree.hpp>

#include <orcus/spreadsheet/factory.hpp>
#include <orcus/spreadsheet/document.hpp>

#include "orcus_test_global.hpp"

#include <cstdlib>
#include <cassert>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include "filesystem_env.hpp"

using namespace orcus;

const fs::path test_base_dir(SRCDIR"/test/xml-mapped");

namespace {

void test_mapped_xml_import()
{
    ORCUS_TEST_FUNC_SCOPE;

    struct test_case
    {
        const char* base_dir;
        bool output_equals_input;
    };

    const std::vector<test_case> tests =
    {
        { SRCDIR"/test/xml-mapped/attribute-basic", true },
        { SRCDIR"/test/xml-mapped/attribute-namespace", true },
        { SRCDIR"/test/xml-mapped/attribute-namespace-2", false },
        { SRCDIR"/test/xml-mapped/attribute-range-self-close", true },
        { SRCDIR"/test/xml-mapped/attribute-single-element", true },
        { SRCDIR"/test/xml-mapped/attribute-single-element-2", true },
        { SRCDIR"/test/xml-mapped/content-basic", true },
        { SRCDIR"/test/xml-mapped/content-namespace", false },
        { SRCDIR"/test/xml-mapped/content-namespace-2", true },
        { SRCDIR"/test/xml-mapped/content-namespace-3", false },
        { SRCDIR"/test/xml-mapped/custom-labels", true },
        { SRCDIR"/test/xml-mapped/custom-labels-2", true },
        { SRCDIR"/test/xml-mapped/fuel-economy", true },
        { SRCDIR"/test/xml-mapped/nested-repeats", false },
        { SRCDIR"/test/xml-mapped/nested-repeats-2", false },
        { SRCDIR"/test/xml-mapped/nested-repeats-3", false },
        { SRCDIR"/test/xml-mapped/nested-repeats-4", false },
    };

    auto dump_xml_structure = [](const file_content& content, xmlns_context& cxt)
    {
        dom::document_tree tree(cxt);
        tree.load(content.str());
        std::ostringstream os;
        tree.dump_compact(os);
        return os.str();
    };

    const fs::path temp_output_xml = fs::temp_directory_path() / "orcus-output.xml";

    std::string strm;

    for (const test_case& tc : tests)
    {
        fs::path base_dir(tc.base_dir);
        fs::path data_file = base_dir / "input.xml";
        fs::path map_file = base_dir / "map.xml";
        fs::path check_file = base_dir / "check.txt";

        // Load the data file content.
        std::cout << "reading " << data_file.string() << std::endl;
        file_content content(data_file.string().data());
        std::string data_strm{content.str()};

        spreadsheet::range_size_t ss{1048576, 16384};
        spreadsheet::document doc{ss};
        spreadsheet::import_factory import_fact(doc);
        spreadsheet::export_factory export_fact(doc);

        xmlns_repository repo;
        xmlns_context cxt = repo.create_context();

        // Parse the map file to define map rules, and parse the data file.
        orcus_xml app(repo, &import_fact, &export_fact);
        file_content map_content(map_file.string().data());
        app.read_map_definition(map_content.str());
        app.read_stream(data_strm);

        // Zero the source data stream to make sure it's completely erased off
        // memory.
        std::for_each(data_strm.begin(), data_strm.end(), [](char& c) { c = '\0'; });
        assert(data_strm[0] == '\0');
        assert(data_strm[data_strm.size()-1] == '\0');

        // Check the content of the document against static check file.
        std::ostringstream os;
        doc.dump_check(os);
        std::string loaded = os.str();
        content.load(check_file.string().data());
        strm = content.str();

        assert(!loaded.empty());
        assert(!strm.empty());

        std::string_view p1(loaded.data(), loaded.size()), p2(strm.data(), strm.size());

        p1 = trim(p1);
        p2 = trim(p2);
        assert(p1 == p2);

        if (tc.output_equals_input)
        {
            // Output to xml file with the linked values coming from the document.
            std::cout << "writing to " << temp_output_xml << std::endl;
            {
                // Create a duplicate source XML stream.
                content.load(data_file.string());
                std::string data_strm_dup{content.str()};
                std::ofstream file(temp_output_xml.string(), std::ios::out | std::ios::trunc);
                assert(file);
                app.write(data_strm_dup, file);
            }

            // Compare the logical xml content of the output xml with the
            // input one. They should be identical.

            // Hold the stream content in memory while the namespace context is being used.
            file_content strm_data_file(data_file.string());
            file_content strm_out_file(temp_output_xml.string());
            std::string dump_input = dump_xml_structure(strm_data_file, cxt);
            std::string dump_output = dump_xml_structure(strm_out_file, cxt);
            assert(!dump_input.empty() && !dump_output.empty());

            std::cout << dump_input << std::endl;
            std::cout << "--" << std::endl;
            std::cout << dump_output << std::endl;
            assert(dump_input == dump_output);
        }
    }

    // Delete the temporary xml output.
    fs::remove(temp_output_xml);
}

void test_mapped_xml_import_no_map_definition()
{
    ORCUS_TEST_FUNC_SCOPE;

    const std::vector<fs::path> tests = {
        test_base_dir / "attribute-basic",
        test_base_dir / "attribute-namespace",
        test_base_dir / "attribute-namespace-2",
        test_base_dir / "attribute-range-self-close",
        test_base_dir / "content-basic",
        test_base_dir / "content-one-column",
        test_base_dir / "content-namespace",
        test_base_dir / "content-namespace-2",
        test_base_dir / "content-namespace-3",
        test_base_dir / "fuel-economy",
        test_base_dir / "nested-repeats",
        test_base_dir / "nested-repeats-2",
        test_base_dir / "nested-repeats-3",
        test_base_dir / "nested-repeats-4",
    };

    for (const fs::path& base_dir : tests)
    {
        fs::path input_file = base_dir / "input.xml";
        fs::path check_file = base_dir / "check-nomap.txt";

        std::cout << "reading " << input_file.string() << std::endl;

        file_content content(input_file.string().data());
        file_content expected(check_file.string().data());

        xmlns_repository repo;

        {
            // Automatically detect map definition without a map file.
            spreadsheet::range_size_t ss{1048576, 16384};
            spreadsheet::document doc{ss};
            spreadsheet::import_factory import_fact(doc);

            orcus_xml app(repo, &import_fact, nullptr);

            app.detect_map_definition(content.str());
            app.read_stream(content.str());

            test::verify_content(__FILE__, __LINE__, doc, expected.str());
        }

        {
            // Generate a map file and use it to import the XML document.
            spreadsheet::range_size_t ss{1048576, 16384};
            spreadsheet::document doc{ss};
            spreadsheet::import_factory import_fact(doc);

            orcus_xml app(repo, &import_fact, nullptr);

            std::ostringstream os;
            app.write_map_definition(content.str(), os);
            std::string map_def = os.str();
            app.read_map_definition(map_def);
            app.read_stream(content.str());

            test::verify_content(__FILE__, __LINE__, doc, expected.str());
        }
    }
}

void test_invalid_map_definition()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path invalids_dir = test_base_dir / "invalids" / "map-defs";

    const std::vector<fs::path> tests = {
        invalids_dir / "not-xml.xml",
        invalids_dir / "non-leaf-element-linked.xml",
    };

    xmlns_repository repo;

    spreadsheet::range_size_t ss{1048576, 16384};
    spreadsheet::document doc{ss};
    spreadsheet::import_factory import_fact(doc);
    orcus_xml app(repo, &import_fact, nullptr);

    for (const fs::path& test : tests)
    {
        std::cout << test.string() << std::endl;
        file_content content(test.string().data());
        doc.clear();

        try
        {
            app.read_map_definition(content.str());
            assert(!"We were expecting an exception, but didn't get one.");
        }
        catch (const invalid_map_error& e)
        {
            // Success!
            std::cout << std::endl
                << "Exception received as expected, with the following message:" << std::endl
                << std::endl
                << test::prefix_multiline_string(e.what(), "  ") << std::endl
                << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            assert(!"Wrong exception thrown.");
        }
    }
}

void test_encoding()
{
    ORCUS_TEST_FUNC_SCOPE;

    const fs::path test_dir = test_base_dir / "encoding";

    struct test_case
    {
        const fs::path path;
        character_set_t charset;
    };

    const test_case tests[] = {
        { test_dir / "utf-8.xml", character_set_t::utf_8 },
        { test_dir / "gbk.xml", character_set_t::gbk },
        { test_dir / "euc-jp.xml", character_set_t::euc_jp },
    };

    for (const auto& test : tests)
    {
        std::cout << "reading " << test.path.string() << std::endl;

        file_content content(test.path.string());

        xmlns_repository repo;

        spreadsheet::range_size_t ss{1048576, 16384};
        spreadsheet::document doc{ss};
        spreadsheet::import_factory import_fact(doc);
        orcus_xml app(repo, &import_fact, nullptr);

        app.read_stream(content.str());

        assert(import_fact.get_character_set() == test.charset);
    }
}

} // anonymous namespace

int main()
{
    test_mapped_xml_import();
    test_mapped_xml_import_no_map_definition();
    test_invalid_map_definition();
    test_encoding();

    return EXIT_SUCCESS;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
