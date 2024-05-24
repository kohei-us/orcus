/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_test_global.hpp"
#include "filesystem_env.hpp"

#include <orcus/orcus_gnumeric.hpp>
#include <orcus/format_detection.hpp>
#include <orcus/stream.hpp>
#include <orcus/spreadsheet/factory.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/sheet.hpp>
#include <orcus/spreadsheet/auto_filter.hpp>
#include <orcus/spreadsheet/shared_strings.hpp>
#include <orcus/spreadsheet/styles.hpp>
#include <orcus/types.hpp>

#include <ixion/address.hpp>
#include <ixion/model_context.hpp>
#include <iostream>
#include <sstream>

using namespace orcus;
namespace ss = orcus::spreadsheet;

namespace {

std::vector<fs::path> dirs = {
    SRCDIR"/test/gnumeric/raw-values-1/",
    SRCDIR"/test/gnumeric/cell-value-types/",
    SRCDIR"/test/gnumeric/formula-cells/",
    SRCDIR"/test/gnumeric/named-expression/",
    SRCDIR"/test/gnumeric/named-expression-sheet-local/",
};

std::unique_ptr<ss::document> load_doc(const fs::path& filepath)
{
    ss::range_size_t ss{1048576, 16384};
    auto doc = std::make_unique<ss::document>(ss);
    ss::import_factory factory(*doc);
    orcus_gnumeric app(&factory);
    app.read_file(filepath.string());

    // Gnumeric doc doesn't cache formula results.
    doc->recalc_formula_cells();

    return doc;
}

void test_gnumeric_detection()
{
    ORCUS_TEST_FUNC_SCOPE;

    for (const auto& dir : dirs)
    {
        fs::path filepath = dir / "input.gnumeric";
        file_content fc(filepath.string());
        assert(!fc.empty());

        format_t detected = detect(fc.str());
        assert(detected == format_t::gnumeric);
    }
}

void test_gnumeric_create_filter()
{
    ORCUS_TEST_FUNC_SCOPE;

    ss::range_size_t ssize{1048576, 16384};
    std::unique_ptr<ss::document> doc = std::make_unique<ss::document>(ssize);
    ss::import_factory factory(*doc);

    auto f = create_filter(format_t::gnumeric, &factory);
    assert(f);
    assert(f->get_name() == "gnumeric");
}

void test_gnumeric_import()
{
    ORCUS_TEST_FUNC_SCOPE;

    for (const auto& dir : dirs)
    {
        std::cout << "checking " << dir << "..." << std::endl;

        // Read the input.gnumeric document.
        fs::path filepath = dir / "input.gnumeric";
        auto doc = load_doc(filepath);

        // Dump the content of the model.
        std::ostringstream os;
        doc->dump_check(os);
        std::string check = os.str();

        // Check that against known control.
        filepath = dir / "check.txt";
        file_content control(filepath.string());

        assert(!check.empty());
        assert(!control.empty());

        test::verify_content(__FILE__, __LINE__, control.str(), check);
    }
}

void test_gnumeric_column_widths_row_heights()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath = SRCDIR"/test/gnumeric/column-width-row-height/input.gnumeric";
    auto doc = load_doc(filepath);

    assert(doc->get_sheet_count() == 1);
    const ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

    {
        // column, column width (twips), column span
        const std::tuple<ss::col_t, ss::col_width_t, ss::col_t> expected[] = {
            { 0, 99 * 20, 1 },
            { 1, 57 * 20, 1 },
            { 2, 84 * 20, 1 },
            { 3, 111 * 20, 1 },
            { 4, ss::get_default_column_width(), 1 },
            { 5, 69 * 20, 3 },
            { 10, 120 * 20, 2 },
        };

        for (const auto& [col, cw_expected, span_expected] : expected)
        {
            ss::col_t col_start, col_end;
            ss::col_width_t cw = sh->get_col_width(col, &col_start, &col_end);
            ss::col_t span = col_end - col_start;

            std::cout << "column: " << col << "; expected=" << cw_expected << "; actual=" << cw
                << "; span-expected=" << span_expected << "; span-actual=" << span << std::endl;

            assert(cw == cw_expected);
            assert(span == span_expected);
        }
    }

    {
        // row, row height (twips), row span
        const std::tuple<ss::row_t, ss::row_height_t, ss::row_t> expected[] = {
            { 0, 18 * 20, 3 },
            { 3, 30 * 20, 1 },
            { 4, 42 * 20, 1 },
            { 5, 51 * 20, 1 },
            { 6, ss::get_default_row_height(), 1 },
            { 7, 27 * 20, 3 },
            { 10, ss::get_default_row_height(), 2 },
            { 12, 36 * 20, 2 },
        };

        for (const auto& [row, rh_expected, span_expected] : expected)
        {
            ss::row_t row_start, row_end;
            ss::row_height_t rh = sh->get_row_height(row, &row_start, &row_end);
            ss::row_t span = row_end - row_start;

            std::cout << "row: " << row << "; expected=" << rh_expected << "; actual=" << rh
                << "; span-expected=" << span_expected << "; span-actual=" << span << std::endl;

            assert(rh == rh_expected);
            assert(span == span_expected);
        }
    }
}

void test_gnumeric_auto_filter()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath = SRCDIR"/test/gnumeric/table/autofilter.gnumeric";
    auto doc = load_doc(filepath);

    assert(doc->get_sheet_count() == 1);
    const ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

    const ss::auto_filter_t* af = sh->get_auto_filter_data();
    assert(af);
    ixion::abs_range_t b2_c11{0, 1, 1, 10, 2};
    assert(af->range == b2_c11);
    assert(af->columns.size() == 2);

    auto it = af->columns.begin();
    assert(it->first == 0);
    {
        const ss::auto_filter_column_t& afc = it->second;
        assert(afc.match_values.size() == 1);
        assert(*afc.match_values.begin() == "A");
    }

    ++it;
    assert(it->first == 1);
    {
        const ss::auto_filter_column_t& afc = it->second;
        assert(afc.match_values.size() == 1);
        assert(*afc.match_values.begin() == "1");
    }
}

