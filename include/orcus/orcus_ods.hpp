/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_ORCUS_ODS_HPP
#define INCLUDED_ORCUS_ORCUS_ODS_HPP

#include "orcus/spreadsheet/import_interface.hpp"
#include "orcus/env.hpp"
#include "interface.hpp"

#include <memory>

namespace orcus {

namespace spreadsheet { namespace iface { class import_factory; }}

struct orcus_ods_impl;
class zip_archive;
class zip_archive_stream;

class ORCUS_DLLPUBLIC orcus_ods : public iface::import_filter
{
    orcus_ods(const orcus_ods&); // disabled
    orcus_ods& operator= (const orcus_ods&); // disabled

public:
    orcus_ods(spreadsheet::iface::import_factory* factory);
    ~orcus_ods();

    static bool detect(const unsigned char* blob, size_t size);

    virtual void read_file(const std::string& filepath) override;

    virtual void read_stream(std::string_view stream) override;

    virtual std::string_view get_name() const override;

private:
    static void list_content(const zip_archive& archive);
    void read_styles(const zip_archive& archive);
    void read_content(const zip_archive& archive);
    void read_content_xml(const unsigned char* p, size_t size);

    void read_file_impl(zip_archive_stream* stream);

private:
    struct impl;
    std::unique_ptr<impl> mp_impl;
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
