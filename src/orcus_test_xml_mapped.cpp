/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_xml.hpp"
#include "orcus/global.hpp"
#include "orcus/sax_ns_parser.hpp"
#include "orcus/xml_namespace.hpp"
#include "orcus/stream.hpp"
#include "orcus/dom_tree.hpp"

#include "orcus/spreadsheet/factory.hpp"
#include "orcus/spreadsheet/document.hpp"

#include "xml_map_sax_handler.hpp"

#include <cstdlib>
#include <cassert>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include <boost/filesystem.hpp>

using namespace std;
using namespace orcus;
namespace fs = boost::filesystem;

namespace {

struct test_case
{
    const char* base_dir;
    bool output_equals_input;
};

const std::vector<test_case> tests =
{
    { SRCDIR"/test/xml-mapped/attribute-basic", true },
    { SRCDIR"/test/xml-mapped/attribute-namespace", true },
    { SRCDIR"/test/xml-mapped/attribute-range-self-close", true },
    { SRCDIR"/test/xml-mapped/attribute-single-element", true },
    { SRCDIR"/test/xml-mapped/attribute-single-element-2", true },
    { SRCDIR"/test/xml-mapped/content-basic", true },
    { SRCDIR"/test/xml-mapped/content-namespace", false },
    { SRCDIR"/test/xml-mapped/content-namespace-2", true },
    { SRCDIR"/test/xml-mapped/fuel-economy", true },
    { SRCDIR"/test/xml-mapped/nested-repeats", false },
    { SRCDIR"/test/xml-mapped/nested-repeats-2", false },
};

const char* temp_output_xml = "out.xml";

void dump_xml_structure(string& dump_content, string& strm, const char* filepath, xmlns_context& cxt)
{
    strm = load_file_content(filepath);
    dom::document_tree tree(cxt);
    tree.load(strm);
    ostringstream os;
    tree.dump_compact(os);
    dump_content = os.str();
}

void test_mapped_xml_import()
{
    string strm;

    for (const test_case& tc : tests)
    {
        string base_dir(tc.base_dir);
        string data_file = base_dir + "/input.xml";
        string map_file = base_dir + "/map.xml";
        string check_file = base_dir + "/check.txt";

        // Load the data file content.
        cout << "reading " << data_file << endl;
        string data_strm = load_file_content(data_file.data());

        spreadsheet::document doc;
        spreadsheet::import_factory import_fact(doc);
        spreadsheet::export_factory export_fact(doc);

        xmlns_repository repo;
        xmlns_context cxt = repo.create_context();

        // Parse the map file to define map rules, and parse the data file.
        orcus_xml app(repo, &import_fact, &export_fact);
        read_map_file(app, map_file.c_str());
        app.read_stream(data_strm.data(), data_strm.size());

        // Zero the source data stream to make sure it's completely erased off
        // memory.
        std::for_each(data_strm.begin(), data_strm.end(), [](char& c) { c = '\0'; });
        assert(data_strm[0] == '\0');
        assert(data_strm[data_strm.size()-1] == '\0');

        // Check the content of the document against static check file.
        ostringstream os;
        doc.dump_check(os);
        string loaded = os.str();
        strm = load_file_content(check_file.c_str());

        assert(!loaded.empty());
        assert(!strm.empty());

        pstring p1(loaded.data(), loaded.size()), p2(strm.data(), strm.size());

        p1 = p1.trim();
        p2 = p2.trim();
        assert(p1 == p2);

        if (tc.output_equals_input)
        {
            // Output to xml file with the linked values coming from the document.
            string out_file = temp_output_xml;
            cout << "writing to " << out_file << endl;
            {
                // Create a duplicate source XML stream.
                string data_strm_dup = load_file_content(data_file.data());
                std::ofstream file(out_file);
                assert(file);
                app.write(data_strm_dup.data(), data_strm_dup.size(), file);
            }

            // Compare the logical xml content of the output xml with the
            // input one. They should be identical.

            string dump_input, dump_output;
            string strm_data_file, strm_out_file; // Hold the stream content in memory while the namespace context is being used.
            dump_xml_structure(dump_input, strm_data_file, data_file.c_str(), cxt);
            dump_xml_structure(dump_output, strm_out_file, out_file.c_str(), cxt);
            assert(!dump_input.empty() && !dump_output.empty());

            cout << dump_input << endl;
            cout << "--" << endl;
            cout << dump_output << endl;
            assert(dump_input == dump_output);

            // Delete the temporary xml output.
            fs::remove(out_file.c_str());
        }
    }
}

} // anonymous namespace

int main()
{
    test_mapped_xml_import();
    return EXIT_SUCCESS;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