void test_gnumeric_hidden_rows_columns()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath = SRCDIR"/test/gnumeric/hidden-rows-columns/input.gnumeric";
    auto doc = load_doc(filepath);

    ss::sheet* sh = doc->get_sheet("Hidden Rows");
    assert(sh);

    ss::row_t row_start = -1, row_end = -1;

    // Row 1 is visible.
    assert(!sh->is_row_hidden(0, &row_start, &row_end));
    assert(row_start == 0);
    assert(row_end == 1); // the end position is non-inclusive.

    // Rows 2-3 are hidden.
    assert(sh->is_row_hidden(1, &row_start, &row_end));
    assert(row_start == 1);
    assert(row_end == 3); // the end position is non-inclusive.

    // Row 4 is visible.
    assert(!sh->is_row_hidden(3, &row_start, &row_end));
    assert(row_start == 3);
    assert(row_end == 4); // the end position is non-inclusive.

    // Row 5 is hidden.
    assert(sh->is_row_hidden(4, &row_start, &row_end));
    assert(row_start == 4);
    assert(row_end == 5); // the end position is non-inclusive.

    // Rows 6-8 are visible.
    assert(!sh->is_row_hidden(5, &row_start, &row_end));
    assert(row_start == 5);
    assert(row_end == 8); // the end position is non-inclusive.

    // Row 9 is hidden.
    assert(sh->is_row_hidden(8, &row_start, &row_end));
    assert(row_start == 8);
    assert(row_end == 9); // the end position is non-inclusive.

    // The rest of the rows are visible.
    assert(!sh->is_row_hidden(9, &row_start, &row_end));
    assert(row_start == 9);
    assert(row_end == doc->get_sheet_size().rows); // the end position is non-inclusive.

    sh = doc->get_sheet("Hidden Columns");
    assert(sh);

    ss::col_t col_start = -1, col_end = -1;

    // Columns A-B are visible.
    assert(!sh->is_col_hidden(0, &col_start, &col_end));
    assert(col_start == 0);
    assert(col_end == 2); // non-inclusive

    // Columns C-E are hidden.
    assert(sh->is_col_hidden(2, &col_start, &col_end));
    assert(col_start == 2);
    assert(col_end == 6); // non-inclusive

    // Columns G-J are visible.
    assert(!sh->is_col_hidden(6, &col_start, &col_end));
    assert(col_start == 6);
    assert(col_end == 10); // non-inclusive

    // Column K is hidden.
    assert(sh->is_col_hidden(10, &col_start, &col_end));
    assert(col_start == 10);
    assert(col_end == 11); // non-inclusive

    // The rest of the columns are all visible.
    assert(!sh->is_col_hidden(11, &col_start, &col_end));
    assert(col_start == 11);
    assert(col_end == doc->get_sheet_size().columns); // non-inclusive
}

void test_gnumeric_merged_cells()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath = SRCDIR"/test/gnumeric/merged-cells/input.gnumeric";
    auto doc = load_doc(filepath);

    const ss::sheet* sheet1 = doc->get_sheet("Sheet1");
    assert(sheet1);

    ss::range_t merge_range = sheet1->get_merge_cell_range(0, 1);
    assert(merge_range.first.column == 1);
    assert(merge_range.last.column == 2);
    assert(merge_range.first.row == 0);
    assert(merge_range.last.row == 0);

    merge_range = sheet1->get_merge_cell_range(0, 3);
    assert(merge_range.first.column == 3);
    assert(merge_range.last.column == 5);
    assert(merge_range.first.row == 0);
    assert(merge_range.last.row == 0);

    merge_range = sheet1->get_merge_cell_range(1, 0);
    assert(merge_range.first.column == 0);
    assert(merge_range.last.column == 0);
    assert(merge_range.first.row == 1);
    assert(merge_range.last.row == 2);

    merge_range = sheet1->get_merge_cell_range(3, 0);
    assert(merge_range.first.column == 0);
    assert(merge_range.last.column == 0);
    assert(merge_range.first.row == 3);
    assert(merge_range.last.row == 5);

    merge_range = sheet1->get_merge_cell_range(2, 2);
    assert(merge_range.first.column == 2);
    assert(merge_range.last.column == 5);
    assert(merge_range.first.row == 2);
    assert(merge_range.last.row == 5);
}

void test_gnumeric_text_alignment()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath = SRCDIR"/test/gnumeric/text-alignment/input.gnumeric";
    auto doc = load_doc(filepath);

    ss::styles& styles = doc->get_styles();

    ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

    struct check
    {
        ss::row_t row;
        ss::col_t col;
        bool apply_align;
        ss::hor_alignment_t hor_align;
        ss::ver_alignment_t ver_align;
    };

    std::vector<check> checks =
    {
        {  1, 2,  true, ss::hor_alignment_t::unknown,     ss::ver_alignment_t::bottom      }, // C2
        {  2, 2,  true, ss::hor_alignment_t::left,        ss::ver_alignment_t::bottom      }, // C3
        {  3, 2,  true, ss::hor_alignment_t::center,      ss::ver_alignment_t::bottom      }, // C4
        {  4, 2,  true, ss::hor_alignment_t::right,       ss::ver_alignment_t::bottom      }, // C5
        {  5, 2,  true, ss::hor_alignment_t::left,        ss::ver_alignment_t::bottom      }, // C6
        {  6, 2,  true, ss::hor_alignment_t::left,        ss::ver_alignment_t::bottom      }, // C7
        {  7, 2,  true, ss::hor_alignment_t::right,       ss::ver_alignment_t::bottom      }, // C8
        {  8, 2,  true, ss::hor_alignment_t::right,       ss::ver_alignment_t::bottom      }, // C9
        {  9, 2,  true, ss::hor_alignment_t::unknown,     ss::ver_alignment_t::middle      }, // C10
        { 10, 2,  true, ss::hor_alignment_t::left,        ss::ver_alignment_t::middle      }, // C11
        { 11, 2,  true, ss::hor_alignment_t::center,      ss::ver_alignment_t::middle      }, // C12
        { 12, 2,  true, ss::hor_alignment_t::right,       ss::ver_alignment_t::middle      }, // C13
        { 13, 2,  true, ss::hor_alignment_t::left,        ss::ver_alignment_t::middle      }, // C14
        { 14, 2,  true, ss::hor_alignment_t::left,        ss::ver_alignment_t::middle      }, // C15
        { 15, 2,  true, ss::hor_alignment_t::right,       ss::ver_alignment_t::middle      }, // C16
        { 16, 2,  true, ss::hor_alignment_t::right,       ss::ver_alignment_t::middle      }, // C17
        { 17, 2,  true, ss::hor_alignment_t::unknown,     ss::ver_alignment_t::top         }, // C18
        { 18, 2,  true, ss::hor_alignment_t::left,        ss::ver_alignment_t::top         }, // C19
        { 19, 2,  true, ss::hor_alignment_t::center,      ss::ver_alignment_t::top         }, // C20
        { 20, 2,  true, ss::hor_alignment_t::right,       ss::ver_alignment_t::top         }, // C21
        { 21, 2,  true, ss::hor_alignment_t::left,        ss::ver_alignment_t::top         }, // C22
        { 22, 2,  true, ss::hor_alignment_t::left,        ss::ver_alignment_t::top         }, // C23
        { 23, 2,  true, ss::hor_alignment_t::right,       ss::ver_alignment_t::top         }, // C24
        { 24, 2,  true, ss::hor_alignment_t::right,       ss::ver_alignment_t::top         }, // C25
        { 25, 2,  true, ss::hor_alignment_t::unknown,     ss::ver_alignment_t::justified   }, // C26
        { 26, 2,  true, ss::hor_alignment_t::justified,   ss::ver_alignment_t::bottom      }, // C27
        { 27, 2,  true, ss::hor_alignment_t::distributed, ss::ver_alignment_t::distributed }, // C28
    };

    for (const check& c : checks)
    {
        std::cout << "row=" << c.row << "; col=" << c.col << std::endl;
        size_t xf = sh->get_cell_format(c.row, c.col);

        const ss::cell_format_t* cf = styles.get_cell_format(xf);
        assert(cf);
        assert(c.apply_align == cf->apply_alignment);

        if (!cf->apply_alignment)
            continue;

        assert(c.hor_align == cf->hor_align);
        assert(c.ver_align == cf->ver_align);
    }
}

