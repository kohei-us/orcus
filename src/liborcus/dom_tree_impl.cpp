/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dom_tree_impl.hpp"

namespace orcus { namespace dom { namespace detail {

attr::attr(xmlns_id_t _ns, std::string_view _name, std::string_view _value) :
    name(_ns, _name), value(_value) {}

std::size_t entity_name_hash::operator()(const entity_name& v) const
{
    return std::hash<std::string_view>{}(v.name) ^ reinterpret_cast<std::size_t>(v.ns);
}

void print(std::ostream& os, const entity_name& name, const xmlns_context& cxt)
{
    if (name.ns)
    {
        std::size_t index = cxt.get_index(name.ns);
        if (index != INDEX_NOT_FOUND)
            os << "ns" << index << ':';
    }
    os << name.name;
}

void print(std::ostream& os, const attr& at, const xmlns_context& cxt)
{
    print(os, at.name, cxt);
    os << "=\"";
    escape(os, at.value);
    os << '"';
}

void escape(std::ostream& os, std::string_view val)
{
    if (val.empty())
        return;

    const char* p = val.data();
    const char* p_end = p + val.size();
    for (; p != p_end; ++p)
    {
        switch (*p)
        {
            case '"':
                os << "\\\"";
                break;
            case '\\':
                os << "\\\\";
                break;
            case '\b':
                os << "\\b";
                break;
            case '\f':
                os << "\\f";
                break;
            case '\n':
                os << "\\n";
                break;
            case '\r':
                os << "\\r";
                break;
            case '\t':
                os << "\\t";
                break;
            default:
                os << *p;
        }
    }
}

node::~node() = default;

element::element(xmlns_id_t _ns, std::string_view _name) :
    node(node_type::element), name(_ns, _name) {}

void element::print(std::ostream& os, const xmlns_context& cxt) const
{
    detail::print(os, name, cxt);
}

element::~element() = default;

content::content(std::string_view _value) : node(node_type::content), value(_value) {}

void content::print(std::ostream& os, const xmlns_context& /*cxt*/) const
{
    os << '"';
    escape(os, value);
    os << '"';
}

content::~content() = default;

}}} // namespace orcus::dom::detail

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
