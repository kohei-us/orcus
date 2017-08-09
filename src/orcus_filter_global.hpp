/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_ORCUS_FILTER_GLOBAL_HPP
#define ORCUS_ORCUS_FILTER_GLOBAL_HPP

#include <memory>
#include <boost/program_options.hpp>

namespace orcus {

struct config;
struct json_config;

namespace iface {

class import_filter;
class document_dumper;

}

/**
 * Interface for supporting additional command-line options.
 */
class extra_args_handler
{
public:
    virtual ~extra_args_handler();

    virtual void add_options(boost::program_options::options_description& desc) = 0;
    virtual void map_to_config(
        config& opt, const boost::program_options::variables_map& vm) = 0;
};

bool parse_import_filter_args(
    int argc, char** argv,
    iface::import_filter& app, iface::document_dumper& doc,
    extra_args_handler* args_handler = nullptr);

/**
 * Parse the command-line options, populate the json_config object, and
 * return that to the caller.
 */
std::unique_ptr<json_config> parse_json_args(int argc, char** argv);

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
