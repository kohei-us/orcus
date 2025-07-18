/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_filter_global.hpp"

#include <mdds/sorted_string_map.hpp>
#include <vector>
#include <iostream>
#include <fstream>

#include "filesystem_env.hpp"

using namespace orcus;

namespace po = boost::program_options;

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

}

std::string gen_help_output_format()
{
    std::ostringstream os;
    os << "Specify the output format.  Supported format types are:" << std::endl;

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
    iface::import_filter& app, iface::document_dumper& doc, const std::string& infile, const std::string& outfile)
{
    if (outfile.empty())
    {
        // Dump to stdout when no output file is specified.
        app.read_file(infile);
        doc.dump_check(std::cout);
        return true;
    }

    if (fs::exists(outfile) && fs::is_directory(outfile))
    {
        std::cerr << "A directory named '" << outfile << "' already exists." << std::endl;
        return false;
    }

    std::ofstream file(outfile.c_str());
    app.read_file(infile);
    doc.dump_check(file);
    return true;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
