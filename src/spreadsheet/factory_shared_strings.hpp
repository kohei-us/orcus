/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/spreadsheet/document_types.hpp>

#include <memory>

#include "factory_strikethrough.hpp"

namespace ixion {

class model_context;

}

namespace orcus {

class string_pool;

namespace spreadsheet {

class styles;
class shared_strings;

namespace detail {

class import_shared_strings : public iface::import_shared_strings
{
    orcus::string_pool& m_string_pool;
    ixion::model_context& m_cxt;
    styles& m_styles;
    shared_strings& m_ss_store;

    std::string m_cur_segment_string;
    format_run_t m_cur_format;
    std::unique_ptr<format_runs_t> mp_cur_format_runs;

    detail::import_strikethrough m_strikethrough_import;

public:
    import_shared_strings(
        string_pool& sp, ixion::model_context& cxt, styles& st,
        orcus::spreadsheet::shared_strings& ss_store);
    virtual ~import_shared_strings() override;

    virtual size_t append(std::string_view s) override;
    virtual size_t add(std::string_view s) override;

    virtual void set_segment_font(size_t font_index) override;
    virtual void set_segment_bold(bool b) override;
    virtual void set_segment_italic(bool b) override;
    virtual void set_segment_superscript(bool b) override;
    virtual void set_segment_subscript(bool b) override;
    virtual void set_segment_font_name(std::string_view s) override;
    virtual void set_segment_font_size(double point) override;
    virtual void set_segment_font_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) override;
    virtual iface::import_strikethrough* start_strikethrough();
    virtual void append_segment(std::string_view s) override;
    virtual size_t commit_segments() override;
};

}}} // namespace orcus::spreadsheet::detail

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
