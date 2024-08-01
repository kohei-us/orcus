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
 * Interface for importing strikethrough attributes of a text.
 */
class ORCUS_DLLPUBLIC import_strikethrough
{
public:
    virtual ~import_strikethrough();

    /**
     * Set the strikethrough style of a font.
     *
     * @param s strikethrough style of a font.
     */
    virtual void set_style(strikethrough_style_t s) = 0;

    /**
     * Set whether the strikethrough of a font consists of a single line or a
     * double line.
     *
     * @param s whether the strikethrough of a font consists of a single line or
     *          a double line.
     */
    virtual void set_type(strikethrough_type_t s) = 0;

    /**
     * Set the width of the strikethrough of a font.
     *
     * @param s the width of the strikethrough of a font.
     */
    virtual void set_width(strikethrough_width_t s) = 0;

    /**
     * Set the text to use as a strikethrough.
     *
     * @param s text to use as a strikethrough.
     */
    virtual void set_text(strikethrough_text_t s) = 0;

    /**
     * Commit the strikethrough attributes in the current buffer.
     */
    virtual void commit() = 0;
};

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
