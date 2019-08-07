/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_CLI_GLOBAL_HPP
#define INCLUDED_ORCUS_CLI_GLOBAL_HPP

#include <boost/program_options.hpp>
#include <iosfwd>

namespace orcus {

class output_stream
{
    std::unique_ptr<std::ofstream> m_ofs;
    std::ostream* m_os;

public:
    output_stream(const boost::program_options::variables_map& vm);

    std::ostream& get();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
