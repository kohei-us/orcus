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

#include <iostream>

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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
