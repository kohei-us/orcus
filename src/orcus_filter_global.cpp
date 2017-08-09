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
        ("debug,d", help_debug)
        ("dump-check", help_dump_check)
        ("output,o", po::value<string>(), help_output)
        ("output-format,f", po::value<string>(), doc_output_format::gen_help_text().data())
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

    if (vm.count("row-size"))
        fact.set_default_row_size(vm["row-size"].as<spreadsheet::row_t>());

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

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
