/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "factory_strikethrough.hpp"

#include <cassert>

namespace orcus { namespace spreadsheet { namespace detail {

struct import_strikethrough::impl
{
    strikethrough_t* ref = nullptr; // destination to commit to
    strikethrough_t buf;
};

import_strikethrough::import_strikethrough() :
    mp_impl(std::make_unique<impl>())
{
}

import_strikethrough::~import_strikethrough() = default;

void import_strikethrough::reset(strikethrough_t* ref)
{
    assert(ref);
    mp_impl->ref = ref;
    mp_impl->buf.reset();
}

void import_strikethrough::set_style(strikethrough_style_t s)
{
    mp_impl->buf.style = s;
}

void import_strikethrough::set_type(strikethrough_type_t s)
{
    mp_impl->buf.type = s;
}

void import_strikethrough::set_width(strikethrough_width_t s)
{
    mp_impl->buf.width = s;
}

void import_strikethrough::set_text(strikethrough_text_t s)
{
    mp_impl->buf.text = s;
}

void import_strikethrough::commit()
{
    assert(mp_impl->ref);
    *mp_impl->ref = mp_impl->buf;
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
