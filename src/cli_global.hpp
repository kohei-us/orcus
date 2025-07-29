/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_CLI_GLOBAL_HPP
#define INCLUDED_ORCUS_CLI_GLOBAL_HPP

#include <orcus/stream.hpp>

#include <boost/program_options.hpp>
#include <iosfwd>

#ifdef _WIN32
#define ORCUS_CLI_MAIN wmain
using arg_char_t = wchar_t;
#else
#define ORCUS_CLI_MAIN main
using arg_char_t = char;
#endif

namespace orcus {

void bootstrap_program();

file_content to_file_content(const char* filepath);
file_content to_file_content(const wchar_t* filepath);

/**
 * This class abstracts away an instance of std::ostream.  It's either
 * std::cout (if no output file path is specified) or std::ofstream if an
 * output file path is specified via CLI.
 */
class output_stream
{
    std::unique_ptr<std::ofstream> m_ofs;
    std::ostream* m_os;

public:
    output_stream(const boost::program_options::variables_map& vm);
    output_stream(output_stream&& other);
    output_stream(const output_stream&) = delete;
    output_stream& operator=(const output_stream&) = delete;

    std::ostream& get();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
