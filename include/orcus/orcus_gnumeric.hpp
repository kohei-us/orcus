/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_ORCUS_GNUMERIC_HPP
#define ORCUS_ORCUS_GNUMERIC_HPP

#include "interface.hpp"

#include <memory>

namespace orcus {

namespace spreadsheet { namespace iface { class import_factory; }}

class ORCUS_DLLPUBLIC orcus_gnumeric : public iface::import_filter
{
    struct impl;
    std::unique_ptr<impl> mp_impl;
public:
    orcus_gnumeric() = delete;
    orcus_gnumeric(const orcus_gnumeric&) = delete;
    orcus_gnumeric& operator=(const orcus_gnumeric&) = delete;

    orcus_gnumeric(spreadsheet::iface::import_factory* factory);
    ~orcus_gnumeric();

    static bool detect(const unsigned char* blob, size_t size);

    virtual void read_file(const std::string& filepath) override;

    virtual void read_stream(std::string_view stream) override;

    virtual std::string_view get_name() const override;
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
