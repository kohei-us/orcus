/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "debug_state_dumper.hpp"
#include "check_dumper.hpp"
#include "document_impl.hpp"
#include "sheet_impl.hpp"
#include "ostream_utils.hpp"

#include <fstream>

namespace fs = boost::filesystem;

namespace orcus { namespace spreadsheet { namespace detail {

doc_debug_state_dumper::doc_debug_state_dumper(const document_impl& doc) : m_doc(doc)
{
}

void doc_debug_state_dumper::dump(const fs::path& outdir) const
{
    dump_properties(outdir);
    dump_styles(outdir);
}

void doc_debug_state_dumper::dump_properties(const fs::path& outdir) const
{
    const fs::path outpath = outdir / "properties.yaml";
    std::ofstream of{outpath.native()};
    if (!of)
        return;

    of << "formula-grammar: " << m_doc.grammar << std::endl;
    of << "origin-date: " << m_doc.origin_date << std::endl;
    of << "output-precision: " << short(m_doc.doc_config.output_precision) << std::endl;
}

void doc_debug_state_dumper::dump_styles(const fs::path& outdir) const
{
    const fs::path outpath = outdir / "styles.yaml";
    std::ofstream of{outpath.native()};
    if (!of)
        return;

    of << std::boolalpha;

    auto to_string = [](std::optional<bool> v) -> std::string
    {
        if (!v)
            return "(unset)";

        return *v ? "true" : "false";
    };

    auto dump_xf = [&of,to_string](std::size_t i, const cell_format_t& xf)
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
           << "    apply-protection: " << xf.apply_protection << std::endl
           << "    wrap-text: " << to_string(xf.wrap_text) << std::endl
           << "    shrink-to-fit: " << to_string(xf.shrink_to_fit) << std::endl;
    };

    auto active_value = [&of](std::string_view name, const auto& v, bool active, int level=2)
    {
        constexpr char q = '"';
        constexpr const char* indent_unit_s = "  ";

        std::string indent = indent_unit_s;
        for (int i = 0; i < level - 1; ++i)
            indent += indent_unit_s;

        of << indent << name << ": ";

        if (active)
        {
            std::ostringstream os;
            os << v;
            std::string s = os.str();
            bool quote = s.find_first_of("#:-") != s.npos;
            if (quote)
                of << q << s << q;
            else
                of << s;
        }
        else
            of << "(unset)";

        of << std::endl;
    };

    auto optional_value = [&of](std::string_view name, const std::optional<auto>& v, int level=2)
    {
        constexpr char q = '"';
        constexpr const char* indent_unit_s = "  ";

        std::string indent = indent_unit_s;
        for (int i = 0; i < level - 1; ++i)
            indent += indent_unit_s;

        of << indent << name << ": ";

        if (v)
        {
            std::ostringstream os;
            os << *v;
            std::string s = os.str();
            bool quote = s.find_first_of("#:-") != s.npos;
            if (quote)
                of << q << s << q;
            else
                of << s;
        }
        else
            of << "(unset)";

        of << std::endl;
    };

    auto dump_border = [&active_value](const border_attrs_t& _attrs, const border_attrs_active_t& _active)
    {
        active_value("style", _attrs.style, _active.style, 3);
        active_value("color", _attrs.border_color, _active.border_color, 3);
        active_value("width", _attrs.border_width, _active.border_width, 3);
    };

    of << "cell-styles:" << std::endl;

