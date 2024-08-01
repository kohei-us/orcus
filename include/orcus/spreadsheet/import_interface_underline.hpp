/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "../env.hpp"
#include "types.hpp"

namespace orcus { namespace spreadsheet { namespace iface {

/**
 * Interface for importing underline attributes of a text.
 */
class ORCUS_DLLPUBLIC import_underline
{
public:
    ~import_underline();

    /**
     * Set the style of an underline.
     *
     * @param e underline style of a font.
     */
    virtual void set_style(underline_style_t e) = 0;

    /**
     * Set the thickness of an underline.
     *
     * @param e Thickness of the underline.
     */
    virtual void set_thickness(underline_thickness_t e) = 0;

    /**
     * Set the spacing of an underline with respect to the text it is applied
     * to.
     *
     * @param e Spacing of an underline.
     */
    virtual void set_spacing(underline_spacing_t e) = 0;

    /**
     * Set the number of vertically-stacked lines in an underline.
     *
     * @param e Number of vertically-stacked lines in an underline.
     */
    virtual void set_count(underline_count_t e) = 0;

    /**
     * Specify the color of an underline in ARGB format.
     *
     * @param alpha alpha component of the color.
     * @param red red component of the color.
     * @param green green component of the color.
     * @param blue blue component of the color.
     *
     * @note If this value is not explicitly set, the font color should be used.
     */
    virtual void set_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) = 0;

    /**
     * Commit the underline attributes in the current buffer.
     */
    virtual void commit() = 0;
};

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
