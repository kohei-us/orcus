/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_INTERFACE_HPP
#define INCLUDED_ORCUS_INTERFACE_HPP

#include "orcus/env.hpp"
#include "orcus/types.hpp"

#include <string>
#include <memory>

namespace orcus {

struct config;

namespace iface {

/**
 * Base interface for import filters.
 */
class ORCUS_DLLPUBLIC import_filter
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    import_filter(format_t input);
    virtual ~import_filter();

    /**
     * Read the content of a file.
     *
     * @param filepath path to a local file.  It must be a system path.
     */
    virtual void read_file(std::string_view filepath) = 0;

    /**
     * Read the content of an in-memory stream.
     *
     * @param stream in-memory stream to read from.
     */
    virtual void read_stream(std::string_view stream) = 0;

    /**
     * Get the name of a filter.
     *
     * @return name of a filter.
     */
    virtual std::string_view get_name() const = 0;

    void set_config(const orcus::config& v);
    const orcus::config& get_config() const;
};

/**
 * Base interface for document content dumpers.
 */
class ORCUS_DLLPUBLIC document_dumper
{
public:
    virtual ~document_dumper();

    /**
     * Dump the content of a document in a specified format, either into set of
     * multiple files, or a single file.
     *
     * @param format Output format type in which to dump the content.
     * @param output Depending on the output format type, this can be either an
     *               output directory path where multiple output files get
     *               created, or an output file path where the content of the
     *               entire document gets dumped into.
     */
    virtual void dump(dump_format_t format, const std::string& output) const = 0;

    /**
     * Dump the content of a document in a specialized "check" format suitable
     * for content verification.
     *
     * @param os output stream to write the transformed content to.
     */
    virtual void dump_check(std::ostream& os) const = 0;
};

}}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