void test_gnumeric_cell_properties_wrap_and_shrink()
{
    ORCUS_TEST_FUNC_SCOPE;

    // NB : Gnumeric doesn't appear to support shrink-to-fit, so we only check
    // wrap-text for now.  When Gnumeric supports shrink-to-fit, re-generate the
    // test file from test/xls-xml/cell-properties/wrap-and-shrink.xml.

    fs::path filepath = SRCDIR"/test/gnumeric/cell-properties/wrap-and-shrink.gnumeric";
    auto doc = load_doc(filepath);

    const ss::styles& styles = doc->get_styles();
    const ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

    std::size_t xfid = sh->get_cell_format(0, 1); // B1
    const ss::cell_format_t* xf = styles.get_cell_format(xfid);
    assert(xf);
    assert(xf->wrap_text);
    assert(!*xf->wrap_text);
//  assert(xf->shrink_to_fit);
//  assert(!*xf->shrink_to_fit);

    xfid = sh->get_cell_format(1, 1); // B2
    xf = styles.get_cell_format(xfid);
    assert(xf);
    assert(xf->wrap_text);
    assert(*xf->wrap_text);
//  assert(xf->shrink_to_fit);
//  assert(!*xf->shrink_to_fit);

    xfid = sh->get_cell_format(2, 1); // B3
    xf = styles.get_cell_format(xfid);
    assert(xf);
    assert(xf->wrap_text);
    assert(!*xf->wrap_text);
//  assert(xf->shrink_to_fit);
//  assert(*xf->shrink_to_fit);
}

void test_gnumeric_background_fill()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath = SRCDIR"/test/gnumeric/background-color/standard.gnumeric";
    auto doc = load_doc(filepath);

    ss::styles& styles = doc->get_styles();

    ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

    struct check
    {
        ss::row_t row;
        ss::col_t col;
        ss::fill_pattern_t pattern_type;
        ss::color_t fg_color;
    };

    std::vector<check> checks =
    {
        {  1, 0, ss::fill_pattern_t::solid, { 255, 192,   0,   0 } }, // A2  - dark red
        {  2, 0, ss::fill_pattern_t::solid, { 255, 255,   0,   0 } }, // A3  - red
        {  3, 0, ss::fill_pattern_t::solid, { 255, 255, 192,   0 } }, // A4  - orange
        {  4, 0, ss::fill_pattern_t::solid, { 255, 255, 255,   0 } }, // A5  - yellow
        {  5, 0, ss::fill_pattern_t::solid, { 255, 146, 208,  80 } }, // A6  - light green
        {  6, 0, ss::fill_pattern_t::solid, { 255,   0, 176,  80 } }, // A7  - green
        {  7, 0, ss::fill_pattern_t::solid, { 255,   0, 176, 240 } }, // A8  - light blue
        {  8, 0, ss::fill_pattern_t::solid, { 255,   0, 112, 192 } }, // A9  - blue
        {  9, 0, ss::fill_pattern_t::solid, { 255,   0,  32,  96 } }, // A10 - dark blue
        { 10, 0, ss::fill_pattern_t::solid, { 255, 112,  48, 160 } }, // A11 - purple
    };

    ss::color_t color_white(255, 255, 255, 255);

    for (const check& c : checks)
    {
        size_t xf = sh->get_cell_format(c.row, c.col);

        const ss::cell_format_t* cf = styles.get_cell_format(xf);
        assert(cf);

        const ss::fill_t* fill_data = styles.get_fill(cf->fill);
        assert(fill_data);
        assert(fill_data->pattern_type == c.pattern_type);
        assert(fill_data->fg_color == c.fg_color);

        // The font colors are all white in the colored cells.
        const ss::font_t* font_data = styles.get_font(cf->font);
        assert(font_data);

        assert(font_data->color == color_white);
    }
}

