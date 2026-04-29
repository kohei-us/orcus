/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

// NB: This header cannot depend on orcus API.

#ifdef NDEBUG
// release build
#undef NDEBUG
#include <cassert>
#else
// debug build
#include <cassert>
#endif

#include <iostream>
#include <sstream>
#include <chrono>

namespace orcus {

class file_content;

namespace test {

class stack_printer
{
public:
    explicit stack_printer(const char* msg);
    ~stack_printer();

private:
    double get_time() const;

    std::string m_msg;
    double m_start_time;
};

class assert_error : public std::exception
{
    std::string m_msg;

public:
    assert_error(const char* filename, size_t line_no, const char* msg);

    virtual const char* what() const noexcept override;
};

void verify_content(
    const char* filename, size_t line_no, std::string_view expected, const std::string& actual);

template<typename EnumT>
bool verify_stream_value(EnumT v, std::string_view expected)
{
    std::ostringstream os;
    os << v;
    return os.str() == expected;
}

orcus::file_content to_file_content(const std::string& path);
orcus::file_content to_file_content(const std::wstring& path);

void print_path(const std::string& path);
void print_path(const std::wstring& path);

}} // namespace orcus::test

#define ORCUS_TEST_FUNC_SCOPE orcus::test::stack_printer __sp__(__func__)

#define ORCUS_TEST_EXPECT_THROW(expr, exception_type) \
    do { \
        bool thrown = false; \
        try { \
            expr; \
        } catch (const exception_type&) { \
            thrown = true; \
        } catch (...) { \
            std::cerr << __FILE__ << ":" << __LINE__ \
                      << " expected exception of type '" #exception_type "' but got a different exception" << std::endl; \
            assert(false); \
        } \
        if (!thrown) { \
            std::cerr << __FILE__ << ":" << __LINE__ \
                      << " expected exception of type '" #exception_type "' but no exception was thrown" << std::endl; \
            assert(false); \
        } \
    } while (false)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
