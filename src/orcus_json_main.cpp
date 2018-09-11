/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/json_document_tree.hpp"
#include "orcus/json_parser_base.hpp"
#include "orcus/config.hpp"
#include "orcus/stream.hpp"
#include "orcus/xml_namespace.hpp"
#include "orcus/dom_tree.hpp"
#include "orcus/global.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <memory>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace orcus;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

const char* help_program =
"The FILE must specify a path to an existing file.";

const char* help_json_output =
"Output file path.";

const char* help_json_output_format =
"Specify the format of output file.  Supported format types are:\n"
"  * XML (xml)\n"
"  * JSON (json)\n"
"  * flat tree dump (check)\n"
"  * no output (none)";

const char* err_no_input_file = "No input file.";

void print_json_usage(std::ostream& os, const po::options_description& desc)
{
    os << "Usage: orcus-json [options] FILE" << endl << endl;
    os << help_program << endl << endl << desc;
}

/**
 * Parse the command-line options, populate the json_config object, and
 * return that to the caller.
 */
std::unique_ptr<json_config> parse_json_args(int argc, char** argv)
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Print this help.")
        ("resolve-refs", "Resolve JSON references to external files.")
        ("output,o", po::value<string>(), help_json_output)
        ("output-format,f", po::value<string>(), help_json_output_format);

    po::options_description hidden("Hidden options");
    hidden.add_options()
        ("input", po::value<string>(), "input file");

    po::options_description cmd_opt;
    cmd_opt.add(desc).add(hidden);

    po::positional_options_description po_desc;
    po_desc.add("input", -1);

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
        print_json_usage(cerr, desc);
        return nullptr;
    }

    if (vm.count("help"))
    {
        print_json_usage(cout, desc);
        return nullptr;
    }

    std::unique_ptr<json_config> config = orcus::make_unique<json_config>();

    if (vm.count("input"))
        config->input_path = vm["input"].as<string>();

    if (vm.count("output"))
        config->output_path = vm["output"].as<string>();

    if (vm.count("resolve-refs"))
        config->resolve_references = true;

    if (vm.count("output-format"))
    {
        std::string outformat = vm["output-format"].as<string>();
        if (outformat == "none")
            config->output_format = json_config::output_format_type::none;
        else if (outformat == "xml")
            config->output_format = json_config::output_format_type::xml;
        else if (outformat == "json")
            config->output_format = orcus::json_config::output_format_type::json;
        else if (outformat == "check")
            config->output_format = orcus::json_config::output_format_type::check;
        else
        {
            cerr << "Unknown output format type '" << outformat << "'." << endl;
            return nullptr;
        }
    }
    else
    {
        cerr << "Output format is not specified." << endl;
        print_json_usage(cerr, desc);
        return nullptr;
    }

    if (config->input_path.empty())
    {
        cerr << err_no_input_file << endl;
        print_json_usage(cerr, desc);
        return nullptr;
    }

    if (!fs::exists(config->input_path))
    {
        cerr << "Input file does not exist: " << config->input_path << endl;
        return nullptr;
    }

    if (config->output_format != json_config::output_format_type::none)
    {
        if (config->output_path.empty())
        {
            cerr << "Output file not given." << endl;
            return nullptr;
        }

        // Check to make sure the output path doesn't point to an existing
        // directory.
        if (fs::is_directory(config->output_path))
        {
            cerr << "Output file path points to an existing directory.  Aborting." << endl;
            return nullptr;
        }
    }

    return config;
}

std::unique_ptr<json::document_tree> load_doc(const std::string& strm, const json_config& config)
{
    std::unique_ptr<json::document_tree> doc(orcus::make_unique<json::document_tree>());
    try
    {
        doc->load(strm, config);
    }
    catch (const json::parse_error& e)
    {
        cerr << create_parse_error_output(strm, e.offset()) << endl;
        throw;
    }
    return doc;
}

int main(int argc, char** argv)
{
    std::unique_ptr<json_config> config;

    try
    {
        config = parse_json_args(argc, argv);
    }
    catch (const std::exception& e)
    {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }

    if (!config)
        return EXIT_FAILURE;

    try
    {
        std::string strm = load_file_content(config->input_path.c_str());
        std::unique_ptr<json::document_tree> doc = load_doc(strm, *config);

        switch (config->output_format)
        {
            case json_config::output_format_type::xml:
            {
                ofstream fs(config->output_path.c_str());
                fs << doc->dump_xml();
            }
            break;
            case json_config::output_format_type::json:
            {
                ofstream fs(config->output_path.c_str());
                fs << doc->dump();
            }
            break;
            case json_config::output_format_type::check:
            {
                string xml_strm = doc->dump_xml();
                xmlns_repository repo;
                xmlns_context cxt = repo.create_context();
                dom_tree dom(cxt);
                dom.load(xml_strm);

                ofstream fs(config->output_path.c_str());
                dom.dump_compact(fs);
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
