/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gnumeric_value_format_parser.hpp"
#include <orcus/exception.hpp>
#include <orcus/measurement.hpp>

#include <cassert>
#include <sstream>

namespace orcus {

std::size_t gnumeric_value_format_parser::get_pos() const
{
    return std::distance(m_head, m_cur);
}

void gnumeric_value_format_parser::segment()
{
    // segment: [type=value:start:end]

    assert(*m_cur == '[');
    const char* p0 = nullptr;
    std::size_t pos = 0;

    gnumeric_value_format_segment seg;

    for (++m_cur; m_cur != m_end; ++m_cur)
    {
        if (!p0)
            p0 = m_cur;

        switch (*m_cur)
        {
            case ']':
            {
                if (pos != 2)
                    throw parse_error("value format segment is not formatted properly", get_pos());

                std::string_view s{p0, std::distance(p0, m_cur)};
                if (s.empty())
                    throw parse_error("segment value is empty", get_pos());

                seg.end = to_long(s);
                m_segments.push_back(std::move(seg));
                return;
            }
            case '=':
            {
                std::string_view s{p0, std::distance(p0, m_cur)};
                seg.type = to_gnumeric_value_format_type(s);
                if (seg.type == gnumeric_value_format_type::unknown)
                {
                    std::ostringstream os;
                    os << "invalid value format type '" << s << "'";
                    throw parse_error(os.str(), get_pos());
                }

                p0 = nullptr;
                break;
            }
            case ':':
            {
                std::string_view s{p0, std::distance(p0, m_cur)};

                switch (pos)
                {
                    case 0:
                        seg.value = s;
                        break;
                    case 1:
                        seg.start = to_long(s);
                        break;
                    default:
                        throw parse_error("too many value partitions", get_pos());
                }

                p0 = nullptr;
                ++pos;
                break;
            }
        }
    }

    throw parse_error("']' was never reached", get_pos());
}

gnumeric_value_format_parser::gnumeric_value_format_parser(std::string_view format) :
    m_head(format.data()), m_cur(m_head), m_end(m_head + format.size())
{
}

void gnumeric_value_format_parser::parse()
{
    if (m_cur == m_end)
        return;

    // @[segment][segment][segment]...

    if (*m_cur++ != '@')
        throw parse_error("first character must be '@'", get_pos());

    for (; m_cur != m_end; ++m_cur)
    {
        if (*m_cur != '[')
            throw parse_error("'[' was expected", get_pos());

        segment();
        assert(*m_cur == ']');
    }
}

std::vector<gnumeric_value_format_segment> gnumeric_value_format_parser::pop_segments()
{
    return std::move(m_segments);
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
