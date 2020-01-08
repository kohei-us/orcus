/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ooxml_namespace_types.hpp"

namespace orcus {

const xmlns_id_t NS_ooxml_a    = "http://schemas.openxmlformats.org/drawingml/2006/main";
const xmlns_id_t NS_ooxml_r    = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";
const xmlns_id_t NS_ooxml_xdr  = "http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing";
const xmlns_id_t NS_ooxml_xlsx = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";

const xmlns_id_t NS_opc_ct  = "http://schemas.openxmlformats.org/package/2006/content-types";
const xmlns_id_t NS_opc_rel = "http://schemas.openxmlformats.org/package/2006/relationships";

const xmlns_id_t NS_mc = "http://schemas.openxmlformats.org/markup-compatibility/2006";
const xmlns_id_t NS_mso_x14 = "http://schemas.microsoft.com/office/spreadsheetml/2009/9/main";

namespace {

xmlns_id_t ooxml_ns[] = {
    NS_ooxml_a,
    NS_ooxml_r,
    NS_ooxml_xdr,
    NS_ooxml_xlsx,
    nullptr
};

xmlns_id_t opc_ns[] = {
    NS_opc_ct,
    NS_opc_rel,
    nullptr
};

xmlns_id_t misc_ns[] = {
    NS_mc,
    NS_mso_x14,
    nullptr
};

}

const xmlns_id_t* NS_ooxml_all = ooxml_ns;
const xmlns_id_t* NS_opc_all = opc_ns;
const xmlns_id_t* NS_misc_all = misc_ns;

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
