/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/spreadsheet/shared_strings.hpp>
#include <ixion/model_context.hpp>

#include <iostream>
#include <algorithm>
#include <unordered_map>

namespace orcus { namespace spreadsheet {

// format runs for all shared strings, mapped by string IDs.
using format_runs_map_type = std::unordered_map<size_t, std::unique_ptr<format_runs_t>>;

struct shared_strings::impl
{
    ixion::model_context& context;

    /**
     * Container for all format runs of all formatted strings.  Format runs
     * are mapped with the string IDs.
     */
    format_runs_map_type formats;

    impl(ixion::model_context& cxt) : context(cxt) {}
};

shared_strings::shared_strings(ixion::model_context& cxt) : mp_impl(std::make_unique<impl>(cxt)) {}

shared_strings::~shared_strings() = default;

void shared_strings::set_format_runs(std::size_t sindex, std::unique_ptr<format_runs_t> runs)
{
    mp_impl->formats.insert_or_assign(sindex, std::move(runs));
}

const format_runs_t* shared_strings::get_format_runs(std::size_t index) const
{
    auto it = mp_impl->formats.find(index);
    if (it != mp_impl->formats.end())
        return it->second.get();
    return nullptr;
}

const std::string* shared_strings::get_string(std::size_t index) const
{
    return mp_impl->context.get_string(index);
}

void shared_strings::dump() const
{
    std::cout << "number of shared strings: " << mp_impl->context.get_string_count() << std::endl;
}

}}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
