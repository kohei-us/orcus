/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/exception.hpp"

#include <sstream>

using namespace std;

namespace orcus {

general_error::general_error(const string& msg) :
    m_msg(msg)
{
}

general_error::general_error(const std::string& cls, const std::string& msg)
{
    ostringstream os;
    os << cls << ": " << msg;
    m_msg = os.str();
}

general_error::~general_error() throw()
{
}

const char* general_error::what() const throw()
{
    return m_msg.c_str();
}

void general_error::append_msg(const std::string& s)
{
    m_msg += s;
}

invalid_arg_error::invalid_arg_error(const std::string& msg) :
    std::invalid_argument(msg) {}

invalid_arg_error::~invalid_arg_error() throw() {}

xml_structure_error::xml_structure_error(const string& msg) :
    general_error(msg) {}

xml_structure_error::~xml_structure_error() throw() {}

json_structure_error::json_structure_error(const string& msg) :
    general_error(msg) {}

json_structure_error::~json_structure_error() throw() {}

invalid_map_error::invalid_map_error(const string& msg) :
    general_error(msg) {}

invalid_map_error::~invalid_map_error() throw() {}

value_error::value_error(const string& msg) :
    general_error(msg) {}

value_error::~value_error() throw() {}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
