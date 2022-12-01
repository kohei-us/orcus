/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_ORCUS_CSV_HPP
#define ORCUS_ORCUS_CSV_HPP

#include "interface.hpp"

namespace orcus {

namespace spreadsheet { namespace iface {
    class import_factory;
}}

class ORCUS_DLLPUBLIC orcus_csv : public iface::import_filter
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    orcus_csv() = delete;
    orcus_csv(const orcus_csv&) = delete;
    orcus_csv& operator=(const orcus_csv&) = delete;

    orcus_csv(spreadsheet::iface::import_factory* factory);
    ~orcus_csv();

    virtual void read_file(std::string_view filepath) override;
    virtual void read_stream(std::string_view stream) override;

    virtual std::string_view get_name() const override;
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
