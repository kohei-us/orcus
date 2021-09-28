/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xpath_parser.hpp"
#include "orcus/exception.hpp"
#include "orcus/xml_namespace.hpp"

#define ORCUS_DEBUG_XPATH_PARSER 0

namespace orcus {

xpath_parser::token::token(xmlns_id_t _ns, std::string_view _name, bool _attribute) :
    ns(_ns), name(_name), attribute(_attribute)
{
#if ORCUS_DEBUG_XPATH_PARSER
    cout << "xpath_parser::token: (ns='" << (ns ? ns : "none") << "', name='" << name << "', attribute=" << attribute << ")" << endl;
#endif
}

xpath_parser::token::token() : ns(XMLNS_UNKNOWN_ID), attribute(false) {}
xpath_parser::token::token(const token& r) : ns(r.ns), name(r.name), attribute(r.attribute) {}

xpath_parser::xpath_parser(const xmlns_context& cxt, const char* p, size_t n, xmlns_id_t default_ns) :
    m_cxt(cxt),
    mp_char(p),
    mp_end(p+n),
    m_default_ns(default_ns)
{
    if (!n)
        throw xpath_error("empty path");

    if (*p != '/')
        throw xpath_error("first character must be '/'.");

    ++mp_char;
}

xpath_parser::token xpath_parser::next()
{
    if (mp_char == mp_end)
        return token();

    const char* p0 = nullptr;
    size_t len = 0;
    xmlns_id_t ns = m_default_ns;

    bool attribute = *mp_char == '@';
    if (attribute)
        ++mp_char;

    for (; mp_char != mp_end; ++mp_char, ++len)
    {
        if (!p0)
        {
            p0 = mp_char;
            len = 0;
        }

        switch (*mp_char)
        {
            case '/':
            {
                ++mp_char; // skip the '/'.
                return token(ns, std::string_view(p0, len), attribute);
            }
            case ':':
            {
                // What comes before ':' is a namespace. Reset the name and
                // convert the namespace to a proper ID.
                std::string_view ns_name(p0, len);
                ns = m_cxt.get(ns_name);
                p0 = nullptr; // reset the name.
                break;
            }
            default:
                ;
        }
    }

    // '/' has never been encountered.  It must be the last name in the path.
    return token(ns, std::string_view(p0, len), attribute);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