void test_gnumeric_colored_text()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath = SRCDIR"/test/gnumeric/colored-text/input.gnumeric";
    auto doc = load_doc(filepath);

    const ss::sheet* sheet1 = doc->get_sheet("ColoredText");
    assert(sheet1);

    const ss::shared_strings& ss = doc->get_shared_strings();

    const ss::styles& styles = doc->get_styles();

    // Column A contains colored cells.

    struct check
    {
        ss::row_t row;
        ss::color_elem_t red;
        ss::color_elem_t green;
        ss::color_elem_t blue;
        std::string text;
    };

    std::vector<check> checks = {
        {  1, 0xC0, 0x00, 0x00, "Dark Red"    },
        {  2, 0xFF, 0x00, 0x00, "Red"         },
        {  3, 0xFF, 0xC0, 0x00, "Orange"      },
        {  4, 0xFF, 0xFF, 0x00, "Yellow"      },
        {  5, 0x92, 0xD0, 0x50, "Light Green" },
        {  6, 0x00, 0xB0, 0x50, "Green"       },
        {  7, 0x00, 0xB0, 0xF0, "Light Blue"  },
        {  8, 0x00, 0x70, 0xC0, "Blue"        },
        {  9, 0x00, 0x20, 0x60, "Dark Blue"   },
        { 10, 0x70, 0x30, 0xA0, "Purple"      },
    };

    for (const check& c : checks)
    {
        size_t xfi = sheet1->get_cell_format(c.row, 0);
        const ss::cell_format_t* xf = styles.get_cell_format(xfi);
        assert(xf);

        const ss::font_t* font = styles.get_font(xf->font);
        assert(font);
        assert(font->color);
        assert(font->color.value().red == c.red);
        assert(font->color.value().green == c.green);
        assert(font->color.value().blue == c.blue);

        size_t si = sheet1->get_string_identifier(c.row, 0);
        const std::string* s = ss.get_string(si);
        assert(s);
        assert(*s == c.text);
    }

    {
        // Cell B2 contains mix-colored text.
        size_t si = sheet1->get_string_identifier(1, 1);
        const std::string* s = ss.get_string(si);
        assert(s);
        assert(*s == "Red and Blue");
        const spreadsheet::format_runs_t* fmt_runs = ss.get_format_runs(si);
        assert(fmt_runs);

        // There should be 2 segments that are color-formatted.
        assert(fmt_runs->size() == 2);

        // The 'Red' segment should be in red color.
        const spreadsheet::format_run_t* fmt = &fmt_runs->at(0);
        assert(fmt->color);
        assert(fmt->color->alpha == 0xFF);
        assert(fmt->color->red == 0xFF);
        assert(fmt->color->green == 0);
        assert(fmt->color->blue == 0);
        assert(fmt->pos == 0);
        assert(fmt->size == 3);

        // The 'Blue' segment should be in blue color.
        fmt = &fmt_runs->at(1);
        assert(fmt->color);
        assert(fmt->color->alpha == 0xFF);
        assert(fmt->color->red == 0);
        assert(fmt->color->green == 0x00);
        assert(fmt->color->blue == 0xFF);
        assert(fmt->pos == 8);
        assert(fmt->size == 4);
    }

    {
        // Cell B3 too
        size_t si = sheet1->get_string_identifier(2, 1);
        const std::string* s = ss.get_string(si);
        assert(s);
        assert(*s == "Green and Orange");
        const spreadsheet::format_runs_t* fmt_runs = ss.get_format_runs(si);
        assert(fmt_runs);

        assert(fmt_runs->size() == 2);

        // 'Green' segment
        const spreadsheet::format_run_t* fmt = &fmt_runs->at(0);
        assert(fmt->color);
        assert(fmt->color->alpha == 0xFF);
        assert(fmt->color->red == 0);
        assert(fmt->color->green == 0xFF);
        assert(fmt->color->blue == 0);
        assert(fmt->pos == 0);
        assert(fmt->size == 5);

        // 'Orange' segment
        fmt = &fmt_runs->at(1);
        assert(fmt->color);
        assert(fmt->color->alpha == 0xFF);
        assert(fmt->color->red == 0xFF);
        assert(fmt->color->green == 0x99);
        assert(fmt->color->blue == 0);
        assert(fmt->pos == 10);
        assert(fmt->size == 6);
    }
}

