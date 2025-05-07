/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "debug_state_context.hpp"

#include <ixion/address.hpp>

namespace orcus { namespace spreadsheet { namespace detail {

debug_state_context::debug_state_context() :
    m_resolver(ixion::formula_name_resolver::get(ixion::formula_name_resolver_t::excel_a1, nullptr))
{
    if (!m_resolver)
        throw std::runtime_error("failed to instantiate formula name resolver");
}

std::string debug_state_context::print_range(const ixion::abs_rc_range_t& range) const
{
    ixion::abs_address_t origin;
    ixion::range_t name;
    name.first.row = range.first.row;
    name.first.column = range.first.column;
    name.last.row = range.last.row;
    name.last.column = range.last.column;
    name.set_absolute(false);

    return m_resolver->get_name(name, origin, false);
}

void debug_state_context::ensure_yaml_string(std::ostream& os, std::string_view s) const
{
    if (s.empty())
        // nothing to print
        return;

    bool quote = false;
    const char* p = s.data();
    const char* p_end = p + s.size();
    for (; p != p_end; ++p)
    {
        switch (*p)
        {
            case ':':
            case '{':
            case '}':
            case '[':
            case ']':
            case ',':
            case '&':
            case '*':
            case '#':
            case '?':
            case '|':
                quote = true;
                break;
        }
    }

    // check for a leading "- "
    if (!quote)
    {
        p = s.data();
        if (*p++ == '-')
        {
            if (p == p_end || *p == ' ')
                quote = true;
        }
    }

    if (quote)
    {
        os << '"';
        p = s.data();
        for (; p != p_end; ++p)
        {
            char c = *p;
            if (c == '"')
                os << '\\';
            os << c;
        }
        os << '"';
    }
    else
        os << s;
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
