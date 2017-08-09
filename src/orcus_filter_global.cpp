/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_filter_global.hpp"
#include "orcus/pstring.hpp"
#include "orcus/config.hpp"
#include "orcus/interface.hpp"
#include "orcus/global.hpp"

#include <mdds/sorted_string_map.hpp>
#include <boost/filesystem.hpp>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;
using namespace orcus;

namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace orcus {

extra_args_handler::~extra_args_handler() {}

namespace {

namespace doc_output_format {

enum class type
{
    unspecified,
    none,
    flat,
    html,
    json,
    csv
};

typedef mdds::sorted_string_map<doc_output_format::type> map_type;

// Keys must be sorted.
const std::vector<map_type::entry> entries =
{
    { ORCUS_ASCII("csv"),  type::csv  },
    { ORCUS_ASCII("flat"), type::flat },
    { ORCUS_ASCII("html"), type::html },
    { ORCUS_ASCII("json"), type::json },
    { ORCUS_ASCII("none"), type::none },
};

// The order must match that of the entries above.
const std::vector<const char*> descriptions =
{
    "CSV format",
    "flat text format",
    "HTML format",
    "JSON format",
    "no output",
};

const map_type& get()
{
    static map_type mt(entries.data(), entries.size(), type::unspecified);
    return mt;
}

std::string gen_help_text()
{
    std::ostringstream os;
    os << "Specify the format of output file.  Supported format types are:";

    for (size_t i = 0, n = entries.size(); i < n; ++i)
        os << std::endl << "  * " << std::string(entries[i].key, entries[i].keylen)
            << " - " << descriptions[i];

    return os.str();
}

}

const char* help_program =
"The FILE must specify a path to an existing file.";

const char* help_output =
"Output directory path, or output file when --dump-check option is used.";

const char* help_dump_check =
"Dump the content to stdout in a special format used for content verification "
"in automated tests.";

const char* help_debug =
"Turn on a debug mode to generate run-time debug output.";

const char* help_json_output =
"Output file path.";

const char* help_json_output_format =
"Specify the format of output file.  Supported format types are:\n"
"  * XML (xml)\n"
"  * JSON (json)\n"
"  * flat tree dump (check)\n"
"  * no output (none)";

const char* err_no_input_file = "No input file.";

}

bool handle_dump_check(
    iface::import_filter& app, iface::document_dumper& doc, const string& infile, const string& outfile)
{
    if (outfile.empty())
    {
        // Dump to stdout when no output file is specified.
        app.read_file(infile);
        doc.dump_check(cout);
        return true;
    }

    if (fs::exists(outfile) && fs::is_directory(outfile))
    {
        cerr << "A directory named '" << outfile << "' already exists." << endl;
        return false;
    }

    ofstream file(outfile.c_str());
    app.read_file(infile);
    doc.dump_check(file);
    return true;
}

bool parse_import_filter_args(
    int argc, char** argv, iface::import_filter& app, iface::document_dumper& doc,
    extra_args_handler* args_handler)
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Print this help.")
        ("debug,d", help_debug)
        ("dump-check", help_dump_check)
        ("output,o", po::value<string>(), help_output)
        ("output-format,f", po::value<string>(), doc_output_format::gen_help_text().data());

    if (args_handler)
        args_handler->add_options(desc);

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
        cout << e.what() << endl;
        cout << desc;
        return false;
    }

    if (vm.count("help"))
    {
        cout << "Usage: orcus-" << app.get_name() << " [options] FILE" << endl << endl;
        cout << help_program << endl << endl << desc;
        return true;
    }

    std::string infile, outdir, outformat_s;
    doc_output_format::type outformat = doc_output_format::type::unspecified;

    if (vm.count("input"))
        infile = vm["input"].as<string>();

    if (vm.count("output"))
        outdir = vm["output"].as<string>();

    if (vm.count("output-format"))
    {
        outformat_s = vm["output-format"].as<string>();
        outformat = doc_output_format::get().find(outformat_s.data(), outformat_s.size());
    }

    if (infile.empty())
    {
        cerr << err_no_input_file << endl;
        return false;
    }

    config opt = app.get_config();
    opt.debug = vm.count("debug") > 0;

    if (args_handler)
        args_handler->map_to_config(opt, vm);

    app.set_config(opt);

    if (vm.count("dump-check"))
    {
        // 'outdir' is used as the output file path in this mode.
        return handle_dump_check(app, doc, infile, outdir);
    }

    if (outformat == doc_output_format::type::unspecified)
    {
        std::cerr << "You must specify one of the supported output formats." << endl;
        return false;
    }

    if (outformat == doc_output_format::type::none)
    {
        // When "none" format is specified, just read the input file and exit.
        app.read_file(infile);
        return true;
    }

    if (outdir.empty())
    {
        cerr << "No output directory." << endl;
        return false;
    }

    if (fs::exists(outdir))
    {
        if (!fs::is_directory(outdir))
        {
            cerr << "A file named '" << outdir << "' already exists, and is not a directory." << endl;
            return false;
        }
    }
    else
        fs::create_directory(outdir);

    app.read_file(infile);

    switch (outformat)
    {
        case doc_output_format::type::flat:
            doc.dump_flat(outdir);
            break;
        case doc_output_format::type::html:
            doc.dump_html(outdir);
            break;
        case doc_output_format::type::json:
            doc.dump_json(outdir);
            break;
        case doc_output_format::type::csv:
            doc.dump_csv(outdir);
            break;
        default:
        {
            std::cerr << "Unknown output format type '" << outformat_s << "'. No output files have been generated." << std::endl;
        }

    }

    return true;
}

void print_json_usage(std::ostream& os, const po::options_description& desc)
{
    os << "Usage: orcus-json [options] FILE" << endl << endl;
    os << help_program << endl << endl << desc;
}

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

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
