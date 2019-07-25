/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_json_cli.hpp"
#include "orcus/config.hpp"

#ifdef __ORCUS_SPREADSHEET_MODEL
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/orcus_json.hpp"
#endif

#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>
#include <sstream>

using namespace std;

namespace orcus { namespace detail {

#ifdef __ORCUS_SPREADSHEET_MODEL

void map_to_sheets_and_dump(const file_content& content, cmd_params& params)
{
    spreadsheet::document doc;
    spreadsheet::import_factory factory(doc);
    orcus_json app(&factory);

    if (params.map_file.empty())
        // Automatic mapping of JSON to table.
        app.detect_map_definition(content.data(), content.size());
    else
        app.read_map_definition(params.map_file.data(), params.map_file.size());

    app.read_stream(content.data(), content.size());
    doc.dump(params.config->output_format, params.config->output_path);
}

#else

void map_to_sheets_and_dump(const file_content& content, cmd_params& params)
{
    throw std::runtime_error(
        "map mode disabled as the spreadsheet model backend is not available.");
}

#endif

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
