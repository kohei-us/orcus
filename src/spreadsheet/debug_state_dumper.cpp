/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "debug_state_dumper.hpp"
#include "check_dumper.hpp"
#include "document_impl.hpp"

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
    fs::path outpath = outdir / "styles.yaml";
    std::ofstream of{outpath};
    if (!of)
        return;

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

        of << "  - id: " << i << std::endl
           << "    font: " << xf->font << std::endl
           << "    fill: " << xf->fill << std::endl
           << "    border: " << xf->border << std::endl
           << "    protection: " << xf->protection << std::endl
           << "    number-format: " << xf->number_format << std::endl
           << "    style-xf: " << xf->style_xf << std::endl;

        // TODO: dump more
    }

    of << "cell-formats:" << std::endl;

    for (std::size_t i = 0; i < m_doc.styles_store.get_cell_formats_count(); ++i)
    {
        const cell_format_t* xf = m_doc.styles_store.get_cell_format(i);
        assert(xf);

        of << "  - id: " << i << std::endl
           << "    font: " << xf->font << std::endl
           << "    fill: " << xf->fill << std::endl
           << "    border: " << xf->border << std::endl
           << "    protection: " << xf->protection << std::endl
           << "    number-format: " << xf->number_format << std::endl
           << "    style-xf: " << xf->style_xf << std::endl;

        // TODO: dump more
    }

    of << "fonts:" << std::endl;

    for (std::size_t i = 0; i < m_doc.styles_store.get_font_count(); ++i)
    {
        const font_t* font = m_doc.styles_store.get_font(i);
        assert(font);

        of << "  - id: " << i << std::endl
           << "    name: " << font->name << std::endl
           << "    size: " << font->size << std::endl
           << "    bold: " << font->bold << std::endl
           << "    italic: " << font->italic << std::endl
           << "    color: \"" << font->color << '"' << std::endl
           << "    underline-color: \"" << font->underline_color << '"' << std::endl;

        // TODO: dump more
    }

    of << "fills:" << std::endl;

    for (std::size_t i = 0; i < m_doc.styles_store.get_fill_count(); ++i)
    {
        const fill_t* fill = m_doc.styles_store.get_fill(i);
        assert(fill);

        of << "  - id: " << i << std::endl
           << "    pattern: " << int(fill->pattern_type) << std::endl
           << "    fg-color: \"" << fill->fg_color << '"' << std::endl
           << "    bg-color: \"" << fill->bg_color << '"' << std::endl;

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
            of << "  - rows: " << it_seg->start << '-' << it_seg->end << std::endl;
            of << "    xf: " << it_seg->value << std::endl;
        }
    }
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
