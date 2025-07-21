/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
