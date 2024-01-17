/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_xml.hpp"
#include "orcus/xml_namespace.hpp"
#include "orcus/xml_structure_tree.hpp"
#include "orcus/dom_tree.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/stream.hpp"
#include "orcus/sax_parser_base.hpp"

#include "orcus_filter_global.hpp"
#include "cli_global.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <mdds/sorted_string_map.hpp>

#include "filesystem_env.hpp"

using namespace orcus;
using namespace std;
namespace po = boost::program_options;

namespace {

namespace output_mode {

enum class type {
    unknown,
    dump,
    map,
    map_gen,
    transform_xml,
    structure,
};

using map_type = mdds::sorted_string_map<type>;

// Keys must be sorted.
constexpr map_type::entry_type entries[] = {
    { "dump",      type::dump          },
    { "map",       type::map           },
    { "map-gen",   type::map_gen       },
    { "structure", type::structure     },
    { "transform", type::transform_xml },
};

const map_type& get()
{
    static const map_type mt(entries, std::size(entries), type::unknown);
    return mt;
}

} // namespace output_mode

std::string to_string(output_mode::type t)
{
    for (const output_mode::map_type::entry_type& e : output_mode::entries)
        if (t == e.value)
            return std::string(e.key);

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
    os << "Path to either an output directory, or an output file.";
    return os.str();
}

std::string build_mode_help_text()
{
    std::ostringstream os;
    os << "Mode of operation. Select one of the following options: ";
    auto it = output_mode::entries, ite = output_mode::entries + std::size(output_mode::entries);
    --ite;

    for (; it != ite; ++it)
        os << std::string(it->key) << ", ";

    os << "or " << std::string(it->key) << ".";
    return os.str();
}

std::string build_map_help_text()
{
    std::ostringstream os;
    os << "Path to the map file. A map file is required for all modes except for the "
        << to_string(output_mode::type::structure) << " mode.";
    return os.str();
}

bool parse_and_dump_structure(const file_content& content, const std::string& output)
{
    xmlns_repository repo;
    xmlns_context cxt = repo.create_context();
    xml_structure_tree tree(cxt);
    tree.parse(content.str());

    if (output.empty())
    {
        tree.dump_compact(cout);
        return true;
    }

    ofstream file(output);
    if (!file)
    {
        cerr << "failed to create output file: " << output << endl;
        return false;
    }

    tree.dump_compact(file);

    return true;
}

void dump_document_structure(const file_content& content, output_stream& os)
{
    xmlns_repository repo;
    xmlns_context cxt = repo.create_context();
    dom::document_tree tree(cxt);
    tree.load(content.str());

    tree.dump_compact(os.get());
}

} // anonymous namespace

int main(int argc, char** argv) try
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
    output_mode::type mode = output_mode::get().find(s);

    if (mode == output_mode::type::unknown)
    {
        cerr << "Unknown output mode: " << s << endl;
        print_usage(cout, desc);
        return EXIT_FAILURE;
    }

    fs::path input_path = vm["input"].as<std::string>();

    if (!fs::is_regular_file(input_path))
    {
        cerr << input_path << " is not a valid file." << endl;
        return EXIT_FAILURE;
    }

    fs::path map_path;
    if (vm.count("map"))
    {
        map_path = vm["map"].as<std::string>();

        if (!fs::is_regular_file(map_path))
        {
            cerr << map_path << " is not a valid map file." << endl;
            return EXIT_FAILURE;
        }
    }

    std::string output;
    if (vm.count("output"))
        output = vm["output"].as<std::string>();

    file_content content(input_path.string().data());

    try
    {
        switch (mode)
        {
            case output_mode::type::structure:
            {
                bool success = parse_and_dump_structure(content, output);
                return success ? EXIT_SUCCESS : EXIT_FAILURE;
            }
            case output_mode::type::dump:
            {
                output_stream os(vm);
                dump_document_structure(content, os);
                return EXIT_SUCCESS;
            }
            case output_mode::type::map_gen:
            {
                output_stream os(vm);
                xmlns_repository repo;
                orcus_xml app(repo, nullptr, nullptr);
                app.write_map_definition(content.str(), os.get());
                return EXIT_SUCCESS;
            }
            default:
                ;
        }

        spreadsheet::range_size_t ss{1048576, 16384};
        spreadsheet::document doc{ss};
        spreadsheet::import_factory import_fact(doc);
        spreadsheet::export_factory export_fact(doc);

        xmlns_repository repo;
        orcus_xml app(repo, &import_fact, &export_fact);

        if (map_path.empty())
            app.detect_map_definition(content.str());
        else
        {
            file_content map_content(map_path.string().data());
            app.read_map_definition(map_content.str());
        }

        app.read_stream(content.str());

        switch (mode)
        {
            case output_mode::type::map:
            {
                dump_format_t format = dump_format_t::unknown;
                s.clear();

                if (vm.count("output-format"))
                {
                    s = vm["output-format"].as<std::string>();
                    format = to_dump_format_enum(s);
                }
                else
                {
                    cerr << "Output format is not specified." << endl;
                    print_usage(cout, desc);
                    return EXIT_FAILURE;
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
                app.write(content.str(), file);
                break;
            }
            default:
                ;
        }
    }
    catch (const malformed_xml_error& e)
    {
        cerr << create_parse_error_output(content.str(), e.offset()) << endl;
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
    cerr << e.what() << endl;
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
