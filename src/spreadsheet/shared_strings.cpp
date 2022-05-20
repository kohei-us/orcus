/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/spreadsheet/shared_strings.hpp>
#include <orcus/global.hpp>
#include <ixion/model_context.hpp>

#include <iostream>
#include <algorithm>
#include <unordered_map>

using namespace std;

namespace orcus { namespace spreadsheet {

// format runs for all shared strings, mapped by string IDs.
using format_runs_map_type = std::unordered_map<size_t, std::unique_ptr<format_runs_t>>;

struct shared_strings::impl
{
    ixion::model_context& m_cxt;

    /**
     * Container for all format runs of all formatted strings.  Format runs
     * are mapped with the string IDs.
     */
    format_runs_map_type m_formats;

    impl(ixion::model_context& cxt) : m_cxt(cxt) {}
};

shared_strings::shared_strings(ixion::model_context& cxt) : mp_impl(std::make_unique<impl>(cxt)) {}

shared_strings::~shared_strings() {}

void shared_strings::set_format_runs(std::size_t sindex, std::unique_ptr<format_runs_t> runs)
{
    mp_impl->m_formats.insert_or_assign(sindex, std::move(runs));
}

const format_runs_t* shared_strings::get_format_runs(size_t index) const
{
    auto it = mp_impl->m_formats.find(index);
    if (it != mp_impl->m_formats.end())
        return it->second.get();
    return nullptr;
}

const string* shared_strings::get_string(size_t index) const
{
    return mp_impl->m_cxt.get_string(index);
}

namespace {

struct print_string
{
    size_t m_count;
public:
    print_string() : m_count(1) {}
    void operator() (std::string_view ps)
    {
        cout << m_count++ << ": '" << ps << "'" << endl;
    }
};

}

void shared_strings::dump() const
{
    cout << "number of shared strings: " << mp_impl->m_cxt.get_string_count() << endl;
}

}}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
