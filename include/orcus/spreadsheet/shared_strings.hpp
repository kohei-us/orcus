/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_SHARED_STRINGS_HPP
#define INCLUDED_ORCUS_SPREADSHEET_SHARED_STRINGS_HPP

#include "document_types.hpp"

#include <vector>
#include <memory>
#include <string>

namespace ixion { class model_context; }

namespace orcus {

namespace spreadsheet {

/**
 * This class manages access to a pool of shared string instances for both
 * unformatted strings and rich-text strings.  The underlying string values
 * themselves are stored externally in the `ixion::model_context` instance
 * which this class references; this class itself only stores the format
 * properties of the rich-text strings.
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
     * Set the entire format runs of a string.
     *
     * @param sindex index of the string to associate the format runs with.
     * @param runs format runs.
     */
    void set_format_runs(std::size_t sindex, std::unique_ptr<format_runs_t> runs);

    /**
     * Get the entire format runs of a string.
     *
     * @param index index of the string to get the format runs of.
     *
     * @return pointer to the format runs, or @p nullptr if no format runs exist
     *         for the specified string index.
     */
    const format_runs_t* get_format_runs(std::size_t index) const;

    /**
     * Get an underlying string value associated with an index.
     *
     * @param index index of a string value.
     *
     * @return pointer to a string value associated with the index, or @p
     *         nullptr in case of an invalid string index.
     */
    const std::string* get_string(std::size_t index) const;

    /**
     * @todo Take std::ostream as an output param.
     */
    void dump() const;
};

}}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
