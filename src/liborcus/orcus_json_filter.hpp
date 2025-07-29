/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <orcus/interface.hpp>
#include <orcus/orcus_json.hpp>

namespace orcus {

namespace spreadsheet { namespace iface {

class import_factory;

}}

class orcus_json_filter : public iface::import_filter
{
    orcus_json m_core;

public:
    orcus_json_filter(spreadsheet::iface::import_factory* im_fact);
    virtual ~orcus_json_filter();

    static bool detect(const unsigned char* blob, std::size_t size);

    virtual void read_stream(std::string_view stream) override;

    virtual void read_file(std::string_view filepath) override;

    virtual void read_file(std::u16string_view filepath) override;

    virtual std::string_view get_name() const override;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
