/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_xml.hpp"
#include "orcus/xml_namespace.hpp"
#include "orcus/xml_structure_tree.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/stream.hpp"
#include "orcus/global.hpp"

#include "xml_map_sax_handler.hpp"
#include "orcus_filter_global.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <mdds/sorted_string_map.hpp>

using namespace orcus;
using namespace std;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace {

namespace output_mode {

enum class type {
    unknown,
    dump_document,
    transform_xml,
    dump_document_check,
    xml_structure,
};

typedef mdds::sorted_string_map<type> map_type;

// Keys must be sorted.
const std::vector<map_type::entry> entries =
{
    { ORCUS_ASCII("dump"),          type::dump_document       },
    { ORCUS_ASCII("dump-check"),    type::dump_document_check },
    { ORCUS_ASCII("transform"),     type::transform_xml       },
    { ORCUS_ASCII("xml-structure"), type::xml_structure       },
};

const map_type& get()
{
    static map_type mt(entries.data(), entries.size(), type::unknown);
    return mt;
}

} // namespace output_mode

std::string to_string(output_mode::type t)
{
    for (const output_mode::map_type::entry& e : output_mode::entries)
        if (t == e.value)
            return std::string(e.key, e.keylen);

    return std::string();
}

void print_usage(ostream& os, const po::options_description& desc)
{
    os << "Usage: orcus-xml [OPTIONS] FILE" << endl << endl;
    os << desc;
}

std::string build_output_help_text()
{
    std::ostringstream os;
    os << "Output directory path, or output file in the "
        << to_string(output_mode::type::dump_document_check) << " mode.";
    return os.str();
}

std::string build_mode_help_text()
{
    std::ostringstream os;
    os << "Mode of operation. Select one of the following options: ";
    auto it = output_mode::entries.cbegin(), ite = output_mode::entries.cend();
    --ite;

    for (; it != ite; ++it)
        os << std::string(it->key, it->keylen) << ", ";

    os << "or " << std::string(it->key, it->keylen) << ".";
    return os.str();
}

std::string build_map_help_text()
{
    std::ostringstream os;
    os << "Path to the map file. A map file is required for all modes except for the "
        << to_string(output_mode::type::xml_structure) << " mode.";
    return os.str();
}

} // anonymous namespace

int main(int argc, char** argv)
{
    po::options_description desc("Options");
    desc.add_options()
        ("help,h", "Print this help.")
        ("mode", po::value<std::string>(), build_mode_help_text().data())
        ("map,m", po::value<std::string>(), build_map_help_text().data())
        ("output,o", po::value<std::string>(), build_output_help_text().data())
        ("output-format,f", po::value<string>(), gen_help_output_format().data())
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

    std::string output;
    if (vm.count("output"))
        output = vm["output"].as<std::string>();

    try
    {
        xmlns_repository repo;
        file_content content(input_path.data());

        if (mode == output_mode::type::xml_structure)
        {
            xmlns_context cxt = repo.create_context();
            xml_structure_tree tree(cxt);
            tree.parse(content.data(), content.size());

            if (output.empty())
            {
                tree.dump_compact(cout);
                return EXIT_SUCCESS;
            }

            ofstream file(output);
            if (!file)
            {
                cerr << "failed to create output file: " << output << endl;
                return EXIT_FAILURE;
            }

            tree.dump_compact(file);

            return EXIT_SUCCESS;
        }

        if (!vm.count("map") || map_path.empty())
        {
            cerr << "Map file is required, but is not given." << endl;
            print_usage(cout, desc);
            return EXIT_FAILURE;
        }

        if (!fs::is_regular_file(map_path))
        {
            cerr << "'" << map_path << "' is not a valid map file." << endl;
            return EXIT_FAILURE;
        }

        spreadsheet::document doc;
        spreadsheet::import_factory import_fact(doc);
        spreadsheet::export_factory export_fact(doc);

        orcus_xml app(repo, &import_fact, &export_fact);

        read_map_file(app, map_path.data());
        app.read_stream(content.data(), content.size());

        switch (mode)
        {
            case output_mode::type::dump_document:
            {
                dump_format_t format = dump_format_t::unknown;

                if (vm.count("output-format"))
                {
                    s = vm["output-format"].as<std::string>();
                    format = to_dump_format_enum(s.data(), s.size());
                }

                if (format == dump_format_t::unknown)
                {
                    std::cerr << "Unsupported output format: '" << s << "'" << endl;
                    return EXIT_FAILURE;
                }

                doc.dump(format, output);
                break;
            }
            case output_mode::type::transform_xml:
            {
                if (output.empty())
                {
                    cout << "output xml file name not provided" << endl;
                    print_usage(cout, desc);
                    return EXIT_FAILURE;
                }

                ofstream file(output);
                if (!file)
                {
                    cerr << "failed to create output file: " << output << endl;
                    return EXIT_FAILURE;
                }

                // Write transformed xml content to file.
                app.write(content.data(), content.size(), file);
                break;
            }
            case output_mode::type::dump_document_check:
            {
                if (output.empty())
                {
                    doc.dump_check(cout);
                    break;
                }

                ofstream file(output);
                if (!file)
                {
                    cerr << "failed to create output file: " << output << endl;
                    return EXIT_FAILURE;
                }

                doc.dump_check(file);
                break;
            }
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
