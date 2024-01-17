/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/sax_token_parser.hpp"
#include "orcus/tokens.hpp"

#include <mdds/sorted_string_map.hpp>
#include <cctype>

namespace orcus {

namespace {

enum class decl_attr_type { unknown, version, encoding, standalone };

namespace decl_attr {

using map_type = mdds::sorted_string_map<decl_attr_type>;

// Keys must be sorted.
constexpr map_type::entry_type entries[] = {
    { "encoding",   decl_attr_type::encoding   },
    { "standalone", decl_attr_type::standalone },
    { "version",    decl_attr_type::version    },
};

const map_type& get()
{
    static map_type mt(entries, std::size(entries), decl_attr_type::unknown);
    return mt;
}

} // namespace decl_attr

}

sax_token_handler_wrapper_base::sax_token_handler_wrapper_base(const tokens& _tokens) :
    m_tokens(_tokens) {}

xml_token_t sax_token_handler_wrapper_base::tokenize(std::string_view name) const
{
    xml_token_t token = XML_UNKNOWN_TOKEN;
    if (!name.empty())
        token = m_tokens.get_token(name);
    return token;
}

void sax_token_handler_wrapper_base::set_element(const sax_ns_parser_element& elem)
{
    m_elem.ns = elem.ns;
    m_elem.name = tokenize(elem.name);
    m_elem.raw_name = elem.name;
}

void sax_token_handler_wrapper_base::attribute(std::string_view name, std::string_view val)
{
    decl_attr_type dat = decl_attr::get().find(name);

    switch (dat)
    {
        case decl_attr_type::version:
        {
            const char* p = val.data();
            const char* p_end = p + val.size();

            long v;
            const char* endptr = parse_integer(p, p_end, v);

            if (!endptr || endptr >= p_end || *endptr != '.')
                break;

            m_declaration.version_major = v;
            p = endptr + 1;

            endptr = parse_integer(p, p_end, v);

            if (!endptr || endptr > p_end)
                break;

            m_declaration.version_minor = v;
            break;
        }
        case decl_attr_type::encoding:
        {
            m_declaration.encoding = to_character_set(val);
            break;
        }
        case decl_attr_type::standalone:
            m_declaration.standalone = (val == "yes") ? true : false;
            break;
        default:
            ;
    }
}

void sax_token_handler_wrapper_base::attribute(const sax_ns_parser_attribute& attr)
{
    m_elem.attrs.push_back(
       xml_token_attr_t(
           attr.ns, tokenize(attr.name), attr.name,
           attr.value, attr.transient));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
