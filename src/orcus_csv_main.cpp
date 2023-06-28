/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_csv.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/config.hpp"

#include "orcus_filter_global.hpp"

#include <cstdlib>
#include <iostream>

using namespace std;
using namespace orcus;
namespace po = boost::program_options;

class csv_args_handler : public extra_args_handler
{
    constexpr static const char* help_row_header =
        "Specify the number of header rows to repeat if the source content gets split into multiple sheets.";

    constexpr static const char* help_row_size =
        "Specify the number of maximum rows in each sheet.";

    constexpr static const char* help_split =
        "Specify whether or not to split the data into multiple sheets in case it won't fit in a single sheet.";

    spreadsheet::import_factory& m_fact;

public:
    csv_args_handler(spreadsheet::import_factory& fact) : m_fact(fact) {}
    virtual ~csv_args_handler() override {}

    virtual void add_options(po::options_description& desc) override
    {
        desc.add_options()
            ("row-header", po::value<size_t>(), help_row_header)
            ("split", help_split);
    }

    virtual void map_to_config(config& opt, const po::variables_map& vm) override
    {
        auto csv = std::get<config::csv_config>(opt.data);

        if (vm.count("row-header"))
            csv.header_row_size = vm["row-header"].as<size_t>();

        csv.split_to_multiple_sheets = vm.count("split") > 0;

        opt.data = csv;
    }
};

int main(int argc, char** argv)
{
    spreadsheet::range_size_t ss{1048576, 16384};
    spreadsheet::document doc{ss};
    spreadsheet::import_factory fact(doc);
    orcus_csv app(&fact);
    csv_args_handler hdl(fact);

    try
    {
        if (!parse_import_filter_args(argc, argv, fact, app, doc, &hdl))
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
