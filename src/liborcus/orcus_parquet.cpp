/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/orcus_parquet.hpp>
#include <orcus/stream.hpp>

namespace orcus {

struct orcus_parquet::impl
{
    spreadsheet::iface::import_factory* factory;

    impl(spreadsheet::iface::import_factory* _factory) : factory(_factory) {}
};

orcus_parquet::orcus_parquet(spreadsheet::iface::import_factory* factory) :
    iface::import_filter(format_t::parquet),
    mp_impl(std::make_unique<impl>(factory))
{
}

orcus_parquet::~orcus_parquet() = default;

bool orcus_parquet::detect(const unsigned char* /*blob*/, size_t /*size*/)
{
    return false;
}

void orcus_parquet::read_file(std::string_view filepath)
{
    file_content fc{filepath};
    read_stream(fc.str());
}

void orcus_parquet::read_stream(std::string_view /*stream*/)
{
}

std::string_view orcus_parquet::get_name() const
{
    return "parquet";
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
