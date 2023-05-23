/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "./interface.hpp"
#include "./spreadsheet/import_interface.hpp"

namespace orcus {

namespace spreadsheet { namespace iface { class import_factory; }}

class ORCUS_DLLPUBLIC orcus_parquet : public iface::import_filter
{
public:
    orcus_parquet(const orcus_parquet&) = delete;
    orcus_parquet& operator=(const orcus_parquet&) = delete;

    orcus_parquet(spreadsheet::iface::import_factory* factory);
    ~orcus_parquet();

    static bool detect(const unsigned char* blob, size_t size);

    virtual void read_file(std::string_view filepath) override;

    virtual void read_stream(std::string_view stream) override;

    virtual std::string_view get_name() const override;

private:
    class impl;
    std::unique_ptr<impl> mp_impl;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
