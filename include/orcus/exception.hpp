/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_EXCEPTION_HPP
#define INCLUDED_ORCUS_EXCEPTION_HPP

#include <stdexcept>
#include <string>

#include "env.hpp"

namespace orcus {

class ORCUS_PSR_DLLPUBLIC general_error : public std::exception
{
public:
    explicit general_error(std::string msg);
    explicit general_error(std::string_view cls, std::string_view msg);
    virtual ~general_error() noexcept;
    virtual const char* what() const noexcept;

protected:
    void append_msg(const std::string& s);

private:
    std::string m_msg;
};

class ORCUS_PSR_DLLPUBLIC invalid_arg_error : public std::invalid_argument
{
public:
    explicit invalid_arg_error(const std::string& msg);
    virtual ~invalid_arg_error() noexcept;
};

class ORCUS_PSR_DLLPUBLIC xml_structure_error : public general_error
{
public:
    explicit xml_structure_error(std::string msg);
    virtual ~xml_structure_error() noexcept;
};

class ORCUS_PSR_DLLPUBLIC json_structure_error : public general_error
{
public:
    explicit json_structure_error(std::string msg);
    virtual ~json_structure_error() noexcept;
};

class ORCUS_PSR_DLLPUBLIC invalid_map_error : public general_error
{
public:
    explicit invalid_map_error(std::string msg);
    virtual ~invalid_map_error() noexcept;
};

class ORCUS_PSR_DLLPUBLIC value_error : public general_error
{
public:
    explicit value_error(std::string msg);
    virtual ~value_error() noexcept;
};

/**
 * Error indicating improper xpath syntax.
 */
class ORCUS_PSR_DLLPUBLIC xpath_error : public general_error
{
public:
    xpath_error(std::string msg);
    virtual ~xpath_error() noexcept;
};

/**
 * This gets thrown when a public interface method is expected to return a
 * non-null pointer to another interface but actually returns a null pointer.
 */
class ORCUS_PSR_DLLPUBLIC interface_error : public general_error
{
public:
    interface_error(std::string msg);
    virtual ~interface_error() noexcept;
};

/**
 * Exception related to a parsing error that includes an offset in the stream
 * where the error occurred.
 */
class ORCUS_PSR_DLLPUBLIC parse_error : public general_error
{
    std::ptrdiff_t m_offset;  /// offset in the stream where the error occurred.

protected:
    parse_error(std::string_view cls, std::string_view msg, std::ptrdiff_t offset);

public:
    parse_error(std::string msg, std::ptrdiff_t offset);

    /**
     * Get the offset in a stream associated with the error.
     *
     * @return offset in a stream where the error occurred.
     */
    std::ptrdiff_t offset() const;

    static void throw_with(
        std::string_view msg_before, char c, std::string_view msg_after, std::ptrdiff_t offset);

    static void throw_with(
        std::string_view msg_before, std::string_view msg, std::string_view msg_after, std::ptrdiff_t offset);
};

/**
 * This exception is thrown when SAX parser detects a malformed XML document.
 */
class ORCUS_PSR_DLLPUBLIC malformed_xml_error : public parse_error
{
public:
    malformed_xml_error() = delete;
    malformed_xml_error(std::string_view msg, std::ptrdiff_t offset);
    virtual ~malformed_xml_error();
};

/**
 * Exception related to parsing of zip archive stream.
 */
class ORCUS_PSR_DLLPUBLIC zip_error : public general_error
{
public:
    zip_error(std::string_view msg);
    virtual ~zip_error();
};

namespace detail {

/**
 * Internal error used in multi-threaded parsing to signal that the parser
 * thread has been aborted.
 */
class ORCUS_PSR_DLLPUBLIC parsing_aborted_error : public std::exception {};

}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
