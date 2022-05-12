/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_filter_global.hpp"
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

const std::map<dump_format_t, std::string_view> descriptions =
{
    std::make_pair(dump_format_t::check, "Flat format that fully encodes document content. Suitable for automated testing."),
    std::make_pair(dump_format_t::csv,   "CSV format."),
    std::make_pair(dump_format_t::flat,  "Flat text format that displays document content in grid."),
    std::make_pair(dump_format_t::html,  "HTML format."),
    std::make_pair(dump_format_t::json,  "JSON format."),
    std::make_pair(dump_format_t::xml,   "This format is currently unsupported."),
    std::make_pair(dump_format_t::yaml,  "This format is currently unsupported."),
    std::make_pair(dump_format_t::debug_state, "This format dumps the internal state of the document in detail, useful for debugging."),
    std::make_pair(dump_format_t::none,  "No output to be generated. Maybe useful during development."),
};

const char* help_program =
"The FILE must specify a path to an existing file.";

const char* help_output =
"Output directory path, or output file when --dump-check option is used.";

const char* help_dump_check =
"Dump the content to stdout in a special format used for content verification "
"in automated tests.";

const char* help_debug =
"Turn on a debug mode and optionally specify a debug level in order to generate run-time debug outputs.";

const char* help_recalc =
"Re-calculate all formula cells after the documetn is loaded.";

const char* help_formula_error_policy =
"Specify whether to abort immediately when the loader fails to parse the first "
"formula cell ('fail'), or skip the offending cells and continue ('skip').";

const char* help_row_size =
"Specify the number of maximum rows in each sheet.";

const char* err_no_input_file = "No input file.";

}

std::string gen_help_output_format()
{
    std::ostringstream os;
    os << "Specify the output format.  Supported format types are:" << endl;

    for (std::pair<std::string_view, dump_format_t> entry : get_dump_format_entries())
    {
        std::string_view desc;
        auto it_desc = descriptions.find(entry.second);
        if (it_desc != descriptions.end())
            desc = it_desc->second;

        os << std::endl << "* " << entry.first << " - " << desc;
    }

    return os.str();
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
    bool debug = false;
    bool recalc_formula_cells = false;

    po::options_description desc("Options");
    desc.add_options()
        ("help,h", "Print this help.")
        ("debug,d", po::bool_switch(&debug), help_debug)
        ("recalc,r", po::bool_switch(&recalc_formula_cells), help_recalc)
        ("error-policy,e", po::value<string>()->default_value("fail"), help_formula_error_policy)
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

    std::string infile, outdir;
    dump_format_t outformat = dump_format_t::unknown;

    if (vm.count("input"))
        infile = vm["input"].as<string>();

    if (vm.count("output"))
        outdir = vm["output"].as<string>();

    if (vm.count("output-format"))
    {
        std::string outformat_s = vm["output-format"].as<string>();
        outformat = to_dump_format_enum(outformat_s);
    }

    if (vm.count("row-size"))
        fact.set_default_row_size(vm["row-size"].as<spreadsheet::row_t>());

    std::string error_policy_s = vm["error-policy"].as<std::string>();
    spreadsheet::formula_error_policy_t error_policy =
        spreadsheet::to_formula_error_policy(error_policy_s);

    if (error_policy == spreadsheet::formula_error_policy_t::unknown)
    {
        cerr << "Unrecognized error policy: " << error_policy_s << endl;
        return false;
    }

    fact.set_formula_error_policy(error_policy);

    if (infile.empty())
    {
        cerr << err_no_input_file << endl;
        return false;
    }

    config opt = app.get_config();
    opt.debug = debug;

    if (args_handler)
        args_handler->map_to_config(opt, vm);

    app.set_config(opt);

    fact.set_recalc_formula_cells(recalc_formula_cells);

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

    try
    {
        app.read_file(infile);
        doc.dump(outformat, outdir);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return false;
    }

    return true;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
