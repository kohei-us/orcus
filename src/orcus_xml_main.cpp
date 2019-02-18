/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_xml.hpp"
#include "orcus/xml_namespace.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/stream.hpp"
#include "orcus/global.hpp"

#include "xml_map_sax_handler.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <mdds/sorted_string_map.hpp>

using namespace orcus;
using namespace std;
namespace po = boost::program_options;

namespace {

void print_usage(ostream& os, const po::options_description& desc)
{
    os << "Usage: orcus-xml [OPTIONS] FILE" << endl << endl;
    os << desc;
}

namespace output_mode {

enum class type {
    unknown, dump_document, transform_xml, dump_document_check
};

typedef mdds::sorted_string_map<type> map_type;

// Keys must be sorted.
const std::vector<map_type::entry> entries =
{
    { ORCUS_ASCII("dump"),       type::dump_document       },
    { ORCUS_ASCII("dump-check"), type::dump_document_check },
    { ORCUS_ASCII("transform"),  type::transform_xml       },
};

const map_type& get()
{
    static map_type mt(entries.data(), entries.size(), type::unknown);
    return mt;
}

} // namespace output_mode

} // anonymous namespace

int main(int argc, char** argv)
{
    po::options_description desc("Options");
    desc.add_options()
        ("help,h", "Print this help.")
        ("mode", po::value<std::string>(), "Either dump, transoform, or dump-check.")
        ("map,m", po::value<std::string>(), "Path to the map file.")
        ("output,o", po::value<std::string>(), "Output file path.")
    ;

    po::options_description hidden("");
    hidden.add_options()
        ("input", po::value<string>(), "input file");

    po::positional_options_description po_desc;
    po_desc.add("input", 1);

    po::options_description cmd_opt;
    cmd_opt.add(desc).add(hidden);

    po::variables_map vm;
    try
    {
        po::store(
            po::command_line_parser(argc, argv).options(cmd_opt).positional(po_desc).run(), vm);
        po::notify(vm);
    }
    catch (const exception& e)
    {
        // Unknown options.
        cerr << e.what() << endl;
        print_usage(cout, desc);
        return EXIT_FAILURE;
    }

    if (vm.count("help"))
    {
        print_usage(cout, desc);
        return EXIT_FAILURE;
    }

    if (!vm.count("input"))
    {
        cerr << "No input file." << endl;
        print_usage(cout, desc);
        return EXIT_FAILURE;
    }

    if (!vm.count("mode"))
    {
        cerr << "Mode not specified." << endl;
        print_usage(cout, desc);
        return EXIT_FAILURE;
    }

    std::string s = vm["mode"].as<std::string>();
    output_mode::type mode = output_mode::get().find(s.data(), s.size());

    if (mode == output_mode::type::unknown)
    {
        cerr << "Unknown output mode: " << s << endl;
        print_usage(cout, desc);
        return EXIT_FAILURE;
    }

    std::string input_path = vm["input"].as<std::string>();

    std::string map_path;
    if (vm.count("map"))
        map_path = vm["map"].as<std::string>();

    try
    {
        spreadsheet::document doc;
        spreadsheet::import_factory import_fact(doc);
        spreadsheet::export_factory export_fact(doc);

        xmlns_repository repo;
        orcus_xml app(repo, &import_fact, &export_fact);

        read_map_file(app, map_path.data());
        std::string strm = load_file_content(input_path.data());
        app.read_stream(strm.data(), strm.size());

        switch (mode)
        {
            case output_mode::type::dump_document:
            {
                doc.dump_flat("./flat");
                break;
            }
            case output_mode::type::transform_xml:
            {
                if (argc <= 4)
                {
                    cout << "output xml file name not provided" << endl;
                    print_usage(cout, desc);
                    return EXIT_FAILURE;
                }

                ofstream file(argv[4]);
                if (!file)
                {
                    cerr << "failed to create output file: " << argv[4] << endl;
                    return EXIT_FAILURE;
                }

                // Write transformed xml content to file.
                app.write(strm.data(), strm.size(), file);
            }
            break;
            case output_mode::type::dump_document_check:
            {
                if (argc <= 4)
                {
                    doc.dump_check(cout);
                    break;
                }

                ofstream file(argv[4]);
                if (!file)
                {
                    cerr << "failed to create output file: " << argv[4] << endl;
                    return EXIT_FAILURE;
                }

                doc.dump_check(file);
            }
            break;
            default:
                ;
        }
    }
    catch (const std::exception& e)
    {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
