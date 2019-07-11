/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_json_cli.hpp"
#include "orcus/json_document_tree.hpp"
#include "orcus/json_parser_base.hpp"
#include "orcus/json_structure_tree.hpp"
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

namespace orcus { namespace detail {

cmd_params::cmd_params() {}

cmd_params::cmd_params(cmd_params&& other) :
    config(std::move(other.config)),
    mode(other.mode),
    map_file(std::move(other.map_file))
{
}

cmd_params::~cmd_params() {}

std::ostream& cmd_params::get_output_stream()
{
    std::ostream* os = &cout;

    if (!config->output_path.empty())
    {
        // Output to stdout when output path is not given.
        fs = std::make_unique<std::ofstream>(config->output_path.data());
        os = fs.get();
    }

    return *os;
}

}}

namespace {

namespace mode {

typedef mdds::sorted_string_map<detail::mode_t> map_type;

// Keys must be sorted.
const std::vector<map_type::entry> entries =
{
    { ORCUS_ASCII("convert"),   detail::mode_t::convert   },
    { ORCUS_ASCII("map"),       detail::mode_t::map       },
    { ORCUS_ASCII("structure"), detail::mode_t::structure },
};

const map_type& get()
{
    static map_type mt(entries.data(), entries.size(), detail::mode_t::unknown);
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

const char* help_json_map =
"Path to a map file.  This parameter is only used for map mode, and it is "
"required for map mode."
;

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

/**
 * Reset params.config in case of failure.
 */
void parse_args_for_convert(
    detail::cmd_params& params, const po::options_description& desc, const po::variables_map& vm)
{
    if (vm.count("resolve-refs"))
        params.config->resolve_references = true;

    if (vm.count("output-format"))
    {
        std::string s = vm["output-format"].as<string>();
        params.config->output_format = to_dump_format_enum(s.data(), s.size());

        if (params.config->output_format == dump_format_t::unknown)
        {
            cerr << "Unknown output format type '" << s << "'." << endl;
            params.config.reset();
            return;
        }
    }
    else
    {
        cerr << "Output format is not specified." << endl;
        print_json_usage(cerr, desc);
        params.config.reset();
        return;
    }

    if (params.config->output_format != dump_format_t::none)
    {
        // Check to make sure the output path doesn't point to an existing
        // directory.
        if (fs::is_directory(params.config->output_path))
        {
            cerr << "Output file path points to an existing directory.  Aborting." << endl;
            params.config.reset();
            return;
        }
    }
}

/**
 * Reset params.config in case of failure.
 */
void parse_args_for_map(
    detail::cmd_params& params, const po::options_description& desc, const po::variables_map& vm)
{
    if (!vm.count("map"))
    {
        cerr << "Path to a map file is required, but is not given.";
        params.config.reset();
        return;
    }

    fs::path map_path = vm["map"].as<std::string>();
    if (!fs::is_regular_file(map_path))
    {
        cerr << map_path.string() << " is not a valid file." << endl;
        params.config.reset();
        return;
    }

    params.map_file.load(map_path.string().data());

    parse_args_for_convert(params, desc, vm);
}

/**
 * Parse the command-line options, populate the json_config object, and
 * return that to the caller.
 */
detail::cmd_params parse_json_args(int argc, char** argv)
{
    detail::cmd_params params;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Print this help.")
        ("mode", po::value<std::string>(), build_mode_help_text().data())
        ("resolve-refs", "Resolve JSON references to external files.")
        ("output,o", po::value<string>(), help_json_output)
        ("output-format,f", po::value<string>(), help_json_output_format)
        ("map,m", po::value<string>(), help_json_map)
    ;

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
        return params;
    }

    if (vm.count("help"))
    {
        print_json_usage(cout, desc);
        return params;
    }

    if (vm.count("mode"))
    {
        std::string s = vm["mode"].as<std::string>();
        params.mode = mode::get().find(s.data(), s.size());
        if (params.mode == detail::mode_t::unknown)
        {
            cerr << "Unknown mode string '" << s << "'." << endl;
            return params;
        }
    }

    params.config = orcus::make_unique<json_config>();

    if (vm.count("input"))
        params.config->input_path = vm["input"].as<string>();

    if (params.config->input_path.empty())
    {
        // No input file is given.
        cerr << err_no_input_file << endl;
        print_json_usage(cerr, desc);
        params.config.reset();
        return params;
    }

    if (!fs::exists(params.config->input_path))
    {
        cerr << "Input file does not exist: " << params.config->input_path << endl;
        params.config.reset();
        return params;
    }

    if (vm.count("output"))
        params.config->output_path = vm["output"].as<string>();

    switch (params.mode)
    {
        case detail::mode_t::structure:
            // Structure mode only needs input and output parameters.
            break;
        case detail::mode_t::convert:
            parse_args_for_convert(params, desc, vm);
            break;
        case detail::mode_t::map:
            parse_args_for_map(params, desc, vm);
            break;
        default:
            assert(!"This should not happen since the mode check is done way earlier.");
    }

    return params;
}

std::unique_ptr<json::document_tree> load_doc(const orcus::file_content& content, const json_config& config)
{
    std::unique_ptr<json::document_tree> doc(orcus::make_unique<json::document_tree>());
    doc->load(content.data(), content.size(), config);
    return doc;
}

void build_doc_and_dump(const orcus::file_content& content, detail::cmd_params& params)
{
    std::unique_ptr<json::document_tree> doc = load_doc(content, *params.config);
    std::ostream& os = params.get_output_stream();

    switch (params.config->output_format)
    {
        case dump_format_t::xml:
        {
            os << doc->dump_xml();
            break;
        }
        case dump_format_t::json:
        {
            os << doc->dump();
            break;
        }
        case dump_format_t::check:
        {
            string xml_strm = doc->dump_xml();
            xmlns_repository repo;
            xmlns_context ns_cxt = repo.create_context();
            dom::document_tree dom(ns_cxt);
            dom.load(xml_strm);

            dom.dump_compact(os);
            break;
        }
        default:
            ;
    }
}

} // anonymous namespace

int main(int argc, char** argv)
{
    file_content content;

    try
    {
        detail::cmd_params params = parse_json_args(argc, argv);

        if (!params.config || params.mode == detail::mode_t::unknown)
            return EXIT_FAILURE;

        assert(!params.config->input_path.empty());
        content.load(params.config->input_path.data());

        switch (params.mode)
        {
            case detail::mode_t::structure:
            {
                json::structure_tree tree;
                tree.parse(content.data(), content.size());
                tree.dump_compact(params.get_output_stream());
                break;
            }
            case detail::mode_t::map:
            {
                map_to_sheets_and_dump(content, params);
                break;
            }
            case detail::mode_t::convert:
            {
                build_doc_and_dump(content, params);
                break;
            }
            default:
                cerr << "Unkonwn mode has been given." << endl;
                return EXIT_FAILURE;
        }
    }
    catch (const json::parse_error& e)
    {
        cerr << create_parse_error_output(content.str(), e.offset()) << endl;
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception& e)
    {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
