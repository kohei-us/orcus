/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/xml_namespace.hpp>
#include <orcus/exception.hpp>
#include <orcus/string_pool.hpp>

#include <unordered_map>
#include <vector>
#include <limits>
#include <sstream>
#include <algorithm>
#include <cassert>

namespace orcus {

namespace {

using xmlns_list_type = std::vector<xmlns_id_t>;
using alias_map_type = std::unordered_map<std::string_view, xmlns_list_type>;

// builtin namespace 'xml'
constexpr std::string_view NS_XML_ALIAS = "xml";
constexpr std::string_view NS_XML_URI = "http://www.w3.org/XML/1998/namespace";

}

typedef std::unordered_map<std::string_view, std::size_t> strid_map_type;

struct xmlns_repository::impl
{
    size_t m_predefined_ns_size;
    string_pool m_pool; /// storage of live string instances.
    std::vector<std::string_view> m_identifiers; /// map strings to numerical identifiers.
    strid_map_type m_strid_map; /// string-to-numerical identifiers map for quick lookup.

    impl() : m_predefined_ns_size(0) {}
};

xmlns_repository::xmlns_repository() : mp_impl(std::make_unique<impl>()) {}
xmlns_repository::xmlns_repository(xmlns_repository&& other) : mp_impl(std::move(other.mp_impl)) {}
xmlns_repository::~xmlns_repository() = default;

xmlns_repository& xmlns_repository::operator= (xmlns_repository&& other)
{
    mp_impl = std::move(other.mp_impl);
    return *this;
}

xmlns_id_t xmlns_repository::intern(std::string_view uri)
{
    // See if the uri is already registered.
    strid_map_type::iterator it = mp_impl->m_strid_map.find(uri);
    if (it != mp_impl->m_strid_map.end())
        return it->first.data();

    try
    {
        auto r = mp_impl->m_pool.intern(uri);
        std::string_view uri_interned = r.first;

        if (!uri_interned.empty())
        {
            // Intern successful.
            if (r.second)
            {
                // This is a new instance. Assign a numerical identifier.
                mp_impl->m_strid_map.insert(
                    strid_map_type::value_type(r.first, mp_impl->m_identifiers.size()));
                mp_impl->m_identifiers.push_back(r.first);

                assert(mp_impl->m_pool.size()+mp_impl->m_predefined_ns_size == mp_impl->m_identifiers.size());
                assert(mp_impl->m_pool.size()+mp_impl->m_predefined_ns_size == mp_impl->m_strid_map.size());
            }
            return uri_interned.data();
        }
    }
    catch (const general_error&)
    {
    }

    return XMLNS_UNKNOWN_ID;
}

void xmlns_repository::add_predefined_values(const xmlns_id_t* predefined_ns)
{
    if (!predefined_ns)
        return;

    const xmlns_id_t* val = &predefined_ns[0];
    for (; *val; ++val)
    {
        std::string_view s(*val);
        mp_impl->m_strid_map.insert(
            strid_map_type::value_type(s, mp_impl->m_identifiers.size()));
        mp_impl->m_identifiers.push_back(s);

        ++mp_impl->m_predefined_ns_size;
    }
}

xmlns_context xmlns_repository::create_context()
{
    return xmlns_context(*this);
}

xmlns_id_t xmlns_repository::get_identifier(size_t index) const
{
    if (index >= mp_impl->m_identifiers.size())
        return XMLNS_UNKNOWN_ID;

    // All identifier strings are interned which means they are all null-terminated.
    return mp_impl->m_identifiers[index].data();
}

std::string xmlns_repository::get_short_name(xmlns_id_t ns_id) const
{
    size_t index = get_index(ns_id);

    if (index == INDEX_NOT_FOUND)
        return std::string("???");

    std::ostringstream os;
    os << "ns" << index;
    return os.str();
}

size_t xmlns_repository::get_index(xmlns_id_t ns_id) const
{
    if (!ns_id)
        return INDEX_NOT_FOUND;

    auto it = mp_impl->m_strid_map.find(std::string_view(ns_id));
    if (it == mp_impl->m_strid_map.end())
        return INDEX_NOT_FOUND;

    return it->second;
}

struct xmlns_context::impl
{
    xmlns_repository* repo = nullptr;
    xmlns_list_type m_all_ns; /// all namespaces ever used in this context.
    xmlns_list_type m_default;
    alias_map_type m_map;
    alias_map_type m_builtin_map; /// built-in aliases not tracked as declared namespaces.

    bool m_trim_all_ns = true;

    impl() {}
    impl(xmlns_repository& _repo) : repo(&_repo)
    {
        register_builtin(NS_XML_ALIAS, NS_XML_URI);
    }

