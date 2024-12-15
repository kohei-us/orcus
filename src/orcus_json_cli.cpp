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

#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <sstream>

#include <mdds/sorted_string_map.hpp>
#include <boost/program_options.hpp>

#include "filesystem_env.hpp"

using namespace orcus;
namespace po = boost::program_options;

namespace orcus { namespace detail {

cmd_params::cmd_params() = default;
cmd_params::cmd_params(cmd_params&& other) = default;
cmd_params::~cmd_params() = default;

}}

namespace {

namespace mode {

using map_type = mdds::sorted_string_map<detail::mode_t>;

// Keys must be sorted.
constexpr map_type::entry_type entries[] =
{
    { "convert",   detail::mode_t::convert   },
    { "lint",      detail::mode_t::lint      },
    { "map",       detail::mode_t::map       },
    { "map-gen",   detail::mode_t::map_gen   },
    { "structure", detail::mode_t::structure },
    { "subtree",   detail::mode_t::subtree   },
};

const map_type& get()
{
    static map_type mt(entries, std::size(entries), detail::mode_t::unknown);
    return mt;
}

} // namespace mode

const char* help_program =
"The FILE must specify the path to an existing file.";

const char* help_json_output =
"Output file path.";

const char* help_json_output_format =
"Specify the format of output file.  Supported format types are:\n"
"\n"
"  * XML (xml)\n"
"  * JSON (json)\n"
"  * YAML (yaml)\n"
"  * flat tree dump (check)\n"
"  * no output (none)";

const char* help_json_map =
"Path to a map file.  This parameter is only used in map mode, and it is "
"required in that mode."
;

const char* help_indent =
"Number of whitespace characters to use for one indent level.  This is applicable "
"when the command generates output in JSON format."
;

const char* help_json_path =
"JSONPath expression specifying the root of a subtree to extract.  It is only used "
"in subtree mode."
;

const char* err_no_input_file = "No input file.";

void print_json_usage(std::ostream& os, const po::options_description& desc)
{
    os << "Usage: orcus-json [options] FILE\n\n";
    os << help_program << "\n\n" << desc;
}

std::string build_mode_help_text()
{
    std::ostringstream os;
    os << "Mode of operation. Select one of the following options: ";
    auto it = mode::entries, ite = mode::entries + std::size(mode::entries);
    --ite;

    for (; it != ite; ++it)
        os << it->key << ", ";

    os << "or " << it->key << ".";
    return os.str();
}

/**
 * Reset params.config in case of failure.
 */
void parse_args_for_convert(
    detail::cmd_params& params, const po::options_description& desc, const po::variables_map& vm)
{
    if (vm.count("indent"))
        params.indent = vm["indent"].as<std::size_t>();

    if (vm.count("resolve-refs"))
        params.config->resolve_references = true;

    if (vm.count("output-format"))
    {
        std::string s = vm["output-format"].as<std::string>();
        params.output_format = to_dump_format_enum(s);

        if (params.output_format == dump_format_t::unknown)
        {
            std::cerr << "Unknown output format type '" << s << "'." << std::endl;
            params.config.reset();
            return;
        }
    }
    else
    {
        std::cerr << "Output format is not specified." << std::endl;
        print_json_usage(std::cerr, desc);
        params.config.reset();
        return;
    }
}

void parse_args_for_lint(
    detail::cmd_params& params, const po::options_description& /*desc*/, const po::variables_map& vm)
{
    if (vm.count("indent"))
        params.indent = vm["indent"].as<std::size_t>();
}

void parse_args_for_subtree(
    detail::cmd_params& params, const po::options_description& /*desc*/, const po::variables_map& vm)
{
    if (vm.count("indent"))
        params.indent = vm["indent"].as<std::size_t>();

    if (vm.count("path"))
        params.json_path = vm["path"].as<std::string>();
}

/**
 * Reset params.config in case of failure.
 */
void parse_args_for_map(
    detail::cmd_params& params, const po::options_description& desc, const po::variables_map& vm)
{
    if (vm.count("map"))
    {
        fs::path map_path = vm["map"].as<std::string>();
        if (!fs::is_regular_file(map_path))
        {
            std::cerr << map_path.string() << " is not a valid file." << std::endl;
            params.config.reset();
            return;
        }

        params.map_file.load(map_path.string().data());
    }
    else
    {
        // Auto-mapping mode
    }

    parse_args_for_convert(params, desc, vm);
}

/**
 * Parse the command-line options, populate the json_config object, and
 * return that to the caller.
 */
detail::cmd_params parse_json_args(int argc, char** argv)
{
    detail::cmd_params params;

    po::options_description desc("Options");
    desc.add_options()
        ("help,h", "Print this help.")
        ("mode", po::value<std::string>(), build_mode_help_text().data())
        ("resolve-refs", "Resolve JSON references to external files.")
        ("output,o", po::value<std::string>(), help_json_output)
        ("output-format,f", po::value<std::string>(), help_json_output_format)
        ("map,m", po::value<std::string>(), help_json_map)
        ("indent,i", po::value<std::size_t>(), help_indent)
        ("path,p", po::value<std::string>(), help_json_path)
    ;

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
        print_json_usage(std::cerr, desc);
        return params;
    }

    if (vm.count("help"))
    {
        print_json_usage(std::cout, desc);
        return params;
    }

