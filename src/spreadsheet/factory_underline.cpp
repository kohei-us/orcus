/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "factory_underline.hpp"

#include <cassert>

namespace orcus { namespace spreadsheet { namespace detail {

struct import_underline::impl
{
    underline_t* ref = nullptr; // destination to commit to
    underline_t buf;
};

import_underline::import_underline() :
    mp_impl(std::make_unique<impl>())
{
}

import_underline::~import_underline() = default;

void import_underline::reset(underline_t* ref)
{
    assert(ref);
    mp_impl->ref = ref;
    mp_impl->buf.reset();
}

void import_underline::set_style(underline_style_t e)
{
    mp_impl->buf.style = e;
}

void import_underline::set_thickness(underline_thickness_t e)
{
    mp_impl->buf.thickness = e;
}

void import_underline::set_spacing(underline_spacing_t e)
{
    mp_impl->buf.spacing = e;
}

void import_underline::set_count(underline_count_t e)
{
    mp_impl->buf.count = e;
}

void import_underline::set_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    mp_impl->buf.color = color_t(alpha, red, green, blue);
}

void import_underline::commit()
{
    assert(mp_impl->ref);
    *mp_impl->ref = mp_impl->buf;
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
