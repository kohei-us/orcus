/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include <orcus/orcus_ods.hpp>
#include <orcus/stream.hpp>
#include <orcus/parser_global.hpp>
#include <orcus/spreadsheet/factory.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/sheet.hpp>
#include <orcus/spreadsheet/shared_strings.hpp>
#include <orcus/spreadsheet/styles.hpp>

#include <cstdlib>
#include <cassert>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#include <boost/filesystem.hpp>
#include <mdds/flat_segment_tree.hpp>

using namespace orcus;
using namespace orcus::spreadsheet;

namespace ss = orcus::spreadsheet;

namespace fs = boost::filesystem;

typedef mdds::flat_segment_tree<std::size_t, bool> bool_segment_type;

namespace {

std::unique_ptr<ss::document> load_doc(const fs::path& filepath)
{
    spreadsheet::range_size_t ss{1048576, 16384};
    std::unique_ptr<ss::document> doc = std::make_unique<ss::document>(ss);
    import_factory factory(*doc);
    orcus_ods app(&factory);
    app.read_file(filepath.string());

    return doc;
}

std::vector<const char*> dirs = {
    SRCDIR"/test/ods/raw-values-1/",
    SRCDIR"/test/ods/formula-1/",
    SRCDIR"/test/ods/formula-2/",
    SRCDIR"/test/ods/named-range/",
    SRCDIR"/test/ods/named-expression/",
    SRCDIR"/test/ods/named-expression-sheet-local/",
};

void test_ods_import_cell_values()
{
    for (const char* dir : dirs)
    {
        fs::path filepath{dir};
        filepath /= "input.ods";
        std::cout << filepath << std::endl;

        auto doc = load_doc(filepath);
        assert(doc);
        doc->recalc_formula_cells();

        // Dump the content of the model.
        std::ostringstream os;
        doc->dump_check(os);
        std::string check = os.str();

        // Check that against known control.
        filepath = dir;
        filepath /= "check.txt";
        file_content control{filepath.string()};

        assert(!check.empty());
        assert(!control.empty());

        assert(trim(check) == trim(control.str()));
    }
}

void test_ods_import_column_widths_row_heights()
{
    fs::path filepath{SRCDIR"/test/ods/column-width-row-height/input.ods"};
    auto doc = load_doc(filepath);
    assert(doc);

    assert(doc->get_sheet_count() > 0);
    spreadsheet::sheet* sh = doc->get_sheet(0);
    assert(sh);

    // Column widths are in twips.
    col_width_t cw = sh->get_col_width(1, nullptr, nullptr);
    assert(cw == 1440); // 1 in
    cw = sh->get_col_width(2, nullptr, nullptr);
    assert(cw == 2160); // 1.5 in
    cw = sh->get_col_width(3, nullptr, nullptr);
    assert(cw == 2592); // 1.8 in

    // Row heights are in twips too.
    row_height_t rh = sh->get_row_height(3, nullptr, nullptr);
    assert(rh == 720); // 0.5 in
    rh = sh->get_row_height(4, nullptr, nullptr);
    assert(rh == 1440); // 1 in
    rh = sh->get_row_height(5, nullptr, nullptr);
    assert(rh == 2160); // 1.5 in
    rh = sh->get_row_height(6, nullptr, nullptr);
    assert(rh == 2592); // 1.8 in
}

void test_ods_import_formatted_text()
{
    fs::path filepath{SRCDIR"/test/ods/formatted-text/bold-and-italic.ods"};
    auto doc = load_doc(filepath);
    assert(doc);

    assert(doc->get_sheet_count() > 0);
    spreadsheet::sheet* sh = doc->get_sheet(0);
    assert(sh);

    const shared_strings& ss = doc->get_shared_strings();

    const styles& styles = doc->get_styles();

    // A1 is unformatted
    size_t str_id = sh->get_string_identifier(0,0);
    const std::string* str = ss.get_string(str_id);
    assert(str && *str == "Normal Text");
    std::size_t xfid = sh->get_cell_format(0, 0);
    const ss::cell_format_t* xf = styles.get_cell_format(xfid);
    assert(xf);
    const ss::cell_style_t* xstyle = styles.get_cell_style_by_xf(xf->style_xf);
    assert(xstyle);
    assert(xstyle->name == "Default");
    const format_runs_t* fmt = ss.get_format_runs(str_id);
    assert(!fmt); // The string should be unformatted.

    // A2 is all bold via cell format.
    str_id = sh->get_string_identifier(1,0);
    str = ss.get_string(str_id);
    assert(str && *str == "Bold Text");
    xfid = sh->get_cell_format(1,0);
    xf = styles.get_cell_format(xfid);
    assert(xf);
    const font_t* font_data = styles.get_font(xf->font);
    assert(font_data && font_data->bold && !font_data->italic);
    fmt = ss.get_format_runs(str_id);
    assert(!fmt); // This string should be unformatted.

    // A3 is all italic.
    str_id = sh->get_string_identifier(2,0);
    str = ss.get_string(str_id);
    assert(str && *str == "Italic Text");
    xfid = sh->get_cell_format(2,0);
    xf = styles.get_cell_format(xfid);
    assert(xf);
    font_data = styles.get_font(xf->font);
    assert(font_data && !font_data->bold && font_data->italic);
    fmt = ss.get_format_runs(str_id);
    assert(!fmt); // This string should be unformatted.

    // A4 is all bold and italic.
    str_id = sh->get_string_identifier(3,0);
    str = ss.get_string(str_id);
    assert(str && *str == "Bold and Italic Text");
    xfid = sh->get_cell_format(3,0);
    xf = styles.get_cell_format(xfid);
    assert(xf);
    font_data = styles.get_font(xf->font);
    assert(font_data && font_data->bold && font_data->italic);
    fmt = ss.get_format_runs(str_id);
    assert(!fmt); // This string should be unformatted.

    // A5 has mixed format runs.
    str_id = sh->get_string_identifier(4,0);
    str = ss.get_string(str_id);
    assert(str && *str == "Bold and Italic mixed");
    xfid = sh->get_cell_format(4,0);
    xf = styles.get_cell_format(xfid);
    assert(xf);
    font_data = styles.get_font(xf->font);
    fmt = ss.get_format_runs(str_id);
    assert(fmt); // This string should be formatted.

    {
        // Check the bold format segment.
        bool_segment_type bold_runs(0, str->size(), font_data->bold);
        for (size_t i = 0, n = fmt->size(); i < n; ++i)
        {
            format_run run = fmt->at(i);
            bold_runs.insert_back(run.pos, run.pos+run.size, run.bold);
        }

        bold_runs.build_tree();
        bool is_bold = false;
        size_t start_pos, end_pos;

        // The first four letters 'Bold' should be bold.
        bool good = bold_runs.search_tree(0, is_bold, &start_pos, &end_pos).second;
        assert(good);
        assert(is_bold);
        assert(start_pos == 0);
        assert(end_pos == 4);

        // The rest should be non-bold.
        good = bold_runs.search_tree(4, is_bold, &start_pos, &end_pos).second;
        assert(good);
        assert(!is_bold);
        assert(start_pos == 4);
        assert(end_pos == str->size());
    }

    {
        // Check the italic format segment.
        bool_segment_type italic_runs(0, str->size(), font_data->italic);
        for (size_t i = 0, n = fmt->size(); i < n; ++i)
        {
            format_run run = fmt->at(i);
            italic_runs.insert_back(run.pos, run.pos+run.size, run.italic);
        }

        italic_runs.build_tree();
        bool it_italic = false;
        size_t start_pos, end_pos;

        // The first 9 letters 'Bold and ' should not be italic.
        bool good = italic_runs.search_tree(0, it_italic, &start_pos, &end_pos).second;
        assert(good);
        assert(!it_italic);
        assert(start_pos == 0);
        assert(end_pos == 9);

        // The next 6 letters 'Italic' should be italic.
        good = italic_runs.search_tree(9, it_italic, &start_pos, &end_pos).second;
        assert(good);
        assert(it_italic);
        assert(start_pos == 9);
        assert(end_pos == 15);

        // The rest should be non-italic.
        good = italic_runs.search_tree(15, it_italic, &start_pos, &end_pos).second;
        assert(good);
        assert(!it_italic);
        assert(start_pos == 15);
        assert(end_pos == str->size());
    }
}

void test_ods_import_number_formats()
{
    fs::path filepath{SRCDIR"/test/ods/number-format/basic-set.ods"};

    auto doc = load_doc(filepath);
    assert(doc);

    assert(doc->get_sheet_count() > 0);
    spreadsheet::sheet* sh = doc->get_sheet(0);
    assert(sh);

    const styles& styles = doc->get_styles();

    struct check
    {
        row_t row;
        col_t col;
        std::string_view format;
    };

    const check checks[] = {
        { 1, 1, "#.000000" }, // B2
        { 2, 1, "[>=0][$₹]#,##0.00;[RED]-[$₹]#,##0.00" }, // B3
        { 3, 1, "0.00%" }, // B4
        { 4, 1, "#.00E+00" }, // B5
        { 5, 1, "BOOLEAN" }, // B6
        { 6, 1, "?/11" }, // B7
        { 7, 1, "MM/DD/YY" }, // B8
        { 8, 1, "HH:MM:SS AM/PM" }, // B9
        { 9, 1, "[>=0]0.00;[RED]-0.00" }, // B9
        { 10, 1, "#,##0.00" }, // B10
        { 11, 1, "Head @ Tail" }, // B11
    };

    for (const auto& c : checks)
    {
        std::size_t xfid = sh->get_cell_format(c.row, c.col);
        const cell_format_t* xf = styles.get_cell_format(xfid);
        if (!xf)
        {
            std::cerr << "No cell format entry for (row=" << c.row << "; col=" << c.col << ")" << std::endl;
            assert(false);
        }

        const number_format_t* numfmt = styles.get_number_format(xf->number_format);
        if (!numfmt)
        {
            std::cerr << "No number-format entry for (row=" << c.row << "; col=" << c.col << ")" << std::endl;
            assert(false);
        }

        if (numfmt->format_string != c.format)
        {
            std::cerr << "Number format strings differ: (expected='" << c.format << "'; actual='"
                << numfmt->format_string << "')" << std::endl;

            assert(false);
        }
    }
}

void test_ods_import_cell_properties()
{
    fs::path filepath{SRCDIR"/test/ods/cell-properties/wrap-and-shrink.ods"};

    auto doc = load_doc(filepath);
    assert(doc);

    const ss::styles& styles = doc->get_styles();
    const ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

    std::size_t xfid = sh->get_cell_format(0, 1); // B1
    const ss::cell_format_t* xf = styles.get_cell_format(xfid);
    assert(xf);
    assert(!xf->wrap_text);
    assert(!xf->shrink_to_fit);

    xfid = sh->get_cell_format(1, 1); // B2
    xf = styles.get_cell_format(xfid);
    assert(xf);
    assert(xf->wrap_text);
    assert(*xf->wrap_text);
    assert(!xf->shrink_to_fit);

    xfid = sh->get_cell_format(2, 1); // B3
    xf = styles.get_cell_format(xfid);
    assert(xf);
    assert(!xf->wrap_text);
    assert(xf->shrink_to_fit);
    assert(*xf->shrink_to_fit);
}

void test_ods_import_styles_direct_format()
{
    fs::path filepath{SRCDIR"/test/ods/styles/direct-format.ods"};

    auto doc = load_doc(filepath);
    assert(doc);

    const ss::styles& styles = doc->get_styles();
    const ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

    // B2 - horizontally center, bold and underlined
    std::size_t xfid = sh->get_cell_format(1, 1);
    const ss::cell_format_t* xf = styles.get_cell_format(xfid);
    assert(xf);
    assert(xf->hor_align == ss::hor_alignment_t::center);

    const auto* font = styles.get_font_state(xf->font);
    assert(font);
    assert(font->first.bold);
    assert(font->second.bold);
    assert(font->first.underline_style == ss::underline_t::single_line);
    assert(font->second.underline_style);

    // B4 - yellow background and right-aligned
    xfid = sh->get_cell_format(3, 1);
    xf = styles.get_cell_format(xfid);
    assert(xf);
    assert(xf->hor_align == ss::hor_alignment_t::right);

    const auto* fill = styles.get_fill_state(xf->fill);
    assert(fill);
    assert(fill->first.pattern_type == ss::fill_pattern_t::solid);
    assert(fill->second.pattern_type);
    assert(fill->first.fg_color == ss::color_t(0xFF, 0xFF, 0xFF, 0x00));
    assert(fill->second.fg_color);

    // D4 - named style "Good" applied with no direct formatting on top
    xfid = sh->get_cell_format(3, 3);
    xf = styles.get_cell_format(xfid);
    assert(xf);

    const ss::cell_style_t* xstyle = styles.get_cell_style_by_xf(xf->style_xf);
    assert(xstyle);
    assert(xstyle->name == "Good");

    // D6 - named style "Good" plus wrap-text, center and middle aligned and bold text
    xfid = sh->get_cell_format(5, 3);
    xf = styles.get_cell_format(xfid);
    assert(xf);
    assert(xf->hor_align == ss::hor_alignment_t::center);
    assert(xf->ver_align == ss::ver_alignment_t::middle);
    assert(xf->wrap_text);
    assert(*xf->wrap_text);

    font = styles.get_font_state(xf->font);
    assert(font);
    assert(font->first.bold);
    assert(font->second.bold);

    xstyle = styles.get_cell_style_by_xf(xf->style_xf);
    assert(xstyle);
    assert(xstyle->name == "Good");
}

void test_ods_import_styles_column_styles()
{
    fs::path filepath{SRCDIR"/test/ods/styles/column-styles.ods"};

    auto doc = load_doc(filepath);
    assert(doc);

    const ss::styles& styles = doc->get_styles();
    const ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

    // A1 should have the Default style applied.
    std::size_t xfid = sh->get_cell_format(0, 0);
    const ss::cell_format_t* xf = styles.get_cell_format(xfid);
    assert(xf);

    const ss::cell_style_t* xstyle = styles.get_cell_style_by_xf(xf->style_xf);
    assert(xstyle);
    assert(xstyle->name == "Default");

    // This Default style has some custom properties.
    xf = styles.get_cell_style_format(xf->style_xf);
    assert(xf);

    // Default style has a solid fill with light green color.
    const auto* fill = styles.get_fill_state(xf->fill);
    assert(fill);
    assert(fill->first.pattern_type == ss::fill_pattern_t::solid);
    assert(fill->second.pattern_type);
    assert(fill->first.fg_color == ss::color_t(0xFF, 0xF6, 0xF9, 0xD4));
    assert(fill->second.fg_color);
    assert(!fill->second.bg_color);

    // Default style has a 14pt DejaVu Sans font with normal weight
    const auto* font = styles.get_font_state(xf->font);
    assert(font);
    assert(font->first.name == "DejaVu Sans");
    assert(font->second.name);
    assert(font->first.size == 14.0);
    assert(font->second.size);
    assert(!font->first.bold);
    assert(font->second.bold);

    assert(xf->hor_align == ss::hor_alignment_t::center);

    // Columns B, E, G and rest all should have the "Default" style applied.
    std::size_t xfid_default = xfid;
    for (ss::col_t col : {1, 4, 6, 7, 8})
    {
        std::cout << "column " << col << std::endl;
        xfid = sh->get_cell_format(0, col); // top cell
        assert(xfid == xfid_default);
        xfid = sh->get_cell_format(doc->get_sheet_size().rows-1, col); // bottom cell
        assert(xfid == xfid_default);
    }

    // Column C should have "Gray With Lime" style applied
    xfid = sh->get_cell_format(0, 2);
    xf = styles.get_cell_format(xfid);
    assert(xf);
    xstyle = styles.get_cell_style_by_xf(xf->style_xf);
    assert(xstyle);
    assert(xstyle->name == "Gray With Lime" || xstyle->display_name == "Gray With Lime");

    // Its parent style should be "Default".
    xf = styles.get_cell_style_format(xf->style_xf);
    assert(xf);
    xstyle = styles.get_cell_style_by_xf(xf->style_xf);
    assert(xstyle);
    assert(xstyle->name == "Default");

    // solid gray background
    fill = styles.get_fill_state(xf->fill);
    assert(fill);
    assert(fill->first.pattern_type == ss::fill_pattern_t::solid);
    assert(fill->second.pattern_type);
    assert(fill->first.fg_color == ss::color_t(0xFF, 0xCC, 0xCC, 0xCC));
    assert(fill->second.fg_color);
    assert(!fill->second.bg_color);

    // bold, 16pt font, name not set
    font = styles.get_font_state(xf->font);
    assert(font);
    assert(!font->second.name);
    assert(font->first.size == 16.0);
    assert(font->second.size);
    assert(font->first.bold);
    assert(font->second.bold);

    // left and right borders are solid light green
    const auto* border = styles.get_border_state(xf->border);
    assert(border);

    assert(border->first.left.style == ss::border_style_t::solid);
    assert(border->second.left.style);
    assert(border->first.left.border_width == length_t(length_unit_t::point, 2.01));
    assert(border->second.left.border_width);
    assert(border->first.left.border_color == ss::color_t(0xFF, 0x81, 0xD4, 0x1A));
    assert(border->second.left.border_color);

    assert(border->first.right.style == ss::border_style_t::solid);
    assert(border->second.right.style);
    assert(border->first.right.border_width == length_t(length_unit_t::point, 2.01));
    assert(border->second.right.border_width);
    assert(border->first.right.border_color == ss::color_t(0xFF, 0x81, 0xD4, 0x1A));
    assert(border->second.right.border_color);

    // Column D should have "Emphasis" style applied
    xfid = sh->get_cell_format(0, 3);
    xf = styles.get_cell_format(xfid);
    assert(xf);
    xstyle = styles.get_cell_style_by_xf(xf->style_xf);
    assert(xstyle);
    assert(xstyle->name == "Emphasis" || xstyle->display_name == "Emphasis");

    // Its parent style should be "Default".
    xf = styles.get_cell_style_format(xf->style_xf);
    assert(xf);
    xstyle = styles.get_cell_style_by_xf(xf->style_xf);
    assert(xstyle);
    assert(xstyle->name == "Default");

    // solid pink background
    fill = styles.get_fill_state(xf->fill);
    assert(fill);
    assert(fill->first.pattern_type == ss::fill_pattern_t::solid);
    assert(fill->second.pattern_type);
    assert(fill->first.fg_color == ss::color_t(0xFF, 0xFF, 0xd7, 0xd7));
    assert(fill->second.fg_color);
    assert(!fill->second.bg_color);

    // font name 'Rasa Light', 18pt, underlined (solid double), red, not bold
    font = styles.get_font_state(xf->font);
    assert(font);
    assert(font->first.name == "Rasa Light");
    assert(font->second.name);
    assert(font->first.size == 18.0);
    assert(font->second.size);
    assert(!font->first.bold); // in the file, it is given as a font weight of 250
    assert(font->second.bold);
    assert(font->first.color == ss::color_t(0xFF, 0xFF, 0x00, 0x00));
    assert(font->second.color);
    // double underline is stored as single-line double-type?
    assert(font->first.underline_style == ss::underline_t::single_line);
    assert(font->second.underline_style);
    assert(font->first.underline_type == ss::underline_type_t::double_type);
    assert(font->second.underline_type);
    assert(font->first.underline_color == font->first.color); // same as font color
    assert(font->second.underline_color);
}

} // anonymous namespace

int main()
{
    test_ods_import_cell_values();
    test_ods_import_column_widths_row_heights();
    test_ods_import_formatted_text();
    test_ods_import_number_formats();
    test_ods_import_cell_properties();
    test_ods_import_styles_direct_format();
    test_ods_import_styles_column_styles();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