void test_gnumeric_text_formats()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath = SRCDIR"/test/gnumeric/text-formats/input.gnumeric";
    auto doc = load_doc(filepath);
    assert(doc);

    const auto& styles_pool = doc->get_styles();

    auto get_font = [&styles_pool](const ss::sheet& sh, ss::row_t row, ss::col_t col)
    {
        std::size_t xf = sh.get_cell_format(row, col);

        const ss::cell_format_t* cell_format = styles_pool.get_cell_format(xf);
        assert(cell_format);

        const ss::font_t* font = styles_pool.get_font(cell_format->font);
        assert(font);

        return font;
    };

    auto check_cell_bold = [&get_font](const ss::sheet& sh, ss::row_t row, ss::col_t col, bool expected)
    {
        const ss::font_t* font = get_font(sh, row, col);

        if (expected)
        {
            if (font->bold && *font->bold)
                return true;

            std::cerr << "expected to be bold but it is not "
                << "(sheet=" << sh.get_index() << "; row=" << row << "; column=" << col << ")"
                << std::endl;

            return false;
        }
        else
        {
            if (!font->bold || !*font->bold)
                return true;

            std::cerr << "expected to be non-bold but it is bold "
                << "(sheet=" << sh.get_index() << "; row=" << row << "; column=" << col << ")"
                << std::endl;

            return false;
        }
    };

    auto check_cell_italic = [&get_font](const ss::sheet& sh, ss::row_t row, ss::col_t col, bool expected)
    {
        const ss::font_t* font = get_font(sh, row, col);

        if (expected)
        {
            if (font->italic && *font->italic)
                return true;

            std::cerr << "expected to be italic but it is not "
                << "(sheet=" << sh.get_index() << "; row=" << row << "; column=" << col << ")"
                << std::endl;

            return false;
        }
        else
        {
            if (!font->italic || !*font->italic)
                return true;

            std::cerr << "expected to be non-italic but it is italic "
                << "(sheet=" << sh.get_index() << "; row=" << row << "; column=" << col << ")"
                << std::endl;

            return false;
        }
    };

    auto check_cell_text = [&doc](const ss::sheet& sh, ss::row_t row, ss::col_t col, std::string_view expected)
    {
        const auto& sstrings = doc->get_shared_strings();

        std::size_t si = sh.get_string_identifier(row, col);
        const std::string* s = sstrings.get_string(si);
        if (!s)
        {
            std::cerr << "expected='" << expected << "'; actual=<none> "
                << "(sheet=" << sh.get_index() << "; row=" << row << "; column=" << col << ")"
                << std::endl;

            return false;
        }

        if (*s == expected)
            return true;

        std::cerr << "expected='" << expected << "'; actual='" << *s << "' "
            << "(sheet=" << sh.get_index() << "; row=" << row << "; column=" << col << ")"
            << std::endl;

        return false;
    };

    const ss::sheet* sheet1 = doc->get_sheet("Text Properties");
    assert(sheet1);

    ss::row_t row = 0;
    ss::col_t col = 0;

    // A1 - unformatted
    assert(check_cell_text(*sheet1, row, col, "Normal Text"));
    assert(check_cell_bold(*sheet1, row, col, false));
    assert(check_cell_italic(*sheet1, row, col, false));

    // A2 - bold
    row = 1;
    assert(check_cell_text(*sheet1, row, col, "Bold Text"));
    assert(check_cell_bold(*sheet1, row, col, true));
    assert(check_cell_italic(*sheet1, row, col, false));

    // A3 - italic
    row = 2;
    assert(check_cell_text(*sheet1, row, col, "Italic Text"));
    assert(check_cell_bold(*sheet1, row, col, false));
    assert(check_cell_italic(*sheet1, row, col, true));

    // A4 - bold and italic
    row = 3;
    assert(check_cell_text(*sheet1, row, col, "Bold and Italic Text"));
    assert(check_cell_bold(*sheet1, row, col, true));
    assert(check_cell_italic(*sheet1, row, col, true));

    // A5 - bold and italic mixed - base cell is unformatted and text contains
    // format runs.
    row = 4;
    assert(check_cell_text(*sheet1, row, col, "Bold and Italic mixed"));
    assert(check_cell_bold(*sheet1, row, col, false));
    assert(check_cell_italic(*sheet1, row, col, false));

    std::size_t si = sheet1->get_string_identifier(row, col);
    const ss::format_runs_t* runs = doc->get_shared_strings().get_format_runs(si);
    assert(runs);
    assert(runs->size() == 2u);

    // Bold and ...
    // ^^^^
    assert(runs->at(0).pos == 0);
    assert(runs->at(0).size == 4);
    assert(runs->at(0).bold);
    assert(!runs->at(0).italic);

    // Bold and Italic
    //          ^^^^^^
    assert(runs->at(1).pos == 9);
    assert(runs->at(1).size == 6);
    assert(!runs->at(1).bold);
    assert(runs->at(1).italic);

    // A6
    row = 5;
    assert(check_cell_text(*sheet1, row, col, "Bold base with non-bold part"));
    assert(check_cell_bold(*sheet1, row, col, true));
    assert(check_cell_italic(*sheet1, row, col, false));

    si = sheet1->get_string_identifier(row, col);
    runs = doc->get_shared_strings().get_format_runs(si);
    assert(runs);
    assert(runs->size() == 1u);
    // Bold base with non-bold part
    //                ^^^^^^^^
    assert(runs->at(0).pos == 15);
    assert(runs->at(0).size == 8);
    assert(runs->at(0).bold && !runs->at(0).bold.value()); // explicit non-bold segment
    assert(!runs->at(0).italic);

    // Rest of the cells are imported as unformatted for now, until we support
    // more format properties. See #182.
    row = 6;
    assert(check_cell_text(*sheet1, row, col, "Only partially underlined"));

    {
        row = 7;
        assert(check_cell_text(*sheet1, row, col, "All Underlined"));
        const ss::font_t* font = get_font(*sheet1, row, col);
        assert(font->underline_style);
        assert(*font->underline_style == ss::underline_style_t::solid);
        assert(*font->underline_count == ss::underline_count_t::single_count);
    }

    {
        row = 8;
        assert(check_cell_text(*sheet1, row, col, "Bold and Underlined"));
        const ss::font_t* font = get_font(*sheet1, row, col);
        assert(font->underline_style);
        assert(*font->underline_style == ss::underline_style_t::solid);
        assert(*font->underline_count == ss::underline_count_t::single_count);
        assert(font->bold);
        assert(*font->bold);
    }

    {
        row = 9;
        assert(check_cell_text(*sheet1, row, col, "All Strikethrough"));
        const ss::font_t* font = get_font(*sheet1, row, col);
        assert(test::strikethrough_set(font->strikethrough));
    }

    {
        row = 10;
        assert(check_cell_text(*sheet1, row, col, "Partial strikethrough"));
        si = sheet1->get_string_identifier(row, col);
        runs = doc->get_shared_strings().get_format_runs(si);
        assert(runs);
        assert(runs->size() == 1);
        // Partial strikethrough
        //         ^^^^^^^^^^^^^
        assert(runs->at(0).pos == 8);
        assert(runs->at(0).size == 13);
        assert(test::strikethrough_set(runs->at(0).strikethrough));
    }

    row = 11;
    assert(check_cell_text(*sheet1, row, col, "Superscript"));
    row = 12;
    assert(check_cell_text(*sheet1, row, col, "Subscript"));

    {
        row = 13;
        assert(check_cell_text(*sheet1, row, col, "x2 + y2 = 102"));
        si = sheet1->get_string_identifier(row, col);
        runs = doc->get_shared_strings().get_format_runs(si);
        assert(runs);
        assert(runs->size() == 3);
        assert(runs->at(0).pos == 1);
        assert(runs->at(0).size == 1);
        assert(runs->at(0).superscript && *runs->at(0).superscript);
        assert(runs->at(1).pos == 6);
        assert(runs->at(1).size == 1);
        assert(runs->at(1).superscript && *runs->at(0).superscript);
        assert(runs->at(2).pos == 12);
        assert(runs->at(2).size == 1);
        assert(runs->at(2).superscript && *runs->at(0).superscript);
    }

    {
        row = 14;
        assert(check_cell_text(*sheet1, row, col, "xi = yi + zi"));
        si = sheet1->get_string_identifier(row, col);
        runs = doc->get_shared_strings().get_format_runs(si);
        assert(runs);
        assert(runs->size() == 3);
        assert(runs->at(0).pos == 1);
        assert(runs->at(0).size == 1);
        assert(runs->at(0).subscript && *runs->at(0).subscript);
        assert(runs->at(1).pos == 6);
        assert(runs->at(1).size == 1);
        assert(runs->at(1).subscript && *runs->at(0).subscript);
        assert(runs->at(2).pos == 11);
        assert(runs->at(2).size == 1);
        assert(runs->at(2).subscript && *runs->at(0).subscript);
    }

    {
        const ss::sheet* sheet2 = doc->get_sheet("Fonts");
        assert(sheet2);

        struct check
        {
            ss::row_t row;
            std::string_view font_name;
            double font_unit;
        };

        check checks[] = {
            { 0, "Sans", 12.0 },
            { 1, "FreeSans", 18.0 },
            { 2, "Serif", 14.0 },
            { 3, "Monospace", 9.0 },
            { 4, "DejaVu Sans Mono", 11.0 },
        };

        for (const auto& c : checks)
        {
            std::size_t xf = sheet2->get_cell_format(c.row, 0);
            const ss::cell_format_t* cell_format = styles_pool.get_cell_format(xf);
            assert(cell_format);
            const ss::font_t* font = styles_pool.get_font(cell_format->font);
            assert(font);
            assert(font->name == c.font_name);
            assert(font->size == c.font_unit);

            // Columns A and B should have the same font.
            xf = sheet2->get_cell_format(c.row, 1);
            cell_format = styles_pool.get_cell_format(xf);
            assert(cell_format);
            font = styles_pool.get_font(cell_format->font);
            assert(font);
            assert(font->name == c.font_name);
            assert(font->size == c.font_unit);
        }
    }

    {
        const ss::sheet* sheet3 = doc->get_sheet("Mixed Fonts");
        assert(sheet3);

        // A1
        row = 0;
        col = 0;
        assert(check_cell_text(*sheet3, row, col, "C++ has class and struct as keywords."));

        // Base cell has Serif 12-pt font applied
        auto xf = sheet3->get_cell_format(row, col);
        const ss::cell_format_t* fmt = styles_pool.get_cell_format(xf);
        assert(fmt);
        const ss::font_t* font = styles_pool.get_font(fmt->font);
        assert(font);
        assert(font->name == "Serif");
        assert(font->size == 12.0f);

        // two segments where Monospace font is applied
        si = sheet3->get_string_identifier(row, col);
        runs = doc->get_shared_strings().get_format_runs(si);
        assert(runs);
        assert(runs->size() == 2u);

        // C++ has class ...
        //         ^^^^^
        assert(runs->at(0).pos == 8);
        assert(runs->at(0).size == 5);
        assert(runs->at(0).font == "Monospace");

        // ... and struct as ...
        //         ^^^^^^
        assert(runs->at(1).pos == 18);
        assert(runs->at(1).size == 6);
        assert(runs->at(1).font == "Monospace");

        // A2
        row = 1;
        assert(check_cell_text(*sheet3, row, col, "Text with 12-point font, 24-point font, and 36-point font mixed."));
        si = sheet3->get_string_identifier(row, col);
        runs = doc->get_shared_strings().get_format_runs(si);
        assert(runs);
        assert(runs->size() == 6u);

        // with 12-point font, ...
        //      ^^
        assert(runs->at(0).pos == 10);
        assert(runs->at(0).size == 2);
        assert(runs->at(0).font_size == 12.0f);
        assert(runs->at(0).color == ss::color_t(0xFF, 0xFF, 0, 0)); // red

        // with 12-point font, ...
        //        ^^^^^^
        assert(runs->at(1).pos == 12);
        assert(runs->at(1).size == 6);
        assert(runs->at(1).font_size == 12.0f);

        // 24-point font,
        // ^^
        assert(runs->at(2).pos == 25);
        assert(runs->at(2).size == 2);
        assert(runs->at(2).font_size == 24.0f);
        assert(runs->at(2).color == ss::color_t(0xFF, 0xFF, 0, 0)); // red

        // 24-point font,
        //   ^^^^^^
        assert(runs->at(3).pos == 27);
        assert(runs->at(3).size == 6);
        assert(runs->at(3).font_size == 24.0f);

        // and 36-point font
        //     ^^
        assert(runs->at(4).pos == 44);
        assert(runs->at(4).size == 2);
        assert(runs->at(4).font_size == 36.0f);
        assert(runs->at(4).color == ss::color_t(0xFF, 0xFF, 0, 0)); // red

        // and 36-point font
        //       ^^^^^^
        assert(runs->at(5).pos == 46);
        assert(runs->at(5).size == 6);
        assert(runs->at(5).font_size == 36.0f);
    }
}

