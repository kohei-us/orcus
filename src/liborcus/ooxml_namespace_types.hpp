/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_OOXML_NAMESPACE_TYPES_HPP
#define INCLUDED_ORCUS_OOXML_NAMESPACE_TYPES_HPP

#include "orcus/types.hpp"

namespace orcus {

extern const xmlns_id_t NS_ooxml_a;
extern const xmlns_id_t NS_ooxml_r;
extern const xmlns_id_t NS_ooxml_xlsx;
extern const xmlns_id_t NS_ooxml_xdr;

extern const xmlns_id_t NS_opc_ct;
extern const xmlns_id_t NS_opc_rel;

extern const xmlns_id_t NS_mc;
extern const xmlns_id_t NS_mso_x14;

/**
 * Null-terminated array of all ooxml namespaces.
 */
extern const xmlns_id_t* NS_ooxml_all;

/**
 * Null-terminated array of all opc namespaces.
 */
extern const xmlns_id_t* NS_opc_all;

/**
 * Null-terminated array of all the other namespaces.
 */
extern const xmlns_id_t* NS_misc_all;

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