    if (vm.count("mode"))
    {
        std::string s = vm["mode"].as<std::string>();
        params.mode = mode::get().find(s);
        if (params.mode == detail::mode_t::unknown)
        {
            std::cerr << "Unknown mode string '" << s << "'." << std::endl;
            return params;
        }
    }

    params.config = std::make_unique<json_config>();

    if (vm.count("input"))
        params.config->input_path = vm["input"].as<std::string>();

    if (params.config->input_path.empty())
    {
        // No input file is given.
        std::cerr << err_no_input_file << std::endl;
        print_json_usage(std::cerr, desc);
        params.config.reset();
        return params;
    }

    if (!fs::exists(params.config->input_path))
    {
        std::cerr << "Input file does not exist: " << params.config->input_path << std::endl;
        params.config.reset();
        return params;
    }

    if (vm.count("output"))
        params.output_path = vm["output"].as<std::string>();

    switch (params.mode)
    {
        case detail::mode_t::map_gen:
        case detail::mode_t::structure:
            // Structure and map-gen modes only need input and output parameters.
            params.os = std::make_unique<output_stream>(vm);
            break;
        case detail::mode_t::convert:
            params.os = std::make_unique<output_stream>(vm);
            parse_args_for_convert(params, desc, vm);
            break;
        case detail::mode_t::map:
            parse_args_for_map(params, desc, vm);
            break;
        case detail::mode_t::lint:
            params.os = std::make_unique<output_stream>(vm);
            parse_args_for_lint(params, desc, vm);
            break;
        case detail::mode_t::subtree:
            params.os = std::make_unique<output_stream>(vm);
            parse_args_for_subtree(params, desc, vm);
            break;
        default:
            assert(!"This should not happen since the mode check is done way earlier.");
    }

    return params;
}

std::unique_ptr<json::document_tree> load_doc(const orcus::file_content& content, const json_config& config)
{
    std::unique_ptr<json::document_tree> doc(std::make_unique<json::document_tree>());
    doc->load(content.str(), config);
    return doc;
}

void build_doc_and_dump(const orcus::file_content& content, detail::cmd_params& params)
{
    std::unique_ptr<json::document_tree> doc = load_doc(content, *params.config);
    std::ostream& os = params.os->get();

    switch (params.output_format)
    {
        case dump_format_t::xml:
        {
            os << doc->dump_xml();
            break;
        }
        case dump_format_t::json:
        {
            os << doc->dump(params.indent);
            break;
        }
        case dump_format_t::yaml:
        {
            os << doc->dump_yaml();
            break;
        }
        case dump_format_t::check:
        {
            std::string xml_strm = doc->dump_xml();
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

void parse_and_write_map_file(const orcus::file_content& content, detail::cmd_params& params)
{
    std::vector<json::table_range_t> ranges;

    json::structure_tree::range_handler_type rh = [&ranges](json::table_range_t&& range)
    {
        ranges.push_back(std::move(range));
    };

    json::structure_tree tree;
    tree.parse(content.str());

    tree.process_ranges(rh);

    json::document_tree map_doc = {
        {"sheets", json::array()},
        {"ranges", json::array()}
    };

    json::node root = map_doc.get_document_root();
    json::node sheets_node = root["sheets"];
    json::node ranges_node = root["ranges"];

    size_t range_count = 0;
    for (const json::table_range_t& range : ranges)
    {
        std::ostringstream os;
        os << "range-" << range_count++;
        std::string sheet = os.str();
        sheets_node.push_back(sheet);

        ranges_node.push_back({
            {"sheet", sheet},
            {"row", 0},
            {"column", 0},
            {"row-header", true},
            {"fields", json::array()},
            {"row-groups", json::array()},
        });

        json::node range_node = ranges_node.back();
        json::node fields_node = range_node["fields"];
        json::node row_groups_node = range_node["row-groups"];

        for (const std::string& path : range.paths)
        {
            fields_node.push_back(json::object());
            json::node path_node = fields_node.back();
            path_node["path"] = path;
        }

        for (const std::string& row_group : range.row_groups)
        {
            row_groups_node.push_back(json::object());
            json::node path_node = row_groups_node.back();
            path_node["path"] = row_group;
        }
    }

    std::ostream& os = params.os->get();
    os << map_doc.dump(4);
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
                tree.parse(content.str());
                tree.normalize_tree();
                tree.dump_compact(params.os->get());
                break;
            }
            case detail::mode_t::map:
            {
                map_to_sheets_and_dump(content, params);
                break;
            }
            case detail::mode_t::map_gen:
            {
                parse_and_write_map_file(content, params);
                break;
            }
            case detail::mode_t::convert:
            {
                build_doc_and_dump(content, params);
                break;
            }
            case detail::mode_t::lint:
            {
                auto doc = load_doc(content, *params.config);
                std::ostream& os = params.os->get();
                os << doc->dump(params.indent);
                break;
            }
            case detail::mode_t::subtree:
            {
                auto doc = load_doc(content, *params.config);
                std::ostream& os = params.os->get();
                auto sub = json::subtree(*doc, params.json_path);
                os << sub.dump(params.indent);
                break;
            }
            default:
                std::cerr << "Unkonwn mode has been given." << std::endl;
                return EXIT_FAILURE;
        }
    }
    catch (const parse_error& e)
    {
        std::cerr << create_parse_error_output(content.str(), e.offset()) << std::endl;
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
