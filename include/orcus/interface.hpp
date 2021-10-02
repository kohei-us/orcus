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

class ORCUS_DLLPUBLIC import_filter
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    import_filter(format_t input);
    virtual ~import_filter();

    /// expects a system path to a local file
    virtual void read_file(const std::string& filepath) = 0;

    /// expects the whole content of the file
    virtual void read_stream(std::string_view stream) = 0;

    virtual std::string_view get_name() const = 0;

    void set_config(const orcus::config& v);
    const orcus::config& get_config() const;
};

class ORCUS_DLLPUBLIC document_dumper
{
public:
    virtual ~document_dumper();
    virtual void dump(dump_format_t format, const std::string& output) const = 0;
    virtual void dump_check(std::ostream& os) const = 0;
};

}}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