    for (std::size_t i = 0; i < m_doc.styles_store.get_cell_styles_count(); ++i)
    {
        const cell_style_t* cs = m_doc.styles_store.get_cell_style(i);
        assert(cs);

        of << "  - id: " << i << std::endl
           << "    name: " << cs->name << std::endl
           << "    display-name: " << cs->display_name << std::endl
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

    for (std::size_t i = 0; i < m_doc.styles_store.get_font_count(); ++i)
    {
        const auto* state = m_doc.styles_store.get_font_state(i);
        assert(state);
        const font_t& font = state->first;
        const font_active_t& active = state->second;

        of << "  - id: " << i << std::endl;
        active_value("name", font.name, active.name, 2);
        active_value("name-asian", font.name_asian, active.name_asian, 2);
        active_value("name-complex", font.name_complex, active.name_complex, 2);
        active_value("size", font.size, active.size);
        active_value("size-asian", font.size_asian, active.size_asian);
        active_value("size-complex", font.size_complex, active.size_complex);
        active_value("bold", font.bold, active.bold);
        active_value("bold-asian", font.bold_asian, active.bold_asian);
        active_value("bold-complex", font.bold_complex, active.bold_complex);
        active_value("italic", font.italic, active.italic);
        active_value("italic-asian", font.italic_asian, active.italic_asian);
        active_value("italic-complex", font.italic_complex, active.italic_complex);
        active_value("underline-style", font.underline_style, active.underline_style, 2);
        active_value("underline-width", font.underline_width, active.underline_width, 2);
        active_value("underline-mode", font.underline_mode, active.underline_mode, 2);
        active_value("underline-type", font.underline_type, active.underline_type, 2);
        active_value("underline-color", font.underline_color, active.underline_color, 2);
        active_value("color", font.color, active.color, 2);
        active_value("strikethrough-style", font.strikethrough_style, active.strikethrough_style, 2);
        active_value("strikethrough-width", font.strikethrough_width, active.strikethrough_width, 2);
        active_value("strikethrough-type", font.strikethrough_type, active.strikethrough_type, 2);
        active_value("strikethrough-text", font.strikethrough_text, active.strikethrough_text, 2);
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
        active_value("fg-color", fill.fg_color, active.fg_color, 2);
        active_value("bg-color", fill.bg_color, active.bg_color, 2);
    }

    of << "borders:" << std::endl;

    for (std::size_t i = 0; i < m_doc.styles_store.get_border_count(); ++i)
    {
        const auto* state = m_doc.styles_store.get_border_state(i);
        assert(state);

        of << "  - id: " << i << std::endl;

        const border_t& border = state->first;
        const border_active_t& active = state->second;

        of << "    top:" << std::endl;
        dump_border(border.top, active.top);
        of << "    bottom:" << std::endl;
        dump_border(border.bottom, active.bottom);
        of << "    left:" << std::endl;
        dump_border(border.left, active.left);
        of << "    right:" << std::endl;
        dump_border(border.right, active.right);
        of << "    diagonal:" << std::endl;
        dump_border(border.diagonal, active.diagonal);
        of << "    diagonal-bl-tr:" << std::endl;
        dump_border(border.diagonal_bl_tr, active.diagonal_bl_tr);
        of << "    diagonal-tl-br:" << std::endl;
        dump_border(border.diagonal_tl_br, active.diagonal_tl_br);
    }

    of << "protections:" << std::endl;

    for (std::size_t i = 0; i < m_doc.styles_store.get_protection_count(); ++i)
    {
        const auto* state = m_doc.styles_store.get_protection_state(i);
        assert(state);

        of << "  - id: " << i << std::endl;
        const protection_t& prot = state->first;
        const protection_active_t& active = state->second;

        active_value("locked", prot.locked, active.locked, 2);
        active_value("hidden", prot.hidden, active.hidden, 2);
        active_value("print-content", prot.print_content, active.print_content, 2);
        active_value("formula-hidden", prot.formula_hidden, active.formula_hidden, 2);
    }

    of << "number-formats:" << std::endl;

    for (std::size_t i = 0; i < m_doc.styles_store.get_number_format_count(); ++i)
    {
        const number_format_t* numfmt = m_doc.styles_store.get_number_format(i);
        assert(numfmt);

        of << "  - id: " << i << std::endl;
        optional_value("identifier", numfmt->identifier, 2);
        optional_value("format-string", numfmt->format_string, 2);
    }
}

sheet_debug_state_dumper::sheet_debug_state_dumper(const sheet_impl& sheet, std::string_view sheet_name) :
    m_sheet(sheet), m_sheet_name(sheet_name) {}

void sheet_debug_state_dumper::dump(const fs::path& outdir) const
{
    dump_cell_values(outdir);
    dump_cell_formats(outdir);
    dump_column_formats(outdir);
    dump_row_formats(outdir);
}

void sheet_debug_state_dumper::dump_cell_values(const fs::path& outdir) const
{
    check_dumper dumper{m_sheet, m_sheet_name};
    fs::path outpath = outdir / "cell-values.txt";
    std::ofstream of{outpath.native()};
    if (of)
        dumper.dump(of);
}

void sheet_debug_state_dumper::dump_cell_formats(const fs::path& outdir) const
{
    fs::path outpath = outdir / "cell-formats.yaml";
    std::ofstream of{outpath.native()};
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

void sheet_debug_state_dumper::dump_column_formats(const fs::path& outdir) const
{
    fs::path outpath = outdir / "column-formats.yaml";
    std::ofstream of{outpath.native()};
    if (!of)
        return;

    auto it_seg = m_sheet.column_formats.begin_segment();
    auto it_seg_end = m_sheet.column_formats.end_segment();

    for (; it_seg != it_seg_end; ++it_seg)
    {
        of << "- columns: " << it_seg->start << '-' << (it_seg->end - 1) << std::endl;
        of << "  xf: " << it_seg->value << std::endl;
    }
}

void sheet_debug_state_dumper::dump_row_formats(const fs::path& outdir) const
{
    fs::path outpath = outdir / "row-formats.yaml";
    std::ofstream of{outpath.native()};
    if (!of)
        return;

    auto it_seg = m_sheet.row_formats.begin_segment();
    auto it_seg_end = m_sheet.row_formats.end_segment();

    for (; it_seg != it_seg_end; ++it_seg)
    {
        of << "- rows: " << it_seg->start << '-' << (it_seg->end - 1) << std::endl;
        of << "  xf: " << it_seg->value << std::endl;
    }
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
