/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_ORCUS_XLS_XML_HPP
#define INCLUDED_ORCUS_ORCUS_XLS_XML_HPP

#include "interface.hpp"
#include <memory>

namespace orcus {

namespace spreadsheet { namespace iface { class import_factory; }}

struct orcus_xls_xml_impl;

class ORCUS_DLLPUBLIC orcus_xls_xml : public iface::import_filter
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    orcus_xls_xml(spreadsheet::iface::import_factory* factory);
    ~orcus_xls_xml();

    orcus_xls_xml(const orcus_xls_xml&) = delete;
    orcus_xls_xml& operator= (const orcus_xls_xml&) = delete;

    static bool detect(const unsigned char* blob, size_t size);

    virtual void read_file(const std::string& filepath) override;
    virtual void read_stream(std::string_view stream) override;

    virtual std::string_view get_name() const override;
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