void test_gnumeric_cell_borders_single_cells()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath = SRCDIR"/test/gnumeric/borders/single-cells.gnumeric";
    auto doc = load_doc(filepath);
    assert(doc);

    ss::styles& styles = doc->get_styles();

    ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

    struct check
    {
        ss::row_t row;
        ss::col_t col;
        ss::border_style_t style;
    };

    std::vector<check> checks =
    {
        {  3, 1, ss::border_style_t::hair                },
        {  5, 1, ss::border_style_t::dotted              },
        {  7, 1, ss::border_style_t::dash_dot_dot        },
        {  9, 1, ss::border_style_t::dash_dot            },
        { 11, 1, ss::border_style_t::dashed              },
        { 13, 1, ss::border_style_t::thin                },
        {  1, 3, ss::border_style_t::medium_dash_dot_dot },
        {  3, 3, ss::border_style_t::slant_dash_dot      },
        {  5, 3, ss::border_style_t::medium_dash_dot     },
        {  7, 3, ss::border_style_t::medium_dashed       },
        {  9, 3, ss::border_style_t::medium              },
        { 11, 3, ss::border_style_t::thick               },
        { 13, 3, ss::border_style_t::double_border       },
    };

    for (const check& c : checks)
    {
        std::cout << "(row: " << c.row << "; col: " << c.col << "; expected: " << int(c.style) << ")" << std::endl;
        size_t xf = sh->get_cell_format(c.row, c.col);
        const ss::cell_format_t* cf = styles.get_cell_format(xf);
        assert(cf);
        assert(cf->apply_border);

        const ss::border_t* border = styles.get_border(cf->border);
        assert(border);
        assert(border->top.style    == c.style);
        assert(border->bottom.style == c.style);
        assert(border->left.style   == c.style);
        assert(border->right.style  == c.style);
    }
}

