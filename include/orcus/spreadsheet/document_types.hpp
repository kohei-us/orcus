/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "types.hpp"
#include <vector>

namespace orcus { namespace spreadsheet {

/**
 * Stores a color value in ARGB format.  Each color component ranges from 0 to
 * 255 (8-bit).
 */
struct ORCUS_SPM_DLLPUBLIC color_t
{
    color_elem_t alpha;
    color_elem_t red;
    color_elem_t green;
    color_elem_t blue;

    color_t();
    color_t(color_elem_t _red, color_elem_t _green, color_elem_t _blue);
    color_t(color_elem_t _alpha, color_elem_t _red, color_elem_t _green, color_elem_t _blue);

    void reset();

    bool operator==(const color_t& other) const;
    bool operator!=(const color_t& other) const;
};

struct ORCUS_SPM_DLLPUBLIC format_run
{
    size_t pos;
    size_t size;
    std::string_view font;
    double font_size;
    color_t color;
    bool bold:1;
    bool italic:1;

    format_run();

    void reset();
    bool formatted() const;
};

using format_runs_t = std::vector<format_run>;

}} // namespace orcus::spreadsheet

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
