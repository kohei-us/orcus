/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "factory_shared_strings.hpp"

#include <orcus/string_pool.hpp>
#include <ixion/model_context.hpp>

namespace orcus { namespace spreadsheet { namespace detail {

import_shared_strings::import_shared_strings(
    string_pool& sp, ixion::model_context& cxt, styles& st,
    orcus::spreadsheet::import_shared_strings& ss_store) :
    m_string_pool(sp),
    m_cxt(cxt),
    m_styles(st),
    m_ss_store(ss_store)
{
}

import_shared_strings::~import_shared_strings() {}

size_t import_shared_strings::append(std::string_view s)
{
    return m_cxt.append_string(s);
}

size_t import_shared_strings::add(std::string_view s)
{
    return m_cxt.add_string(s);
}

void import_shared_strings::set_segment_font(size_t font_index)
{
    const font_t* font_data = m_styles.get_font(font_index);
    if (!font_data)
        return;

    m_cur_format.bold = font_data->bold;
    m_cur_format.italic = font_data->italic;
    m_cur_format.font = font_data->name; // font names are already interned when set.
    m_cur_format.font_size = font_data->size;
    m_cur_format.color = font_data->color;
}

void import_shared_strings::set_segment_bold(bool b)
{
    m_cur_format.bold = b;
}

void import_shared_strings::set_segment_italic(bool b)
{
    m_cur_format.italic = b;
}

void import_shared_strings::set_segment_font_name(std::string_view s)
{
    m_cur_format.font = m_string_pool.intern(s).first;
}

void import_shared_strings::set_segment_font_size(double point)
{
    m_cur_format.font_size = point;
}

void import_shared_strings::set_segment_font_color(
    color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    m_cur_format.color = color_t(alpha, red, green, blue);
}

void import_shared_strings::append_segment(std::string_view s)
{
    if (s.empty())
        return;

    size_t start_pos = m_cur_segment_string.size();
    m_cur_segment_string += s;

    if (m_cur_format.formatted())
    {
        // This segment is formatted.
        // Record the position and size of the format run.
        m_cur_format.pos = start_pos;
        m_cur_format.size = s.size();

        if (!mp_cur_format_runs)
            mp_cur_format_runs = std::make_unique<format_runs_t>();

        mp_cur_format_runs->push_back(m_cur_format);
        m_cur_format.reset();
    }
}

size_t import_shared_strings::commit_segments()
{
    ixion::string_id_t sindex = m_cxt.append_string(m_cur_segment_string);
    m_cur_segment_string.clear();
    m_ss_store.set_format_runs(sindex, std::move(mp_cur_format_runs));
    mp_cur_format_runs.reset();

    return sindex;
}

}}} // namespace orcus::spreadsheet::detail

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
