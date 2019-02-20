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
#include "orcus/spreadsheet/factory.hpp"

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

const std::map<dump_format_t, pstring> descriptions =
{
    std::make_pair(dump_format_t::csv,  "CSV format"),
    std::make_pair(dump_format_t::flat, "flat text format"),
    std::make_pair(dump_format_t::html, "HTML format"),
    std::make_pair(dump_format_t::json, "JSON format"),
    std::make_pair(dump_format_t::none, "no output"),
};

std::string gen_help_output_format()
{
    std::ostringstream os;
    os << "Specify the format of output file.  Supported format types are:";

    for (const std::pair<pstring, dump_format_t>& entry : get_dump_format_entries())
    {
        pstring desc;
        auto it_desc = descriptions.find(entry.second);
        if (it_desc != descriptions.end())
            desc = it_desc->second;

        os << std::endl << "  * " << entry.first << " - " << desc;
    }

    return os.str();
}

const char* help_program =
"The FILE must specify a path to an existing file.";

const char* help_output =
"Output directory path, or output file when --dump-check option is used.";

const char* help_dump_check =
"Dump the content to stdout in a special format used for content verification "
"in automated tests.";

const char* help_debug =
"Turn on a debug mode and optionally specify a debug level in order to generate run-time debug outputs.";

const char* help_row_size =
"Specify the number of maximum rows in each sheet.";

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
    int argc, char** argv, spreadsheet::import_factory& fact,
    iface::import_filter& app, iface::document_dumper& doc,
    extra_args_handler* args_handler)
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Print this help.")
        ("debug,d", po::value<uint16_t>()->default_value(0u)->implicit_value(1u), help_debug)
        ("dump-check", help_dump_check)
        ("output,o", po::value<string>(), help_output)
        ("output-format,f", po::value<string>(), gen_help_output_format().data())
        ("row-size", po::value<spreadsheet::row_t>(), help_row_size);

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
    dump_format_t outformat = dump_format_t::unknown;

    if (vm.count("input"))
        infile = vm["input"].as<string>();

    if (vm.count("output"))
        outdir = vm["output"].as<string>();

    if (vm.count("output-format"))
    {
        outformat_s = vm["output-format"].as<string>();
        outformat = to_dump_format_enum(outformat_s.data(), outformat_s.size());
    }

    if (vm.count("row-size"))
        fact.set_default_row_size(vm["row-size"].as<spreadsheet::row_t>());

    if (infile.empty())
    {
        cerr << err_no_input_file << endl;
        return false;
    }

    config opt = app.get_config();
    opt.debug = vm["debug"].as<uint16_t>();

    if (args_handler)
        args_handler->map_to_config(opt, vm);

    app.set_config(opt);

    if (vm.count("dump-check"))
    {
        // 'outdir' is used as the output file path in this mode.
        return handle_dump_check(app, doc, infile, outdir);
    }

    if (outformat == dump_format_t::unknown)
    {
        std::cerr << "You must specify one of the supported output formats." << endl;
        return false;
    }

    if (outformat == dump_format_t::none)
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
        case dump_format_t::flat:
            doc.dump_flat(outdir);
            break;
        case dump_format_t::html:
            doc.dump_html(outdir);
            break;
        case dump_format_t::json:
            doc.dump_json(outdir);
            break;
        case dump_format_t::csv:
            doc.dump_csv(outdir);
            break;
        default:
        {
            std::cerr << "Unknown output format type '" << outformat_s << "'. No output files have been generated." << std::endl;
        }

    }

    return true;
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
