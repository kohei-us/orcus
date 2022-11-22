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

general_error::~general_error() noexcept
{
}

const char* general_error::what() const noexcept
{
    return m_msg.c_str();
}

void general_error::append_msg(const std::string& s)
{
    m_msg += s;
}

invalid_arg_error::invalid_arg_error(const std::string& msg) :
    std::invalid_argument(msg) {}

invalid_arg_error::~invalid_arg_error() noexcept {}

xml_structure_error::xml_structure_error(const string& msg) :
    general_error(msg) {}

xml_structure_error::~xml_structure_error() noexcept {}

json_structure_error::json_structure_error(const string& msg) :
    general_error(msg) {}

json_structure_error::~json_structure_error() noexcept {}

invalid_map_error::invalid_map_error(const string& msg) :
    general_error(msg) {}

invalid_map_error::~invalid_map_error() noexcept {}

value_error::value_error(const string& msg) :
    general_error(msg) {}

value_error::~value_error() noexcept {}

xpath_error::xpath_error(const string& msg) : general_error(msg) {}

xpath_error::~xpath_error() noexcept {}

interface_error::interface_error(const std::string& msg) : general_error(msg) {}

interface_error::~interface_error() noexcept {}

namespace {

std::string build_offset_msg(std::ptrdiff_t offset)
{
    std::ostringstream os;
    os << " (offset=" << offset << ')';
    return os.str();
}

std::string build_message(std::string_view msg_before, char c, std::string_view msg_after)
{
    std::ostringstream os;
    os << msg_before << c << msg_after;
    return os.str();
}

std::string build_message(
    std::string_view msg_before, std::string_view msg, std::string_view msg_after)
{
    std::ostringstream os;
    os << msg_before << msg << msg_after;
    return os.str();
}

}

parse_error::parse_error(const std::string& msg, std::ptrdiff_t offset) :
    general_error(msg), m_offset(offset)
{
    append_msg(build_offset_msg(offset));
}

parse_error::parse_error(const std::string& cls, const std::string& msg, std::ptrdiff_t offset) :
    general_error(cls, msg), m_offset(offset)
{
    append_msg(build_offset_msg(offset));
}

std::ptrdiff_t parse_error::offset() const
{
    return m_offset;
}

void parse_error::throw_with(
    std::string_view msg_before, char c, std::string_view msg_after, std::ptrdiff_t offset)
{
    throw parse_error(build_message(msg_before, c, msg_after), offset);
}

void parse_error::throw_with(
    std::string_view msg_before, std::string_view msg, std::string_view msg_after, std::ptrdiff_t offset)
{
    throw parse_error(build_message(msg_before, msg, msg_after), offset);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
