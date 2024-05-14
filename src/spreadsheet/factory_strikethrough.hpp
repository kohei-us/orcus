/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <orcus/spreadsheet/import_interface_strikethrough.hpp>
#include <orcus/spreadsheet/document_types.hpp>

#include <memory>

namespace orcus { namespace spreadsheet { namespace detail {

class import_strikethrough : public iface::import_strikethrough
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    import_strikethrough();
    ~import_strikethrough();

    void reset(strikethrough_t* ref);

    virtual void set_style(strikethrough_style_t s) override;
    virtual void set_type(strikethrough_type_t s) override;
    virtual void set_width(strikethrough_width_t s) override;
    virtual void set_text(strikethrough_text_t s) override;
    virtual void commit() override;
};

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
