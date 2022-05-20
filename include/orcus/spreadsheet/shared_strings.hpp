/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_SHARED_STRINGS_HPP
#define INCLUDED_ORCUS_SPREADSHEET_SHARED_STRINGS_HPP

#include <orcus/spreadsheet/styles.hpp>
#include <orcus/env.hpp>

#include <vector>
#include <memory>
#include <string>

namespace ixion { class model_context; }

namespace orcus {

namespace spreadsheet {

struct ORCUS_SPM_DLLPUBLIC format_run
{
    size_t pos;
    size_t size;
    std::string_view font;
    double font_size;
    color_t color;
    bool bold:1;
    bool italic:1;

    format_run();

    void reset();
    bool formatted() const;
};

typedef std::vector<format_run> format_runs_t;

/**
 * This class handles global pool of string instances.
 */
class ORCUS_SPM_DLLPUBLIC shared_strings
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    shared_strings() = delete;
    shared_strings(const shared_strings&) = delete;
    shared_strings& operator=(const shared_strings&) = delete;

    shared_strings(ixion::model_context& cxt);
    ~shared_strings();

    /**
     * Set the entire format runs for a string.
     *
     * @param sindex index of the string to associated the format runs with.
     * @param runs format runs.
     */
    void set_format_runs(std::size_t sindex, std::unique_ptr<format_runs_t> runs);

    const format_runs_t* get_format_runs(size_t index) const;

    const std::string* get_string(size_t index) const;

    void dump() const;
};

}}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
