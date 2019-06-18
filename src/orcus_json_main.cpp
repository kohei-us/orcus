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

#include <mdds/sorted_string_map.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace orcus;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace {

namespace mode {

enum class type {
    unknown,
    convert,
    structure
};

typedef mdds::sorted_string_map<type> map_type;

// Keys must be sorted.
const std::vector<map_type::entry> entries =
{
    { ORCUS_ASCII("convert"),   type::convert   },
    { ORCUS_ASCII("structure"), type::structure },
};

const map_type& get()
{
    static map_type mt(entries.data(), entries.size(), type::unknown);
    return mt;
}

} // namespace mode

const char* help_program =
"The FILE must specify the path to an existing file.";

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

std::string build_mode_help_text()
{
    std::ostringstream os;
    os << "Mode of operation. Select one of the following options: ";
    auto it = mode::entries.cbegin(), ite = mode::entries.cend();
    --ite;

    for (; it != ite; ++it)
        os << std::string(it->key, it->keylen) << ", ";

    os << "or " << std::string(it->key, it->keylen) << ".";
    return os.str();
}

struct cmd_context
{
    std::unique_ptr<json_config> config;
    mode::type mode = mode::type::convert;
};

/**
 * Parse the command-line options, populate the json_config object, and
 * return that to the caller.
 */
cmd_context parse_json_args(int argc, char** argv)
{
    cmd_context cxt;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Print this help.")
        ("mode", po::value<std::string>(), build_mode_help_text().data())
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
        return cxt;
    }

    if (vm.count("help"))
    {
        print_json_usage(cout, desc);
        return cxt;
    }

    if (vm.count("mode"))
    {
        std::string s = vm["mode"].as<std::string>();
        cxt.mode = mode::get().find(s.data(), s.size());
        if (cxt.mode == mode::type::unknown)
        {
            cerr << "Unknown mode string '" << s << "'." << endl;
            return cxt;
        }
    }

    if (cxt.mode == mode::type::structure)
        return cxt;

    cxt.config = orcus::make_unique<json_config>();

    if (vm.count("input"))
        cxt.config->input_path = vm["input"].as<string>();

    if (vm.count("output"))
        cxt.config->output_path = vm["output"].as<string>();

    if (vm.count("resolve-refs"))
        cxt.config->resolve_references = true;

    if (vm.count("output-format"))
    {
        std::string outformat = vm["output-format"].as<string>();
        if (outformat == "none")
            cxt.config->output_format = json_config::output_format_type::none;
        else if (outformat == "xml")
            cxt.config->output_format = json_config::output_format_type::xml;
        else if (outformat == "json")
            cxt.config->output_format = orcus::json_config::output_format_type::json;
        else if (outformat == "check")
            cxt.config->output_format = orcus::json_config::output_format_type::check;
        else
        {
            cerr << "Unknown output format type '" << outformat << "'." << endl;
            cxt.config.reset();
            return cxt;
        }
    }
    else
    {
        cerr << "Output format is not specified." << endl;
        print_json_usage(cerr, desc);
        cxt.config.reset();
        return cxt;
    }

    if (cxt.config->input_path.empty())
    {
        cerr << err_no_input_file << endl;
        print_json_usage(cerr, desc);
        cxt.config.reset();
        return cxt;
    }

    if (!fs::exists(cxt.config->input_path))
    {
        cerr << "Input file does not exist: " << cxt.config->input_path << endl;
        cxt.config.reset();
        return cxt;
    }

    if (cxt.config->output_format != json_config::output_format_type::none)
    {
        // Check to make sure the output path doesn't point to an existing
        // directory.
        if (fs::is_directory(cxt.config->output_path))
        {
            cerr << "Output file path points to an existing directory.  Aborting." << endl;
            cxt.config.reset();
            return cxt;
        }
    }

    return cxt;
}

std::unique_ptr<json::document_tree> load_doc(const orcus::file_content& content, const json_config& config)
{
    std::unique_ptr<json::document_tree> doc(orcus::make_unique<json::document_tree>());
    try
    {
        doc->load(content.data(), content.size(), config);
    }
    catch (const json::parse_error& e)
    {
        cerr << create_parse_error_output(content.str(), e.offset()) << endl;
        throw;
    }
    return doc;
}

} // anonymous namespace

int main(int argc, char** argv)
{
    cmd_context cxt;

    try
    {
        cxt = parse_json_args(argc, argv);
    }
    catch (const std::exception& e)
    {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }


    if (cxt.mode == mode::type::structure)
    {
        cout << "TODO: implement this" << endl;
        return EXIT_SUCCESS;
    }

    if (!cxt.config || cxt.mode == mode::type::unknown)
        return EXIT_FAILURE;

    try
    {
        file_content content(cxt.config->input_path.data());
        std::unique_ptr<json::document_tree> doc = load_doc(content, *cxt.config);

        std::ostream* os = &cout;
        std::unique_ptr<std::ofstream> fs;

        if (!cxt.config->output_path.empty())
        {
            // Output to stdout when output path is not given.
            fs = std::make_unique<std::ofstream>(cxt.config->output_path.data());
            os = fs.get();
        }

        switch (cxt.config->output_format)
        {
            case json_config::output_format_type::xml:
            {
                *os << doc->dump_xml();
                break;
            }
            case json_config::output_format_type::json:
            {
                *os << doc->dump();
                break;
            }
            case json_config::output_format_type::check:
            {
                string xml_strm = doc->dump_xml();
                xmlns_repository repo;
                xmlns_context ns_cxt = repo.create_context();
                dom::document_tree dom(ns_cxt);
                dom.load(xml_strm);

                dom.dump_compact(*os);
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
