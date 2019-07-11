/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_ORCUS_JSON_CLI_HPP
#define INCLUDED_ORCUS_ORCUS_JSON_CLI_HPP

#include "orcus/stream.hpp"

#include <ostream>

namespace orcus {

struct json_config;

namespace detail {

enum class mode_t
{
    unknown,
    convert,
    map,
    structure
};

struct cmd_params
{
    std::unique_ptr<json_config> config; //< json parser configuration.
    std::unique_ptr<std::ofstream> fs; //< output stream instance that we own.
    mode_t mode = mode_t::convert;
    file_content map_file;

    cmd_params(const cmd_params&) = delete;
    cmd_params& operator= (const cmd_params&) = delete;

    cmd_params();
    cmd_params(cmd_params&& other);
    ~cmd_params();

    std::ostream& get_output_stream();
};

void map_to_sheets_and_dump(const file_content& content, cmd_params& params);

}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
