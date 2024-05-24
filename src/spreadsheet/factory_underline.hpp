/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <orcus/spreadsheet/import_interface_underline.hpp>
#include <orcus/spreadsheet/document_types.hpp>

#include <memory>

namespace orcus { namespace spreadsheet { namespace detail {

class import_underline : public iface::import_underline
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    import_underline();
    ~import_underline();

    void reset(underline_t* ref);

    virtual void set_style(underline_style_t e) override;
    virtual void set_thickness(underline_thickness_t e) override;
    virtual void set_spacing(underline_spacing_t e) override;
    virtual void set_count(underline_count_t e) override;
    virtual void set_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) override;
    virtual void commit() override;
};

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
