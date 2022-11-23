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

general_error::general_error(std::string msg) :
    m_msg(std::move(msg))
{
}

general_error::general_error(std::string_view cls, std::string_view msg)
{
    std::ostringstream os;
    os << cls << ": " << msg;
    m_msg = os.str();
}

general_error::~general_error() noexcept = default;

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

xml_structure_error::xml_structure_error(std::string msg) :
    general_error(std::move(msg)) {}

xml_structure_error::~xml_structure_error() noexcept = default;

json_structure_error::json_structure_error(std::string msg) :
    general_error(std::move(msg)) {}

json_structure_error::~json_structure_error() noexcept = default;

invalid_map_error::invalid_map_error(std::string msg) :
    general_error(std::move(msg)) {}

invalid_map_error::~invalid_map_error() noexcept = default;

value_error::value_error(std::string msg) :
    general_error(std::move(msg)) {}

value_error::~value_error() noexcept = default;

xpath_error::xpath_error(std::string msg) : general_error(std::move(msg)) {}

xpath_error::~xpath_error() noexcept = default;

interface_error::interface_error(std::string msg) : general_error(std::move(msg)) {}

interface_error::~interface_error() noexcept = default;

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

parse_error::parse_error(std::string_view cls, std::string_view msg, std::ptrdiff_t offset) :
    general_error(cls, msg), m_offset(offset)
{
    append_msg(build_offset_msg(offset));
}

parse_error::parse_error(std::string msg, std::ptrdiff_t offset) :
    general_error(std::move(msg)), m_offset(offset)
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

malformed_xml_error::malformed_xml_error(std::string_view msg, std::ptrdiff_t offset) :
    orcus::parse_error("malformed_xml_error", msg, offset) {}

malformed_xml_error::~malformed_xml_error() = default;

zip_error::zip_error(std::string_view msg) : general_error("zip_error", msg)
{
}

zip_error::~zip_error() = default;

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
