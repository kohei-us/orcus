/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_TEST_GLOBAL_HPP
#define INCLUDED_ORCUS_TEST_GLOBAL_HPP

#ifdef NDEBUG
// release build
#undef NDEBUG
#include <cassert>
#define NDEBUG
#else
// debug build
#include <cassert>
#endif

#include <iostream>
#include <chrono>

namespace orcus { namespace test {

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

}} // namespace orcus::test

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
