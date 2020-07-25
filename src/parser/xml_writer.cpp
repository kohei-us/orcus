/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/xml_writer.hpp"
#include "orcus/global.hpp"
#include "orcus/xml_namespace.hpp"
#include "orcus/string_pool.hpp"

#include <vector>

namespace orcus {

namespace {

struct _elem
{
    xml_name_t name;
    std::vector<pstring> ns_aliases;
    bool open;

    _elem(const xml_name_t& _name) : name(_name), open(true) {}
};

struct _attr
{
    xml_name_t name;
    pstring value;

    _attr(const xml_name_t& _name, const pstring& _value) :
        name(_name),
        value(_value)
    {}
};

} // anonymous namespace

struct xml_writer::scope::impl
{
    xml_writer* parent;
    xml_name_t elem;

    impl() : parent(nullptr) {}

    impl(xml_writer* _parent, const xml_name_t& _elem) :
        parent(_parent),
        elem(_elem)
    {
        parent->push_element(elem);
    }

    ~impl()
    {
        parent->pop_element();
    }
};

xml_writer::scope::scope(xml_writer* parent, const xml_name_t& elem) : mp_impl(orcus::make_unique<impl>(parent, elem))
{
}

xml_writer::scope::scope(scope&& other) :
    mp_impl(std::move(other.mp_impl))
{
    other.mp_impl = orcus::make_unique<impl>();
}

xml_writer::scope::~scope() {}

xml_writer::scope& xml_writer::scope::operator= (scope&& other)
{
    scope tmp(std::move(other));
    mp_impl.swap(tmp.mp_impl);
    return *this;
}

struct xml_writer::impl
{
    std::ostream& os;
    std::vector<_elem> elem_stack;
    std::vector<pstring> ns_decls;
    std::vector<_attr> attrs;

    string_pool str_pool;
    xmlns_repository repo;
    xmlns_context cxt;

    impl(std::ostream& _os) :
        os(_os),
        cxt(repo.create_context())
    {}

    void print(const xml_name_t& name)
    {
        pstring alias = cxt.get_alias(name.ns);
        if (!alias.empty())
            os << alias << ':';
        os << name.name;
    }

    xml_name_t intern(const xml_name_t& name)
    {
        xml_name_t interned = name;
        interned.name = str_pool.intern(interned.name).first;
        return interned;
    }

    pstring intern(const pstring& value)
    {
        return str_pool.intern(value).first;
    }
};

xml_writer::xml_writer(std::ostream& os) : mp_impl(orcus::make_unique<impl>(os))
{
    os << "<?xml version=\"1.0\"?>";
}

xml_writer::~xml_writer()
{
    // Pop all the elements currently on the stack.
    while (!mp_impl->elem_stack.empty())
        pop_element();
}

void xml_writer::close_current_element()
{
    if (!mp_impl->elem_stack.empty())
    {
        if (mp_impl->elem_stack.back().open)
        {
            mp_impl->os << '>';
            mp_impl->elem_stack.back().open = false;
        }
    }
}

xml_writer::scope xml_writer::set_element_scope(const xml_name_t& name)
{
    return scope(this, name);
}

void xml_writer::push_element(const xml_name_t& _name)
{
    close_current_element();

    auto& os = mp_impl->os;
    xml_name_t name = mp_impl->intern(_name);

    os << '<';
    mp_impl->print(name);

    for (const pstring& alias : mp_impl->ns_decls)
    {
        os << " xmlns";
        if (!alias.empty())
            os << ':' << alias;
        os << "=\"";
        xmlns_id_t ns = mp_impl->cxt.get(alias);
        os << ns << '"';
    }

    for (const _attr& attr : mp_impl->attrs)
    {
        os << ' ';
        mp_impl->print(attr.name);
        os << "=\"";
        os << attr.value << '"';
    }

    mp_impl->attrs.clear();
    mp_impl->ns_decls.clear();

    mp_impl->elem_stack.emplace_back(name);
}

xmlns_id_t xml_writer::add_namespace(const pstring& alias, const pstring& value)
{
    pstring alias_safe = mp_impl->intern(alias);
    xmlns_id_t ns = mp_impl->cxt.push(alias_safe, mp_impl->intern(value));
    mp_impl->ns_decls.push_back(alias_safe);
    return ns;
}

void xml_writer::add_attribute(const xml_name_t& name, const pstring& value)
{
    mp_impl->attrs.emplace_back(mp_impl->intern(name), mp_impl->intern(value));
}

void xml_writer::add_content(const pstring& content)
{
    close_current_element();

    auto& os = mp_impl->os;
    os << content;
}

xml_name_t xml_writer::pop_element()
{
    auto& os = mp_impl->os;

    const _elem& elem = mp_impl->elem_stack.back();
    auto name = elem.name;

    if (elem.open)
    {
        // self-closing element.
        os << "/>";
    }
    else
    {
        os << "</";
        mp_impl->print(name);
        os << '>';
    }

    for (const pstring& alias : mp_impl->elem_stack.back().ns_aliases)
        mp_impl->cxt.pop(alias);

    mp_impl->elem_stack.pop_back();
    return name;
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
