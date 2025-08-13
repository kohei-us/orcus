/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/yaml_document_tree.hpp"
#include "orcus/yaml_parser_base.hpp"
#include "orcus/config.hpp"
#include "orcus/stream.hpp"

#include <iostream>
#include <fstream>
#include <string>

#include <boost/program_options.hpp>

#include "filesystem_env.hpp"

using namespace orcus;

namespace po = boost::program_options;

const char* help_program = "The FILE must specify a path to an existing file.";
const char* err_no_input_file = "No input file.";
const char* help_yaml_output = "Output file path.";
const char* help_yaml_output_format =
"Specify the format of output file.  Supported format types are:\n"
"  1) yaml\n"
"  2) json";

void print_yaml_usage(std::ostream& os, const po::options_description& desc)
{
    os << "Usage: orcus-yaml [options] FILE" << std::endl << std::endl;
    os << help_program << std::endl << std::endl << desc;
}

std::unique_ptr<yaml_config> parse_yaml_args(int argc, char** argv)
{
    po::options_description desc("Options");
    desc.add_options()
        ("help,h", "Print this help.")
        ("output,o", po::value<std::string>(), help_yaml_output)
        ("output-format,f", po::value<std::string>(), help_yaml_output_format);

    po::options_description hidden("Hidden options");
    hidden.add_options()
        ("input", po::value<std::string>(), "input file");

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
    catch (const std::exception& e)
    {
        // Unknown options.
        std::cerr << e.what() << std::endl;
        print_yaml_usage(std::cerr, desc);
        return nullptr;
    }

    if (vm.count("help"))
    {
        print_yaml_usage(std::cout, desc);
        return nullptr;
    }

    std::unique_ptr<yaml_config> config = std::make_unique<yaml_config>();

    if (vm.count("input"))
        config->input_path = vm["input"].as<std::string>();

    if (vm.count("output"))
        config->output_path = vm["output"].as<std::string>();

    if (vm.count("output-format"))
    {
        std::string outformat = vm["output-format"].as<std::string>();
        if (outformat == "none")
            config->output_format = yaml_config::output_format_type::none;
        else if (outformat == "yaml")
            config->output_format = yaml_config::output_format_type::yaml;
        else if (outformat == "json")
            config->output_format = yaml_config::output_format_type::json;
        else
        {
            std::cerr << "Unknown output format type '" << outformat << "'." << std::endl;
            return nullptr;
        }
    }
    else
    {
        std::cerr << "Output format is not specified." << std::endl;
        print_yaml_usage(std::cerr, desc);
        return nullptr;
    }

    if (config->input_path.empty())
    {
        std::cerr << err_no_input_file << std::endl;
        print_yaml_usage(std::cerr, desc);
        return nullptr;
    }

    if (!fs::exists(config->input_path))
    {
        std::cerr << "Input file does not exist: " << config->input_path << std::endl;
        return nullptr;
    }

    if (config->output_format != yaml_config::output_format_type::none)
    {
        if (config->output_path.empty())
        {
            std::cerr << "Output file not given." << std::endl;
            return nullptr;
        }

        // Check to make sure the output path doesn't point to an existing
        // directory.
        if (fs::is_directory(config->output_path))
        {
            std::cerr << "Output file path points to an existing directory.  Aborting." << std::endl;
            return nullptr;
        }
    }

    return config;
}

std::unique_ptr<yaml::document_tree> load_doc(const char* p, size_t n)
{
    std::unique_ptr<yaml::document_tree> doc(std::make_unique<yaml::document_tree>());
    try
    {
        doc->load({p, n});
    }
    catch (const parse_error& e)
    {
        std::cerr << create_parse_error_output(std::string_view(p, n), e.offset()) << std::endl;
        throw;
    }
    return doc;
}

int main(int argc, char** argv)
{
    try
    {
        std::unique_ptr<yaml_config> config = parse_yaml_args(argc, argv);
        if (!config)
            return EXIT_FAILURE;

        file_content content(config->input_path.data());
        std::unique_ptr<yaml::document_tree> doc = load_doc(content.data(), content.size());

        switch (config->output_format)
        {
            case yaml_config::output_format_type::yaml:
            {
                std::ofstream fs(config->output_path.c_str());
                fs << doc->dump_yaml();
            }
            break;
            case yaml_config::output_format_type::json:
            {
                std::ofstream fs(config->output_path.c_str());
                fs << doc->dump_json();
            }
            break;
            default:
                ;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