void test_gnumeric_cell_borders_directions()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath = SRCDIR"/test/gnumeric/borders/directions.gnumeric";
    auto doc = load_doc(filepath);
    assert(doc);

    ss::styles& styles = doc->get_styles();

    ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

    struct check
    {
        ss::row_t row;
        ss::col_t col;
        ss::border_direction_t dir;
    };

    std::vector<check> checks =
    {
        {  1, 1, ss::border_direction_t::top            },
        {  3, 1, ss::border_direction_t::left           },
        {  5, 1, ss::border_direction_t::right          },
        {  7, 1, ss::border_direction_t::bottom         },
        {  9, 1, ss::border_direction_t::diagonal_tl_br },
        { 11, 1, ss::border_direction_t::diagonal_bl_tr },
        { 13, 1, ss::border_direction_t::diagonal       },
    };

    const ss::color_t black{255, 0, 0, 0};

    for (const check& c : checks)
    {
        size_t xf = sh->get_cell_format(c.row, c.col);
        const ss::cell_format_t* cf = styles.get_cell_format(xf);
        assert(cf);
        assert(cf->apply_border);

        const ss::border_t* border = styles.get_border(cf->border);
        assert(border);

        switch (c.dir)
        {
            case ss::border_direction_t::top:
                assert(border->top.style);
                assert(*border->top.style == ss::border_style_t::thin);
                assert(border->top.border_color);
                assert(*border->top.border_color == black);
                assert(!border->top.border_width);
                assert(!border->bottom.style);
                assert(!border->bottom.border_color);
                assert(!border->bottom.border_width);
                assert(!border->left.style);
                assert(!border->left.border_color);
                assert(!border->left.border_width);
                assert(!border->right.style);
                assert(!border->right.border_color);
                assert(!border->right.border_width);
                assert(!border->diagonal.style);
                assert(!border->diagonal.border_color);
                assert(!border->diagonal.border_width);
                assert(!border->diagonal_bl_tr.style);
                assert(!border->diagonal_bl_tr.border_color);
                assert(!border->diagonal_bl_tr.border_width);
                assert(!border->diagonal_tl_br.style);
                assert(!border->diagonal_tl_br.border_color);
                assert(!border->diagonal_tl_br.border_width);
                break;
            case ss::border_direction_t::left:
                assert(!border->top.style);
                assert(!border->top.border_color);
                assert(!border->top.border_width);
                assert(!border->bottom.style);
                assert(!border->bottom.border_color);
                assert(!border->bottom.border_width);
                assert(border->left.style);
                assert(*border->left.style == ss::border_style_t::thin);
                assert(border->left.border_color);
                assert(*border->left.border_color == black);
                assert(!border->left.border_width);
                assert(!border->right.style);
                assert(!border->right.border_color);
                assert(!border->right.border_width);
                assert(!border->diagonal.style);
                assert(!border->diagonal.border_color);
                assert(!border->diagonal.border_width);
                assert(!border->diagonal_bl_tr.style);
                assert(!border->diagonal_bl_tr.border_color);
                assert(!border->diagonal_bl_tr.border_width);
                assert(!border->diagonal_tl_br.style);
                assert(!border->diagonal_tl_br.border_color);
                assert(!border->diagonal_tl_br.border_width);
                break;
            case ss::border_direction_t::right:
                assert(!border->top.style);
                assert(!border->top.border_color);
                assert(!border->top.border_width);
                assert(!border->bottom.style);
                assert(!border->bottom.border_color);
                assert(!border->bottom.border_width);
                assert(!border->left.style);
                assert(!border->left.border_color);
                assert(!border->left.border_width);
                assert(border->right.style);
                assert(*border->right.style == ss::border_style_t::thin);
                assert(border->right.border_color);
                assert(*border->right.border_color == black);
                assert(!border->right.border_width);
                assert(!border->diagonal.style);
                assert(!border->diagonal.border_color);
                assert(!border->diagonal.border_width);
                assert(!border->diagonal_bl_tr.style);
                assert(!border->diagonal_bl_tr.border_color);
                assert(!border->diagonal_bl_tr.border_width);
                assert(!border->diagonal_tl_br.style);
                assert(!border->diagonal_tl_br.border_color);
                assert(!border->diagonal_tl_br.border_width);
                break;
            case ss::border_direction_t::bottom:
                assert(!border->top.style);
                assert(!border->top.border_color);
                assert(!border->top.border_width);
                assert(border->bottom.style);
                assert(*border->bottom.style == ss::border_style_t::thin);
                assert(border->bottom.border_color);
                assert(*border->bottom.border_color == black);
                assert(!border->bottom.border_width);
                assert(!border->left.style);
                assert(!border->left.border_color);
                assert(!border->left.border_width);
                assert(!border->right.style);
                assert(!border->right.border_color);
                assert(!border->right.border_width);
                assert(!border->diagonal.style);
                assert(!border->diagonal.border_color);
                assert(!border->diagonal.border_width);
                assert(!border->diagonal_bl_tr.style);
                assert(!border->diagonal_bl_tr.border_color);
                assert(!border->diagonal_bl_tr.border_width);
                assert(!border->diagonal_tl_br.style);
                assert(!border->diagonal_tl_br.border_color);
                assert(!border->diagonal_tl_br.border_width);
                break;
            case ss::border_direction_t::diagonal:
                assert(!border->top.style);
                assert(!border->top.border_color);
                assert(!border->top.border_width);
                assert(!border->bottom.style);
                assert(!border->bottom.border_color);
                assert(!border->bottom.border_width);
                assert(!border->left.style);
                assert(!border->left.border_color);
                assert(!border->left.border_width);
                assert(!border->right.style);
                assert(!border->right.border_color);
                assert(!border->right.border_width);
                assert(!border->diagonal.style);
                assert(!border->diagonal.border_color);
                assert(!border->diagonal.border_width);
                assert(border->diagonal_bl_tr.style);
                assert(*border->diagonal_bl_tr.style == ss::border_style_t::thin);
                assert(border->diagonal_bl_tr.border_color);
                assert(*border->diagonal_bl_tr.border_color == black);
                assert(!border->diagonal_bl_tr.border_width);
                assert(border->diagonal_tl_br.style);
                assert(*border->diagonal_tl_br.style == ss::border_style_t::thin);
                assert(border->diagonal_tl_br.border_color);
                assert(*border->diagonal_tl_br.border_color == black);
                assert(!border->diagonal_tl_br.border_width);
                break;
            case ss::border_direction_t::diagonal_tl_br:
                assert(!border->top.style);
                assert(!border->top.border_color);
                assert(!border->top.border_width);
                assert(!border->bottom.style);
                assert(!border->bottom.border_color);
                assert(!border->bottom.border_width);
                assert(!border->left.style);
                assert(!border->left.border_color);
                assert(!border->left.border_width);
                assert(!border->right.style);
                assert(!border->right.border_color);
                assert(!border->right.border_width);
                assert(!border->diagonal.style);
                assert(!border->diagonal.border_color);
                assert(!border->diagonal.border_width);
                assert(!border->diagonal_bl_tr.style);
                assert(!border->diagonal_bl_tr.border_color);
                assert(!border->diagonal_bl_tr.border_width);
                assert(border->diagonal_tl_br.style);
                assert(*border->diagonal_tl_br.style == ss::border_style_t::thin);
                assert(border->diagonal_tl_br.border_color);
                assert(*border->diagonal_tl_br.border_color == black);
                assert(!border->diagonal_tl_br.border_width);
                break;
            case ss::border_direction_t::diagonal_bl_tr:
                assert(!border->top.style);
                assert(!border->top.border_color);
                assert(!border->top.border_width);
                assert(!border->bottom.style);
                assert(!border->bottom.border_color);
                assert(!border->bottom.border_width);
                assert(!border->left.style);
                assert(!border->left.border_color);
                assert(!border->left.border_width);
                assert(!border->right.style);
                assert(!border->right.border_color);
                assert(!border->right.border_width);
                assert(!border->diagonal.style);
                assert(!border->diagonal.border_color);
                assert(!border->diagonal.border_width);
                assert(border->diagonal_bl_tr.style);
                assert(*border->diagonal_bl_tr.style == ss::border_style_t::thin);
                assert(border->diagonal_bl_tr.border_color);
                assert(*border->diagonal_bl_tr.border_color == black);
                assert(!border->diagonal_bl_tr.border_width);
                assert(!border->diagonal_tl_br.style);
                assert(!border->diagonal_tl_br.border_color);
                assert(!border->diagonal_tl_br.border_width);
                break;
            default:
                assert(!"unhandled direction!");
        }
    }
}

