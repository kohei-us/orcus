/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

namespace orcus {

class detection_result
{
    bool m_result;

public:
    detection_result(bool result);

    bool get_result() const;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