    impl(const impl& r) :
        repo(r.repo), m_all_ns(r.m_all_ns), m_default(r.m_default), m_map(r.m_map),
        m_builtin_map(r.m_builtin_map), m_trim_all_ns(r.m_trim_all_ns) {}

    void register_builtin(std::string_view alias, std::string_view uri)
    {
        if (!repo)
            throw general_error("this context is not associated with any repo.");

        xmlns_id_t id = uri.data();

        auto it = m_builtin_map.find(alias);
        if (it == m_builtin_map.end())
        {
            xmlns_list_type nslist;
            nslist.push_back(id);
            auto r = m_builtin_map.insert(alias_map_type::value_type(alias, nslist));
            if (!r.second)
                throw general_error("failed to insert built-in namespace");
        }
        else
        {
            it->second.push_back(id);
        }
    }
};

xmlns_context::xmlns_context() : mp_impl(std::make_unique<impl>()) {}
xmlns_context::xmlns_context(xmlns_repository& repo) : mp_impl(std::make_unique<impl>(repo)) {}
xmlns_context::xmlns_context(const xmlns_context& r) : mp_impl(std::make_unique<impl>(*r.mp_impl)) {}
xmlns_context::xmlns_context(xmlns_context&& r) : mp_impl(std::move(r.mp_impl))
{
    r.mp_impl = std::make_unique<impl>();
}

xmlns_context::~xmlns_context() = default;

xmlns_context& xmlns_context::operator= (const xmlns_context& r)
{
    xmlns_context tmp(r);
    tmp.swap(*this);
    return *this;
}

xmlns_context& xmlns_context::operator= (xmlns_context&& r)
{
    xmlns_context tmp(std::move(r));
    tmp.swap(*this);
    return *this;
}

xmlns_id_t xmlns_context::push(std::string_view alias, std::string_view uri)
{
    if (!mp_impl->repo)
        throw general_error("this context is not associated with any repo.");

    if (alias == NS_XML_ALIAS)
    {
        std::ostringstream os;
        os << "builtin namespace '" << NS_XML_ALIAS << "' cannot be explicitly declared";
        throw std::invalid_argument(os.str());
    }

    mp_impl->m_trim_all_ns = true;

    xmlns_id_t id = mp_impl->repo->intern(uri);
    std::string_view uri_interned = id ? std::string_view(id) : std::string_view();

    if (alias.empty())
    {
        // empty alias value is associated with default namespace.
        mp_impl->m_default.push_back(uri_interned.data());
        mp_impl->m_all_ns.push_back(uri_interned.data());
        return mp_impl->m_default.back();
    }

    // See if this alias already exists.
    alias_map_type::iterator it = mp_impl->m_map.find(alias);
    if (it == mp_impl->m_map.end())
    {
        // This is the first time this alias is used.
        xmlns_list_type nslist;
        nslist.push_back(uri_interned.data());
        mp_impl->m_all_ns.push_back(uri_interned.data());
        std::pair<alias_map_type::iterator,bool> r =
            mp_impl->m_map.insert(alias_map_type::value_type(alias, nslist));

        if (!r.second)
            // insertion failed.
            throw general_error("Failed to insert new namespace.");

        return nslist.back();
    }

    // The alias already exists.
    xmlns_list_type& nslist = it->second;
    nslist.push_back(uri_interned.data());
    mp_impl->m_all_ns.push_back(uri_interned.data());
    return nslist.back();
}

void xmlns_context::pop(std::string_view alias)
{
    if (alias.empty())
    {
        // empty alias value is associated with default namespace.
        if (mp_impl->m_default.empty())
            throw general_error("default namespace stack is empty.");

        mp_impl->m_default.pop_back();
        return;
    }

    // See if this alias really exists.
    alias_map_type::iterator it = mp_impl->m_map.find(alias);
    if (it == mp_impl->m_map.end())
    {
        std::ostringstream os;
        os << "alias named '" << alias << "' was attempted to be popped, but was not found in the stack";
        throw general_error(os.str());
    }

    xmlns_list_type& nslist = it->second;
    if (nslist.empty())
        throw general_error("namespace stack for this key is empty.");

    nslist.pop_back();
}

xmlns_id_t xmlns_context::get(std::string_view alias) const
{
    if (alias.empty())
        return mp_impl->m_default.empty() ? XMLNS_UNKNOWN_ID : mp_impl->m_default.back();

    alias_map_type::const_iterator it = mp_impl->m_map.find(alias);
    if (it == mp_impl->m_map.end())
    {
        // Check built-in aliases second (e.g. the predefined "xml" prefix).
        it = mp_impl->m_builtin_map.find(alias);
        if (it == mp_impl->m_builtin_map.end())
            return XMLNS_UNKNOWN_ID;
    }

    return it->second.empty() ? XMLNS_UNKNOWN_ID : it->second.back();
}

size_t xmlns_context::get_index(xmlns_id_t ns_id) const
{
    if (!mp_impl->repo)
        throw general_error("this context is not associated with any repo.");

    return mp_impl->repo->get_index(ns_id);
}

std::string xmlns_context::get_short_name(xmlns_id_t ns_id) const
{
    if (!mp_impl->repo)
        throw general_error("this context is not associated with any repo.");

    return mp_impl->repo->get_short_name(ns_id);
}

std::string_view xmlns_context::get_alias(xmlns_id_t ns_id) const
{
    alias_map_type::const_iterator it = mp_impl->m_map.begin(), it_end = mp_impl->m_map.end();
    for (; it != it_end; ++it)
    {
        const xmlns_list_type& lst = it->second;
        if (lst.empty())
            continue;

        if (lst.back() == ns_id)
            return it->first;
    }

    return std::string_view{};
}

namespace {

struct ns_item
{
    size_t index;
    xmlns_id_t ns;

