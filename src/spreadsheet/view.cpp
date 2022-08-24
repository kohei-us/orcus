/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/spreadsheet/view.hpp"
#include "orcus/spreadsheet/document.hpp"

#include <cassert>
#include <iostream>

namespace orcus { namespace spreadsheet {

struct view::impl
{
    document& m_doc;

    std::vector<std::unique_ptr<sheet_view>> m_sheet_views;
    sheet_t m_active_sheet;

    impl(document& doc) : m_doc(doc), m_active_sheet(0) {}
};

view::view(document& doc) : mp_impl(std::make_unique<impl>(doc)) {}
view::~view() {}

sheet_view* view::get_or_create_sheet_view(sheet_t sheet)
{
    if (sheet < 0)
        return nullptr;

    sheet_t n = mp_impl->m_doc.get_sheet_count();
    if (sheet >= n)
        return nullptr;

    // Make sure the container is large enough for the requested sheet view index.
    n = mp_impl->m_sheet_views.size();
    if (sheet >= n)
        mp_impl->m_sheet_views.resize(sheet+1);

    if (!mp_impl->m_sheet_views[sheet])
        mp_impl->m_sheet_views[sheet] = std::make_unique<sheet_view>(*this);

    return mp_impl->m_sheet_views[sheet].get();
}

const sheet_view* view::get_sheet_view(sheet_t sheet) const
{
    if (sheet < 0)
        return nullptr;

    sheet_t n = mp_impl->m_doc.get_sheet_count();
    if (sheet >= n)
        return nullptr;

    n = mp_impl->m_sheet_views.size();
    if (sheet >= n)
        return nullptr;

    assert(mp_impl->m_sheet_views[sheet]);
    return mp_impl->m_sheet_views[sheet].get();
}

void view::set_active_sheet(sheet_t sheet)
{
    mp_impl->m_active_sheet = sheet;
}

sheet_t view::get_active_sheet() const
{
    return mp_impl->m_active_sheet;
}

namespace {

/**
 * Stores all data for a single sheet pane.
 */
struct sheet_pane_data
{
    range_t m_selection;

    sheet_pane_data()
    {
        m_selection.first.row = -1;
        m_selection.first.column = -1;
        m_selection.last = m_selection.first;
    }
};

size_t to_pane_index(sheet_pane_t pos)
{
    switch (pos)
    {
        case sheet_pane_t::top_left:
            return 0;
        case sheet_pane_t::top_right:
            return 1;
        case sheet_pane_t::bottom_left:
            return 2;
        case sheet_pane_t::bottom_right:
            return 3;
        case sheet_pane_t::unspecified:
        default:
            throw std::runtime_error("invalid sheet pane.");
    }
}

} // anonymous namespace

struct sheet_view::impl
{
    view& m_doc_view;
    sheet_pane_data m_panes[4];
    sheet_pane_t m_active_pane;
    split_pane_t m_split_pane;
    frozen_pane_t m_frozen_pane;

    sheet_pane_data& get_pane(sheet_pane_t pos)
    {
        return m_panes[to_pane_index(pos)];
    }

    const sheet_pane_data& get_pane(sheet_pane_t pos) const
    {
        return m_panes[to_pane_index(pos)];
    }

    impl(view& doc_view) : m_doc_view(doc_view), m_active_pane(sheet_pane_t::top_left)
    {
        m_split_pane.hor_split = 0.0;
        m_split_pane.ver_split = 0.0;
        m_split_pane.top_left_cell.row = -1;
        m_split_pane.top_left_cell.column = -1;
        m_frozen_pane.visible_columns = 0;
        m_frozen_pane.visible_rows = 0;
        m_frozen_pane.top_left_cell.row = -1;
        m_frozen_pane.top_left_cell.column = -1;
    }
};

sheet_view::sheet_view(view& doc_view) : mp_impl(std::make_unique<impl>(doc_view)) {}
sheet_view::~sheet_view() {}

const range_t& sheet_view::get_selection(sheet_pane_t pos) const
{
    const sheet_pane_data& pd = mp_impl->get_pane(pos);
    return pd.m_selection;
}

void sheet_view::set_selection(sheet_pane_t pos, const range_t& range)
{
    sheet_pane_data& pd = mp_impl->get_pane(pos);
    pd.m_selection = range;
}

void sheet_view::set_active_pane(sheet_pane_t pos)
{
    mp_impl->m_active_pane = pos;
}

sheet_pane_t sheet_view::get_active_pane() const
{
    return mp_impl->m_active_pane;
}

void sheet_view::set_split_pane(
    double hor_split, double ver_split, const address_t& top_left_cell)
{
    mp_impl->m_split_pane.hor_split = hor_split;
    mp_impl->m_split_pane.ver_split = ver_split;
    mp_impl->m_split_pane.top_left_cell = top_left_cell;
}

const split_pane_t& sheet_view::get_split_pane() const
{
    return mp_impl->m_split_pane;
}

void sheet_view::set_frozen_pane(col_t visible_cols, row_t visible_rows, const address_t& top_left_cell)
{
    mp_impl->m_frozen_pane.visible_columns = visible_cols;
    mp_impl->m_frozen_pane.visible_rows = visible_rows;
    mp_impl->m_frozen_pane.top_left_cell = top_left_cell;
}

const frozen_pane_t& sheet_view::get_frozen_pane() const
{
    return mp_impl->m_frozen_pane;
}

view& sheet_view::get_document_view()
{
    return mp_impl->m_doc_view;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
