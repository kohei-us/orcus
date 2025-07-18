/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_ORCUS_FILTER_GLOBAL_HPP
#define ORCUS_ORCUS_FILTER_GLOBAL_HPP

#include "cli_global.hpp"

#include <orcus/config.hpp>
#include <orcus/interface.hpp>
#include <orcus/spreadsheet/types.hpp>
#include <orcus/spreadsheet/factory.hpp>

#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>

namespace orcus {

struct config;

namespace spreadsheet {

class import_factory;

}

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

std::string gen_help_output_format();

template<typename ArgCharT>
struct arg_parser_traits;

template<>
struct arg_parser_traits<char>
{
    using path_str_type = std::string;
    using po_clparser_type = boost::program_options::command_line_parser;

    static auto path_value()
    {
        return boost::program_options::value<path_str_type>();
    }

    static std::string_view string_view(const path_str_type& v)
    {
        return v;
    }
};

template<>
struct arg_parser_traits<wchar_t>
{
    using path_str_type = std::wstring;
    using po_clparser_type = boost::program_options::wcommand_line_parser;

    static auto path_value()
    {
        return boost::program_options::wvalue<path_str_type>();
    }

    static std::u16string_view string_view(const path_str_type& v)
    {
        return {reinterpret_cast<const char16_t*>(v.data()), v.size()};
    }
};

template<typename ArgCharT>
class import_filter_arg_parser
{
    using traits = arg_parser_traits<ArgCharT>;

    static constexpr const char* help_program =
    "The FILE must specify a path to an existing file.";

    static constexpr const char* help_output =
    "Output directory path, or output file when --dump-check option is used.";

    static constexpr const char* help_dump_check =
    "Dump the content to stdout in a special format used for content verification "
    "in automated tests.";

    static constexpr const char* help_debug =
    "Turn on a debug mode and optionally specify a debug level in order to generate run-time debug outputs.";

    static constexpr const char* help_recalc =
    "Re-calculate all formula cells after the documetn is loaded.";

    static constexpr const char* help_formula_error_policy =
    "Specify whether to abort immediately when the loader fails to parse the first "
    "formula cell ('fail'), or skip the offending cells and continue ('skip').";

    static constexpr const char* help_row_size =
    "Specify the number of maximum rows in each sheet.";

    static constexpr const char* err_no_input_file = "No input file.";

    spreadsheet::import_factory& m_fact;
    iface::import_filter& m_app;
    iface::document_dumper& m_doc;

public:
    import_filter_arg_parser(
        spreadsheet::import_factory& fact,
        iface::import_filter& app,
        iface::document_dumper& doc
    ) : m_fact(fact), m_app(app), m_doc(doc) {}

    bool parse(int argc, ArgCharT** argv, extra_args_handler* args_handler = nullptr) const
    {
        namespace po = boost::program_options;

        bool debug = false;
        bool recalc_formula_cells = false;

        po::options_description desc("Options");
        desc.add_options()
            ("help,h", "Print this help.")
            ("debug,d", po::bool_switch(&debug), help_debug)
            ("recalc,r", po::bool_switch(&recalc_formula_cells), help_recalc)
            ("error-policy,e", po::value<std::string>()->default_value("fail"), help_formula_error_policy)
            ("dump-check", help_dump_check)
            ("output,o", traits::path_value(), help_output)
            ("output-format,f", po::value<std::string>(), gen_help_output_format().data())
            ("row-size", po::value<spreadsheet::row_t>(), help_row_size);

        if (args_handler)
            args_handler->add_options(desc);

        po::options_description hidden("Hidden options");
        hidden.add_options()
            ("input", traits::path_value(), "input file");

        po::options_description cmd_opt;
        cmd_opt.add(desc).add(hidden);

        po::positional_options_description po_desc;
        po_desc.add("input", 1);

        po::variables_map vm;
        try
        {
            typename traits::po_clparser_type clparser(argc, argv);
            po::store(clparser.options(cmd_opt).positional(po_desc).run(), vm);
            po::notify(vm);
        }
        catch (const std::exception& e)
        {
            // Unknown options.
            std::cout << e.what() << std::endl;
            std::cout << desc;
            return false;
        }

        if (vm.count("help"))
        {
            std::cout << "Usage: orcus-" << m_app.get_name() << " [options] FILE" << std::endl << std::endl;
            std::cout << help_program << std::endl << std::endl << desc;
            return true;
        }

        typename traits::path_str_type infile, outdir;
        dump_format_t outformat = dump_format_t::unknown;

        if (vm.count("input"))
            infile = vm["input"].as<typename traits::path_str_type>();

        if (vm.count("output"))
            outdir = vm["output"].as<typename traits::path_str_type>();

        if (vm.count("output-format"))
        {
            std::string outformat_s = vm["output-format"].as<std::string>();
            outformat = to_dump_format_enum(outformat_s);
        }

        if (vm.count("row-size"))
            m_fact.set_default_row_size(vm["row-size"].as<spreadsheet::row_t>());

        std::string error_policy_s = vm["error-policy"].as<std::string>();
        spreadsheet::formula_error_policy_t error_policy =
            spreadsheet::to_formula_error_policy(error_policy_s);

        if (error_policy == spreadsheet::formula_error_policy_t::unknown)
        {
            std::cerr << "Unrecognized error policy: " << error_policy_s << std::endl;
            return false;
        }

        m_fact.set_formula_error_policy(error_policy);

        if (infile.empty())
        {
            std::cerr << err_no_input_file << std::endl;
            return false;
        }

        config opt = m_app.get_config();
        opt.debug = debug;

        if (args_handler)
            args_handler->map_to_config(opt, vm);

        m_app.set_config(opt);
        m_fact.set_recalc_formula_cells(recalc_formula_cells);

        if (vm.count("dump-check"))
            outformat = dump_format_t::check;

        if (outformat == dump_format_t::unknown)
        {
            std::cerr << "You must specify one of the supported output formats." << std::endl;
            return false;
        }

        try
        {
            m_app.read_file(traits::string_view(infile));
            m_doc.dump(outformat, traits::string_view(outdir));
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            return false;
        }

        return true;
    }
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
