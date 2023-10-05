/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifdef __ORCUS_ODS
#define ODS_ENABLED 1
#else
#define ODS_ENABLED 0
#endif

#ifdef __ORCUS_XLSX
#define XLSX_ENABLED 1
#else
#define XLSX_ENABLED 0
#endif

#ifdef __ORCUS_GNUMERIC
#define GNUMERIC_ENABLED 1
#else
#define GNUMERIC_ENABLED 0
#endif

#ifdef __ORCUS_XLS_XML
#define XLS_XML_ENABLED 1
#else
#define XLS_XML_ENABLED 0
#endif

#ifdef __ORCUS_PARQUET
#define PARQUET_ENABLED 1
#else
#define PARQUET_ENABLED 0
#endif

#include <orcus/format_detection.hpp>
#include <orcus/orcus_csv.hpp>

#if ODS_ENABLED
#include <orcus/orcus_ods.hpp>
#endif
#if XLSX_ENABLED
#include <orcus/orcus_xlsx.hpp>
#endif
#if GNUMERIC_ENABLED
#include <orcus/orcus_gnumeric.hpp>
#endif
#if XLS_XML_ENABLED
#include <orcus/orcus_xls_xml.hpp>
#endif
#if PARQUET_ENABLED
#include <orcus/orcus_parquet.hpp>
#endif

#include <stdexcept>

namespace ss = orcus::spreadsheet;

namespace orcus {

format_t detect(std::string_view strm) try
{
    const auto* p = reinterpret_cast<const unsigned char*>(strm.data());

#if ODS_ENABLED
    if (orcus_ods::detect(p, strm.size()))
        return format_t::ods;
#endif
#if XLSX_ENABLED
    if (orcus_xlsx::detect(p, strm.size()))
        return format_t::xlsx;
#endif
#if GNUMERIC_ENABLED
    if (orcus_gnumeric::detect(p, strm.size()))
        return format_t::gnumeric;
#endif
#if XLS_XML_ENABLED
    if (orcus_xls_xml::detect(p, strm.size()))
        return format_t::xls_xml;
#endif
#if PARQUET_ENABLED
    if (orcus_parquet::detect(p, strm.size()))
        return format_t::parquet;
#endif

    return format_t::unknown;
}
catch (const std::exception&)
{
    return format_t::unknown;
}
catch (...)
{
    return format_t::unknown;
}

std::shared_ptr<iface::import_filter> create_filter(format_t type, ss::iface::import_factory* factory)
{
    if (!factory)
        throw std::invalid_argument("pointer to import factory instance must not be null");
    switch (type)
    {
#if ODS_ENABLED
        case format_t::ods:
            return std::allocate_shared<orcus_ods>(std::allocator<orcus_ods>{}, factory);
#endif
#if XLSX_ENABLED
        case format_t::xlsx:
            return std::allocate_shared<orcus_xlsx>(std::allocator<orcus_xlsx>{}, factory);
#endif
#if GNUMERIC_ENABLED
        case format_t::gnumeric:
            return std::allocate_shared<orcus_gnumeric>(std::allocator<orcus_gnumeric>{}, factory);
#endif
#if XLS_XML_ENABLED
        case format_t::xls_xml:
            return std::allocate_shared<orcus_xls_xml>(std::allocator<orcus_xls_xml>{}, factory);
#endif
#if PARQUET_ENABLED
        case format_t::parquet:
            return std::allocate_shared<orcus_parquet>(std::allocator<orcus_parquet>{}, factory);
#endif
        case format_t::csv:
            return std::allocate_shared<orcus_csv>(std::allocator<orcus_csv>{}, factory);
        case format_t::unknown:
        default:;
    }
    return {};
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