    ns_item(size_t _index, xmlns_id_t _ns) : index(_index), ns(_ns) {}
};

struct less_ns_by_index
{
    bool operator() (const ns_item& left, const ns_item& right) const
    {
        return left.index < right.index;
    }
};

class push_back_ns_to_item
{
    std::vector<ns_item>& m_store;
    const xmlns_context& m_cxt;
public:
    push_back_ns_to_item(std::vector<ns_item>& store, const xmlns_context& cxt) : m_store(store), m_cxt(cxt) {}
    void operator() (xmlns_id_t ns)
    {
        size_t num_id = m_cxt.get_index(ns);
        if (num_id != INDEX_NOT_FOUND)
            m_store.push_back(ns_item(num_id, ns));
    }
};

class push_back_item_to_ns
{
    std::vector<xmlns_id_t>& m_store;
public:
    push_back_item_to_ns(std::vector<xmlns_id_t>& store) : m_store(store) {}
    void operator() (const ns_item& item)
    {
        m_store.push_back(item.ns);
    }
};

}

std::vector<xmlns_id_t> xmlns_context::get_all_namespaces() const
{
    std::vector<xmlns_id_t> nslist;

    if (mp_impl->m_trim_all_ns)
    {
        xmlns_list_type& all_ns = mp_impl->m_all_ns;

        nslist.assign(mp_impl->m_all_ns.begin(), mp_impl->m_all_ns.end());

        // Sort it and remove duplicate.
        std::sort(all_ns.begin(), all_ns.end());
        xmlns_list_type::iterator it_unique_end =
            std::unique(all_ns.begin(), all_ns.end());
        all_ns.erase(it_unique_end, all_ns.end());

        // Now, sort by indices.
        std::vector<ns_item> items;
        std::for_each(all_ns.begin(), all_ns.end(), push_back_ns_to_item(items, *this));
        std::sort(items.begin(), items.end(), less_ns_by_index());

        all_ns.clear();
        std::for_each(items.begin(), items.end(), push_back_item_to_ns(all_ns));

        mp_impl->m_trim_all_ns = false;
    }

    nslist.assign(mp_impl->m_all_ns.begin(), mp_impl->m_all_ns.end());
    return nslist;
}

void xmlns_context::dump(std::ostream& os) const
{
    std::vector<xmlns_id_t> nslist = get_all_namespaces();
    std::vector<xmlns_id_t>::const_iterator it = nslist.begin(), it_end = nslist.end();
    for (; it != it_end; ++it)
    {
        xmlns_id_t ns_id = *it;
        size_t num_id = get_index(ns_id);
        if (num_id == INDEX_NOT_FOUND)
            continue;

        os << "ns" << num_id << "=\"" << ns_id << '"' << std::endl;
    }
}

void xmlns_context::dump_state(std::ostream& os) const
{
    os << "namespaces:" << std::endl;
    for (xmlns_id_t ns_id : get_all_namespaces())
    {
        size_t num_id = get_index(ns_id);
        if (num_id == INDEX_NOT_FOUND)
            continue;

        os << "  ns" << num_id << ": \"" << ns_id << '"' << std::endl;
    }

    os << "aliases:" << std::endl;
    for (const auto& [alias, ns_list] : mp_impl->m_map)
    {
        os << "  " << alias << ":" << std::endl;

        for (const xmlns_id_t ns : ns_list)
            os << "    - " << ns << std::endl;
    }
}

void xmlns_context::swap(xmlns_context& other) noexcept
{
    mp_impl.swap(other.mp_impl);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
