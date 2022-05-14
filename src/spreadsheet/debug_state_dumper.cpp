/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "debug_state_dumper.hpp"
#include "check_dumper.hpp"
#include "document_impl.hpp"
#include "ostream_utils.hpp"

namespace fs = boost::filesystem;

namespace orcus { namespace spreadsheet { namespace detail {

doc_debug_state_dumper::doc_debug_state_dumper(const document_impl& doc) : m_doc(doc)
{
}

void doc_debug_state_dumper::dump(const fs::path& outdir) const
{
    dump_styles(outdir);
}

void doc_debug_state_dumper::dump_styles(const fs::path& outdir) const
{
    const fs::path outpath = outdir / "styles.yaml";
    std::ofstream of{outpath};
    if (!of)
        return;

    ::orcus::detail::ostream_format_guard guard{of};

    of << std::boolalpha;

    auto dump_xf = [&of](std::size_t i, const cell_format_t& xf)
    {
        of << "  - id: " << i << std::endl
           << "    font: " << xf.font << std::endl
           << "    fill: " << xf.fill << std::endl
           << "    border: " << xf.border << std::endl
           << "    protection: " << xf.protection << std::endl
           << "    number-format: " << xf.number_format << std::endl
           << "    style-xf: " << xf.style_xf << std::endl
           << "    horizontal-alignment: " << xf.hor_align << std::endl
           << "    vertical-alignment: " << xf.ver_align << std::endl
           << "    apply-number-format: " << xf.apply_num_format << std::endl
           << "    apply-font: " << xf.apply_font << std::endl
           << "    apply-fill: " << xf.apply_fill << std::endl
           << "    apply-border: " << xf.apply_border << std::endl
           << "    apply-alignment: " << xf.apply_alignment << std::endl
           << "    apply-protection: " << xf.apply_protection << std::endl;
    };

    of << "cell-styles:" << std::endl;

    for (std::size_t i = 0; i < m_doc.styles_store.get_cell_styles_count(); ++i)
    {
        const cell_style_t* cs = m_doc.styles_store.get_cell_style(i);
        assert(cs);

        of << "  - id: " << i << std::endl
           << "    name: " << cs->name << std::endl
           << "    parent: " << cs->parent_name << std::endl
           << "    xf: " << cs->xf << std::endl
           << "    builtin: " << cs->builtin << std::endl;
    }

    of << "cell-style-formats:" << std::endl;

    for (std::size_t i = 0; i < m_doc.styles_store.get_cell_style_formats_count(); ++i)
    {
        const cell_format_t* xf = m_doc.styles_store.get_cell_style_format(i);
        assert(xf);
        dump_xf(i, *xf);
    }

    of << "cell-formats:" << std::endl;

    for (std::size_t i = 0; i < m_doc.styles_store.get_cell_formats_count(); ++i)
    {
        const cell_format_t* xf = m_doc.styles_store.get_cell_format(i);
        assert(xf);
        dump_xf(i, *xf);
    }

    of << "fonts:" << std::endl;

    auto active_value = [&of](std::string_view name, const auto& v, bool active, bool quote=false)
    {
        constexpr char q = '"';

        of << "    " << name << ": ";

        if (active)
        {
            if (quote)
                of << q << v << q;
            else
                of << v;
        }
        else
            of << "(unset)";

        of << std::endl;
    };

    for (std::size_t i = 0; i < m_doc.styles_store.get_font_count(); ++i)
    {
        const auto* state = m_doc.styles_store.get_font_state(i);
        assert(state);
        const font_t& font = state->first;
        const font_active_t& active = state->second;

        of << "  - id: " << i << std::endl;
        active_value("name", font.name, active.name, true);
        active_value("size", font.size, active.size);
        active_value("bold", font.bold, active.bold);
        active_value("italic", font.italic, active.italic);
        active_value("color", font.color, active.color, true);
        active_value("underline-color", font.underline_color, active.underline_color, true);

        // TODO: dump more
    }

    of << "fills:" << std::endl;

    for (std::size_t i = 0; i < m_doc.styles_store.get_fill_count(); ++i)
    {
        const auto* state = m_doc.styles_store.get_fill_state(i);
        assert(state);
        const fill_t& fill = state->first;
        const fill_active_t& active = state->second;

        of << "  - id: " << i << std::endl;
        active_value("pattern", fill.pattern_type, active.pattern_type);
        active_value("fg-color", fill.fg_color, active.fg_color, true);
        active_value("bg-color", fill.bg_color, active.bg_color, true);

        // TODO: dump more
    }

    // TODO: dump more styles.
}

sheet_debug_state_dumper::sheet_debug_state_dumper(const sheet_impl& sheet, std::string_view sheet_name) :
    m_sheet(sheet), m_sheet_name(sheet_name) {}

void sheet_debug_state_dumper::dump(const fs::path& outdir) const
{
    dump_cell_values(outdir);
    dump_cell_formats(outdir);
}

void sheet_debug_state_dumper::dump_cell_values(const fs::path& outdir) const
{
    check_dumper dumper{m_sheet, m_sheet_name};
    fs::path outpath = outdir / "cell-values.txt";
    std::ofstream of{outpath};
    if (of)
        dumper.dump(of);
}

void sheet_debug_state_dumper::dump_cell_formats(const fs::path& outdir) const
{
    fs::path outpath = outdir / "cell-formats.yaml";
    std::ofstream of{outpath};
    if (!of)
        return;

    std::vector<col_t> columns;
    for (const auto& node : m_sheet.cell_formats)
        columns.push_back(node.first);

    std::sort(columns.begin(), columns.end());

    for (const col_t col : columns)
    {
        of << "column: " << col << std::endl;

        auto it = m_sheet.cell_formats.find(col);
        assert(it != m_sheet.cell_formats.end());
        const segment_row_index_type& rows = *it->second;

        auto it_seg = rows.begin_segment();
        auto it_seg_end = rows.end_segment();

        for (; it_seg != it_seg_end; ++it_seg)
        {
            // NB: end position is not inclusive.
            of << "  - rows: " << it_seg->start << '-' << (it_seg->end - 1) << std::endl;
            of << "    xf: " << it_seg->value << std::endl;
        }
    }
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