void test_gnumeric_cell_borders_colors()
{
    ORCUS_TEST_FUNC_SCOPE;

    using ss::color_t;
    using ss::border_style_t;

    fs::path filepath = SRCDIR"/test/gnumeric/borders/colors.gnumeric";
    auto doc = load_doc(filepath);
    assert(doc);

    ss::styles& styles = doc->get_styles();

    ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

    struct check
    {
        ss::row_t row;
        ss::col_t col;
        color_t color;
    };

    std::vector<check> checks =
    {
        { 2, 1, color_t(0xFF, 0xFF,    0,    0) }, // B3 - red
        { 3, 1, color_t(0xFF,    0, 0x70, 0xC0) }, // B4 - blue
        { 4, 1, color_t(0xFF,    0, 0xB0, 0x50) }, // B5 - green
    };

    for (const check& c : checks)
    {
        size_t xf = sh->get_cell_format(c.row, c.col); // B3

        const ss::cell_format_t* cf = styles.get_cell_format(xf);
        assert(cf);
        assert(cf->apply_border);

        const ss::border_t* border = styles.get_border(cf->border);
        assert(border);

        assert(!border->left.style);
        assert(border->right.style);
        assert(*border->right.style == border_style_t::thick);
        assert(!border->top.style);
        assert(!border->bottom.style);

        assert(border->right.border_color == c.color);
    }

    // B7 contains yellow left border, purple right border, and light blue
    // diagonal borders.

    size_t xf = sh->get_cell_format(6, 1); // B7

    const ss::cell_format_t* cf = styles.get_cell_format(xf);
    assert(cf);
    assert(cf->apply_border);

    const ss::border_t* border = styles.get_border(cf->border);
    assert(border);

    assert(border->left.style == border_style_t::thick);
    assert(border->left.border_color == color_t(0xFF, 0xFF, 0xFF, 0)); // yellow

    assert(border->right.style == border_style_t::thick);
    assert(border->right.border_color == color_t(0xFF, 0x70, 0x30, 0xA0)); // purple

    assert(border->diagonal_bl_tr.style == border_style_t::thick);
    assert(border->diagonal_bl_tr.border_color == color_t(0xFF, 0x00, 0xB0, 0xF0)); // light blue

    assert(border->diagonal_tl_br.style == border_style_t::thick);
    assert(border->diagonal_tl_br.border_color == color_t(0xFF, 0x00, 0xB0, 0xF0)); // light blue

    // B7 also contains multi-line string.  Test that as well.
    ixion::model_context& model = doc->get_model_context();
    ixion::string_id_t sid = model.get_string_identifier(ixion::abs_address_t(0,6,1));
    const std::string* s = model.get_string(sid);
    assert(s);
    assert(*s == "<- Yellow\nPurple ->\nLight Blue \\");
}

void test_gnumeric_number_format()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath = SRCDIR"/test/gnumeric/number-formats/input.gnumeric";
    auto doc = load_doc(filepath);
    assert(doc);

    const spreadsheet::styles& styles = doc->get_styles();

    const spreadsheet::sheet* sh = doc->get_sheet(0);
    assert(sh);

    struct check
    {
        ss::row_t row;
        ss::col_t col;
        std::string_view expected;
    };

    std::vector<check> checks =
    {
        { 1, 1, "[$-F800]dddd\\,\\ mmmm\\ dd\\,\\ yyyy" },
        { 2, 1, "[$-409]mmmm\\ d\\,\\ yyyy;@" },
        { 3, 1, "m/d/yy;@" },
        { 4, 1, "m/d/yy h:mm" }, // General Date
    };

    for (const check& c : checks)
    {
        size_t xf = sh->get_cell_format(c.row, c.col);
        const spreadsheet::cell_format_t* cf = styles.get_cell_format(xf);
        assert(cf);

        const spreadsheet::number_format_t* nf = styles.get_number_format(cf->number_format);
        assert(nf);
        std::cout << "row=" << c.row << "; col=" << c.col << "; expected='"
            << c.expected << "'; actual='" << (nf->format_string ? *nf->format_string : "") << "'" << std::endl;
        assert(nf->format_string == c.expected);
    }
}

} // anonymous namespace

int main()
{
    test_gnumeric_detection();
    test_gnumeric_create_filter();
    test_gnumeric_import();
    test_gnumeric_column_widths_row_heights();
    test_gnumeric_auto_filter();
    test_gnumeric_hidden_rows_columns();
    test_gnumeric_merged_cells();
    test_gnumeric_text_alignment();
    test_gnumeric_cell_properties_wrap_and_shrink();
    test_gnumeric_background_fill();
    test_gnumeric_colored_text();
    test_gnumeric_text_formats();
    test_gnumeric_cell_borders_single_cells();
    test_gnumeric_cell_borders_directions();
    test_gnumeric_cell_borders_colors();
    test_gnumeric_number_format();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
