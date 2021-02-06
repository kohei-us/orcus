/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "cli_global.hpp"

#include "orcus/global.hpp"

#include <iostream>
#include <fstream>

#include <boost/filesystem.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace orcus {

output_stream::output_stream(const boost::program_options::variables_map& vm) :
    m_os(&std::cout)
{
    if (!vm.count("output"))
        // No output parameter given. Output to stdout.
        return;

    std::string output_path = vm["output"].as<std::string>();

    if (output_path.empty())
        // Specified output path is empty.
        return;

    // Check to make sure the output path doesn't point to an existing
    // directory.
    if (fs::is_directory(output_path))
    {
        std::ostringstream os;
        os << "Output file path points to an existing directory.";
        throw std::invalid_argument(os.str());
    }

    // Output to stdout when output path is not given.
    m_ofs = std::make_unique<std::ofstream>(output_path.data());
    m_os = m_ofs.get();
}

output_stream::output_stream(output_stream&& other) :
    m_ofs(std::move(other.m_ofs)),
    m_os(other.m_os)
{
    if (m_ofs)
        m_os = m_ofs.get();

    other.m_os = nullptr;
}

std::ostream& output_stream::get()
{
    return *m_os;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
