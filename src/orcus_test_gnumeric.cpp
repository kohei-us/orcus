/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_test_global.hpp"

#include <orcus/orcus_gnumeric.hpp>
#include <orcus/stream.hpp>
#include <orcus/spreadsheet/factory.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/sheet.hpp>
#include <orcus/spreadsheet/auto_filter.hpp>
#include <orcus/spreadsheet/shared_strings.hpp>
#include <orcus/spreadsheet/styles.hpp>

#include <ixion/address.hpp>
#include <boost/filesystem/path.hpp>
#include <iostream>
#include <sstream>

using namespace orcus;
namespace fs = boost::filesystem;
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

    spreadsheet::sheet* sh = doc->get_sheet("Hidden Rows");
    assert(sh);

    spreadsheet::row_t row_start = -1, row_end = -1;

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

    spreadsheet::col_t col_start = -1, col_end = -1;

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

    const spreadsheet::sheet* sheet1 = doc->get_sheet("Sheet1");
    assert(sheet1);

    spreadsheet::range_t merge_range = sheet1->get_merge_cell_range(0, 1);
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
    // NB : Gnumeric doesn't support format runs, so no mixed-color text.

    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath = SRCDIR"/test/gnumeric/colored-text/input.gnumeric";
    auto doc = load_doc(filepath);

    const spreadsheet::sheet* sheet1 = doc->get_sheet("ColoredText");
    assert(sheet1);

    const spreadsheet::shared_strings& ss = doc->get_shared_strings();

    const spreadsheet::styles& styles = doc->get_styles();

    // Column A contains colored cells.

    struct check
    {
        spreadsheet::row_t row;
        spreadsheet::color_elem_t red;
        spreadsheet::color_elem_t green;
        spreadsheet::color_elem_t blue;
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
        const spreadsheet::cell_format_t* xf = styles.get_cell_format(xfi);
        assert(xf);

        const spreadsheet::font_t* font = styles.get_font(xf->font);
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
}

void test_gnumeric_text_formats()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath = SRCDIR"/test/gnumeric/text-formats/input.gnumeric";
    auto doc = load_doc(filepath);
    assert(doc);

    auto is_cell_bold = [&doc](const ss::sheet& sh, ss::row_t row, ss::col_t col, bool expected)
    {
        const auto& styles_pool = doc->get_styles();

        std::size_t xf = sh.get_cell_format(row, col);

        const ss::cell_format_t* cell_format = styles_pool.get_cell_format(xf);
        assert(cell_format);

        const ss::font_t* font = styles_pool.get_font(cell_format->font);
        assert(font);

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

    auto is_cell_italic = [&doc](const ss::sheet& sh, ss::row_t row, ss::col_t col, bool expected)
    {
        const auto& styles_pool = doc->get_styles();

        std::size_t xf = sh.get_cell_format(row, col);

        const ss::cell_format_t* cell_format = styles_pool.get_cell_format(xf);
        assert(cell_format);

        const ss::font_t* font = styles_pool.get_font(cell_format->font);
        assert(font);

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

    auto is_cell_text = [&doc](const ss::sheet& sh, ss::row_t row, ss::col_t col, std::string_view expected)
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
    assert(is_cell_text(*sheet1, row, col, "Normal Text"));
    assert(is_cell_bold(*sheet1, row, col, false));
    assert(is_cell_italic(*sheet1, row, col, false));

    // A2 - bold
    row = 1;
    assert(is_cell_text(*sheet1, row, col, "Bold Text"));
    assert(is_cell_bold(*sheet1, row, col, true));
    assert(is_cell_italic(*sheet1, row, col, false));

    // A3 - italic
    row = 2;
    assert(is_cell_text(*sheet1, row, col, "Italic Text"));
    assert(is_cell_bold(*sheet1, row, col, false));
    assert(is_cell_italic(*sheet1, row, col, true));

    // A4 - bold and italic
    row = 3;
    assert(is_cell_text(*sheet1, row, col, "Bold and Italic Text"));
    assert(is_cell_bold(*sheet1, row, col, true));
    assert(is_cell_italic(*sheet1, row, col, true));

    // A5 - bold and italic mixed - base cell is unformatted and text contains
    // format runs.
    row = 4;
    assert(is_cell_text(*sheet1, row, col, "Bold and Italic mixed"));
    assert(is_cell_bold(*sheet1, row, col, false));
    assert(is_cell_italic(*sheet1, row, col, false));

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
}

} // anonymous namespace

int main()
{
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

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
