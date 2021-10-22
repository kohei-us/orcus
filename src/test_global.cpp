/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"

#include <orcus/stream.hpp>
#include <orcus/parser_global.hpp>

namespace orcus { namespace test {

stack_printer::stack_printer(const char* msg) :
    m_msg(msg)
{
    std::cerr << m_msg << ": --begin" << std::endl;
    m_start_time = get_time();
}

stack_printer::~stack_printer()
{
    double end_time = get_time();
    std::cerr << m_msg << ": --end (duration: " << (end_time-m_start_time) << " sec)" << std::endl;
}

double stack_printer::get_time() const
{
    double v = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
    return v / 1000.0;
}

assert_error::assert_error(const char* filename, size_t line_no, const char* msg)
{
    std::ostringstream os;
    os << filename << ":" << line_no << ": " << msg;
    m_msg = os.str();
}

const char* assert_error::what() const noexcept
{
    return m_msg.data();
}

void verify_content(
    const char* filename, size_t line_no, std::string_view expected, const std::string& actual)
{
    std::string_view s1 = expected;
    std::string_view s2(actual.data(), actual.size());
    s1 = trim(s1);
    s2 = trim(s2);

    if (s1 != s2)
    {
        // TODO : improve the error message to make it more viewer-friendly.

        size_t diff_pos = locate_first_different_char(s1, s2);
        std::string msg_s1 = create_parse_error_output(s1, diff_pos);
        std::string msg_s2 = create_parse_error_output(s2, diff_pos);

        std::ostringstream os;
        os << "content is not as expected: " << std::endl << std::endl
            << "* expected:" << std::endl << std::endl
            << msg_s1 << std::endl
            << "* actual:" << std::endl << std::endl
            << msg_s2;

        throw assert_error(filename, line_no, os.str().data());
    }
}

}} // namespace orcus::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
