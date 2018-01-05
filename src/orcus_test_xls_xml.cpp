/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_xls_xml.hpp"
#include "orcus/pstring.hpp"
#include "orcus/global.hpp"
#include "orcus/stream.hpp"
#include "orcus/config.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/view.hpp"
#include "orcus/spreadsheet/sheet.hpp"
#include "orcus/spreadsheet/shared_strings.hpp"
#include "orcus/spreadsheet/styles.hpp"

#include <cassert>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <iostream>

#include "orcus_test_global.hpp"

using namespace orcus;
using namespace std;

namespace {

config test_config(format_t::xls_xml);

std::vector<const char*> dirs = {
    SRCDIR"/test/xls-xml/basic/",
    SRCDIR"/test/xls-xml/bold-and-italic/",
    SRCDIR"/test/xls-xml/colored-text/",
    SRCDIR"/test/xls-xml/empty-rows/",
    SRCDIR"/test/xls-xml/formula-cells/",
    SRCDIR"/test/xls-xml/merged-cells/",
    SRCDIR"/test/xls-xml/named-expression/",
    SRCDIR"/test/xls-xml/named-expression-sheet-local/",
    SRCDIR"/test/xls-xml/raw-values-1/",
};

std::unique_ptr<spreadsheet::document> load_doc(const string& path)
{
    std::unique_ptr<spreadsheet::document> doc = orcus::make_unique<spreadsheet::document>();
    spreadsheet::import_factory factory(*doc);
    orcus_xls_xml app(&factory);
    app.read_file(path.c_str());

    return doc;
}

void test_xls_xml_import()
{
    for (const char* dir : dirs)
    {
        cout << dir << endl;

        string path(dir);

        // Read the input.xml document.
        path.append("input.xml");
        std::unique_ptr<spreadsheet::document> doc = load_doc(path);

        // Dump the content of the model.
        ostringstream os;
        doc->dump_check(os);
        string check = os.str();

        // Check that against known control.
        path = dir;
        path.append("check.txt");
        string control = load_file_content(path.c_str());

        assert(!check.empty());
        assert(!control.empty());

        pstring s1(&check[0], check.size()), s2(&control[0], control.size());
        assert(s1.trim() == s2.trim());
    }
}

void test_xls_xml_merged_cells()
{
    const char* filepath = SRCDIR"/test/xls-xml/merged-cells/input.xml";

    std::unique_ptr<spreadsheet::document> doc = load_doc(filepath);

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

void test_xls_xml_date_time()
{
    const char* filepath = SRCDIR"/test/xls-xml/date-time/input.xml";

    std::unique_ptr<spreadsheet::document> doc = load_doc(filepath);

    const spreadsheet::sheet* sheet1 = doc->get_sheet("Sheet1");
    assert(sheet1);

    // B1 contains date-only value.
    date_time_t dt = sheet1->get_date_time(0, 1);
    assert(dt == date_time_t(2016, 12, 14));

    // B2 contains date-time value with no fraction seconds.
    dt = sheet1->get_date_time(1, 1);
    assert(dt == date_time_t(2002, 2, 3, 12, 34, 45));

    // B3 contains date-time value with fraction second (1992-03-04 08:34:33.555)
    dt = sheet1->get_date_time(2, 1);
    assert(dt.year == 1992);
    assert(dt.month == 3);
    assert(dt.day == 4);
    assert(dt.hour == 8);
    assert(dt.minute == 34);
    assert(std::floor(dt.second) == 33.0);

    // Evalutate the fraction second as milliseconds.
    double ms = dt.second * 1000.0;
    ms -= std::floor(dt.second) * 1000.0;
    ms = std::round(ms);
    assert(ms == 555.0);
}

void test_xls_xml_bold_and_italic()
{
    std::unique_ptr<spreadsheet::document> doc =
        load_doc(SRCDIR"/test/xls-xml/bold-and-italic/input.xml");

    const spreadsheet::sheet* sheet1 = doc->get_sheet("Sheet1");
    assert(sheet1);

    const spreadsheet::import_shared_strings* ss = doc->get_shared_strings();
    assert(ss);

    const spreadsheet::import_styles* styles = doc->get_styles();
    assert(styles);

    // A1 contains unformatted text.
    size_t si = sheet1->get_string_identifier(0, 0);
    const string* sp = ss->get_string(si);
    assert(sp);
    assert(*sp == "Normal Text");

    // A2 contains bold text.
    si = sheet1->get_string_identifier(1, 0);
    sp = ss->get_string(si);
    assert(sp);
    assert(*sp == "Bold Text");

    size_t xfi = sheet1->get_cell_format(1, 0);
    const spreadsheet::cell_format_t* cf = styles->get_cell_format(xfi);
    assert(cf);
    const spreadsheet::font_t* font = styles->get_font(cf->font);
    assert(font);
    assert(font->bold);
    assert(!font->italic);

    // A3 contains italic text.
    si = sheet1->get_string_identifier(2, 0);
    sp = ss->get_string(si);
    assert(sp);
    assert(*sp == "Italic Text");

    xfi = sheet1->get_cell_format(2, 0);
    cf = styles->get_cell_format(xfi);
    assert(cf);
    font = styles->get_font(cf->font);
    assert(font);
    assert(!font->bold);
    assert(font->italic);

    // A4 contains bold and italic text.
    si = sheet1->get_string_identifier(3, 0);
    sp = ss->get_string(si);
    assert(sp);
    assert(*sp == "Bold and Italic Text");

    xfi = sheet1->get_cell_format(3, 0);
    cf = styles->get_cell_format(xfi);
    assert(cf);
    font = styles->get_font(cf->font);
    assert(font);
    assert(font->bold);
    assert(font->italic);

    // A5 contains a mixed format text.
    si = sheet1->get_string_identifier(4, 0);
    sp = ss->get_string(si);
    assert(sp);
    assert(*sp == "Bold and Italic mixed");

    // The string contains 2 formatted segments.
    const spreadsheet::format_runs_t* fmt_runs = ss->get_format_runs(si);
    assert(fmt_runs);
    assert(fmt_runs->size() == 2);

    // First formatted segment is bold.
    const spreadsheet::format_run* fmt_run = &fmt_runs->at(0);
    assert(fmt_run->pos == 0);
    assert(fmt_run->size == 4);
    assert(fmt_run->bold);
    assert(!fmt_run->italic);

    // Second formatted segment is italic.
    fmt_run = &fmt_runs->at(1);
    assert(fmt_run->pos == 9);
    assert(fmt_run->size == 6);
    assert(!fmt_run->bold);
    assert(fmt_run->italic);
}

void test_xls_xml_colored_text()
{
    std::unique_ptr<spreadsheet::document> doc =
        load_doc(SRCDIR"/test/xls-xml/colored-text/input.xml");

    const spreadsheet::sheet* sheet1 = doc->get_sheet("ColoredText");
    assert(sheet1);

    const spreadsheet::import_shared_strings* ss = doc->get_shared_strings();
    assert(ss);

    const spreadsheet::import_styles* styles = doc->get_styles();
    assert(styles);

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
        const spreadsheet::cell_format_t* xf = styles->get_cell_format(xfi);
        assert(xf);

        const spreadsheet::font_t* font = styles->get_font(xf->font);
        assert(font);

        assert(font->color.red == c.red);
        assert(font->color.green == c.green);
        assert(font->color.blue == c.blue);

        size_t si = sheet1->get_string_identifier(c.row, 0);
        const string* s = ss->get_string(si);
        assert(s);
        assert(*s == c.text);
    }

    // Cell B2 contains mix-colored text.
    size_t si = sheet1->get_string_identifier(1, 1);
    const string* s = ss->get_string(si);
    assert(s);
    assert(*s == "Red and Blue");
    const spreadsheet::format_runs_t* fmt_runs = ss->get_format_runs(si);
    assert(fmt_runs);

    // There should be 2 segments that are color-formatted.
    assert(fmt_runs->size() == 2);

    // The 'Red' segment should be in red color.
    const spreadsheet::format_run* fmt = &fmt_runs->at(0);
    assert(fmt->color.alpha == 0);
    assert(fmt->color.red == 0xFF);
    assert(fmt->color.green == 0);
    assert(fmt->color.blue == 0);
    assert(fmt->pos == 0);
    assert(fmt->size == 3);

    // The 'Blue' segment should be in blue color.
    fmt = &fmt_runs->at(1);
    assert(fmt->color.alpha == 0);
    assert(fmt->color.red == 0);
    assert(fmt->color.green == 0x70);
    assert(fmt->color.blue == 0xC0);
    assert(fmt->pos == 8);
    assert(fmt->size == 4);
}

void test_xls_xml_column_width_row_height()
{
    struct cw_check
    {
        spreadsheet::col_t col;
        double width;
        int decimals;
    };

    struct rh_check
    {
        spreadsheet::row_t row;
        double height;
        int decimals;
    };

    std::unique_ptr<spreadsheet::document> doc =
        load_doc(SRCDIR"/test/xls-xml/column-width-row-height/input.xml");

    const spreadsheet::sheet* sheet1 = doc->get_sheet(0);
    assert(sheet1);

    // Column widths and row heights are stored in twips. Convert them to
    // points so that we can compare them with the values stored in the source
    // file.

    std::vector<cw_check> cw_checks =
    {
        { 1, 56.25, 2 },
        { 2, 82.50, 2 },
        { 3, 108.75, 2 },
        { 5, 66.75, 2 },
        { 6, 66.75, 2 },
        { 7, 66.75, 2 },
        { 10, 119.25, 2 },
        { 11, 119.25, 2 },
    };

    for (const cw_check& check : cw_checks)
    {

        spreadsheet::col_width_t cw = sheet1->get_col_width(check.col, nullptr, nullptr);
        double pt = convert(cw, length_unit_t::twip, length_unit_t::point);
        test::verify_value_to_decimals(__FILE__, __LINE__, check.width, pt, check.decimals);
    }

    std::vector<rh_check> rh_checks =
    {
        {  2, 20.0, 0 },
        {  3, 30.0, 0 },
        {  4, 40.0, 0 },
        {  5, 50.0, 0 },
        {  7, 25.0, 0 },
        {  8, 25.0, 0 },
        {  9, 25.0, 0 },
        { 12, 35.0, 0 },
        { 13, 35.0, 0 },
    };

    for (const rh_check& check : rh_checks)
    {
        spreadsheet::row_height_t rh = sheet1->get_row_height(check.row, nullptr, nullptr);
        double pt = convert(rh, length_unit_t::twip, length_unit_t::point);
        test::verify_value_to_decimals(__FILE__, __LINE__, check.height, pt, check.decimals);
    }
}

void test_xls_xml_background_fill()
{
    pstring path(SRCDIR"/test/xls-xml/background-color/standard.xml");
    std::unique_ptr<spreadsheet::document> doc = load_doc(path.str());

    spreadsheet::import_styles* styles = doc->get_styles();
    assert(styles);

    spreadsheet::sheet* sh = doc->get_sheet(0);
    assert(sh);

    struct check
    {
        spreadsheet::row_t row;
        spreadsheet::col_t col;
        pstring pattern_type;
        spreadsheet::color_t fg_color;
    };

    std::vector<check> checks =
    {
        {  1, 0, "solid", { 255, 192,   0,   0 } }, // A2  - dark red
        {  2, 0, "solid", { 255, 255,   0,   0 } }, // A3  - red
        {  3, 0, "solid", { 255, 255, 192,   0 } }, // A4  - orange
        {  4, 0, "solid", { 255, 255, 255,   0 } }, // A5  - yellow
        {  5, 0, "solid", { 255, 146, 208,  80 } }, // A6  - light green
        {  6, 0, "solid", { 255,   0, 176,  80 } }, // A7  - green
        {  7, 0, "solid", { 255,   0, 176, 240 } }, // A8  - light blue
        {  8, 0, "solid", { 255,   0, 112, 192 } }, // A9  - blue
        {  9, 0, "solid", { 255,   0,  32,  96 } }, // A10 - dark blue
        { 10, 0, "solid", { 255, 112,  48, 160 } }, // A11 - purple
    };

    for (const check& c : checks)
    {
        size_t xf = sh->get_cell_format(c.row, c.col);

        const spreadsheet::cell_format_t* cf = styles->get_cell_format(xf);
        assert(cf);

        const spreadsheet::fill_t* fill_data = styles->get_fill(cf->fill);
        assert(fill_data);
        assert(fill_data->pattern_type == c.pattern_type);
        assert(fill_data->fg_color == c.fg_color);
    }
}

void test_xls_xml_view_cursor_per_sheet()
{
    string path(SRCDIR"/test/xls-xml/view/cursor-per-sheet.xml");

    spreadsheet::document doc;
    spreadsheet::view view(doc);
    spreadsheet::import_factory factory(doc, view);
    orcus_xls_xml app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    // Sheet3 should be active.
    assert(view.get_active_sheet() == 2);

    const spreadsheet::sheet_view* sv = view.get_sheet_view(0);
    assert(sv);

    // NB : the resolver type is set to R1C1 for Excel XML 2003.
    spreadsheet::iface::import_reference_resolver* resolver = factory.get_reference_resolver();
    assert(resolver);

    // On Sheet1, the cursor should be set to C4.
    spreadsheet::range_t expected = resolver->resolve_range(ORCUS_ASCII("R4C3"));
    spreadsheet::range_t actual = sv->get_selection(spreadsheet::sheet_pane_t::top_left);
    assert(expected == actual);

    sv = view.get_sheet_view(1);
    assert(sv);

    // On Sheet2, the cursor should be set to D8.
    expected = resolver->resolve_range(ORCUS_ASCII("R8C4"));
    actual = sv->get_selection(spreadsheet::sheet_pane_t::top_left);
    assert(expected == actual);

    sv = view.get_sheet_view(2);
    assert(sv);

    // On Sheet3, the cursor should be set to D2.
    expected = resolver->resolve_range(ORCUS_ASCII("R2C4"));
    actual = sv->get_selection(spreadsheet::sheet_pane_t::top_left);
    assert(expected == actual);

    sv = view.get_sheet_view(3);
    assert(sv);

    // On Sheet4, the cursor should be set to C5:E8.
    expected = resolver->resolve_range(ORCUS_ASCII("R5C3:R8C5"));
    actual = sv->get_selection(spreadsheet::sheet_pane_t::top_left);
    assert(expected == actual);
}

struct expected_selection
{
    spreadsheet::sheet_pane_t pane;
    const char* sel;
    size_t sel_n;
};

void test_xls_xml_view_cursor_split_pane()
{
    string path(SRCDIR"/test/xls-xml/view/cursor-split-pane.xml");

    spreadsheet::document doc;
    spreadsheet::view view(doc);
    spreadsheet::import_factory factory(doc, view);
    orcus_xls_xml app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    // NB : the resolver type is set to R1C1 for Excel XML 2003.
    spreadsheet::iface::import_reference_resolver* resolver = factory.get_reference_resolver();
    assert(resolver);

    // Sheet4 should be active.
    assert(view.get_active_sheet() == 3);

    const spreadsheet::sheet_view* sv = view.get_sheet_view(0);
    assert(sv);

    // On Sheet1, the view is split into 4.
    assert(sv->get_active_pane() == spreadsheet::sheet_pane_t::bottom_left);
    assert(sv->get_split_pane().hor_split == 5190.0);
    assert(sv->get_split_pane().ver_split == 1800.0);

    {
        spreadsheet::address_t expected = resolver->resolve_address(ORCUS_ASCII("R6C6"));
        spreadsheet::address_t actual = sv->get_split_pane().top_left_cell;
        assert(expected == actual);
    }

    std::vector<expected_selection> expected_selections =
    {
        { spreadsheet::sheet_pane_t::top_left,     ORCUS_ASCII("R4C5")   },
        { spreadsheet::sheet_pane_t::top_right,    ORCUS_ASCII("R2C10")  },
        { spreadsheet::sheet_pane_t::bottom_left,  ORCUS_ASCII("R8C1")   },
        { spreadsheet::sheet_pane_t::bottom_right, ORCUS_ASCII("R17C10") },
    };

    for (const expected_selection& es : expected_selections)
    {
        // cursor in the top-left pane.
        spreadsheet::range_t expected = resolver->resolve_range(es.sel, es.sel_n);
        spreadsheet::range_t actual = sv->get_selection(es.pane);
        assert(expected == actual);
    }

    sv = view.get_sheet_view(1);
    assert(sv);

    // Sheet2 is also split into 4 views.
    assert(sv->get_active_pane() == spreadsheet::sheet_pane_t::top_right);
    assert(sv->get_split_pane().hor_split == 5190.0);
    assert(sv->get_split_pane().ver_split == 2400.0);

    {
        spreadsheet::address_t expected = resolver->resolve_address(ORCUS_ASCII("R8C6"));
        spreadsheet::address_t actual = sv->get_split_pane().top_left_cell;
        assert(expected == actual);
    }

    expected_selections =
    {
        { spreadsheet::sheet_pane_t::top_left,     ORCUS_ASCII("R2C3:R6C3")    },
        { spreadsheet::sheet_pane_t::top_right,    ORCUS_ASCII("R2C8:R2C12")   },
        { spreadsheet::sheet_pane_t::bottom_left,  ORCUS_ASCII("R18C2:R23C3")  },
        { spreadsheet::sheet_pane_t::bottom_right, ORCUS_ASCII("R11C8:R13C10") },
    };

    for (const expected_selection& es : expected_selections)
    {
        // cursor in the top-left pane.
        spreadsheet::range_t expected = resolver->resolve_range(es.sel, es.sel_n);
        spreadsheet::range_t actual = sv->get_selection(es.pane);
        assert(expected == actual);
    }

    sv = view.get_sheet_view(2);
    assert(sv);

    // Sheet3 is horizontally split into top and bottom views (top-left and bottom-left).
    assert(sv->get_active_pane() == spreadsheet::sheet_pane_t::bottom_left);
    assert(sv->get_split_pane().hor_split == 0.0);
    assert(sv->get_split_pane().ver_split == 1500.0);

    {
        spreadsheet::address_t expected = resolver->resolve_address(ORCUS_ASCII("R5C1"));
        spreadsheet::address_t actual = sv->get_split_pane().top_left_cell;
        assert(expected == actual);
    }

    expected_selections =
    {
        { spreadsheet::sheet_pane_t::top_left,     ORCUS_ASCII("R2C4") },
        { spreadsheet::sheet_pane_t::bottom_left,  ORCUS_ASCII("R9C3") },
    };

    for (const expected_selection& es : expected_selections)
    {
        // cursor in the top-left pane.
        spreadsheet::range_t expected = resolver->resolve_range(es.sel, es.sel_n);
        spreadsheet::range_t actual = sv->get_selection(es.pane);
        assert(expected == actual);
    }

    sv = view.get_sheet_view(3);
    assert(sv);

    // Sheet4 is vertically split into left and right views (top-left and top-right).
    assert(sv->get_active_pane() == spreadsheet::sheet_pane_t::top_left);
    assert(sv->get_split_pane().hor_split == 4230.0);
    assert(sv->get_split_pane().ver_split == 0.0);

    {
        spreadsheet::address_t expected = resolver->resolve_address(ORCUS_ASCII("R1C5"));
        spreadsheet::address_t actual = sv->get_split_pane().top_left_cell;
        assert(expected == actual);
    }

    expected_selections =
    {
        { spreadsheet::sheet_pane_t::top_left,  ORCUS_ASCII("R18C2") },
        { spreadsheet::sheet_pane_t::top_right, ORCUS_ASCII("R11C9") },
    };

    for (const expected_selection& es : expected_selections)
    {
        // cursor in the top-left pane.
        spreadsheet::range_t expected = resolver->resolve_range(es.sel, es.sel_n);
        spreadsheet::range_t actual = sv->get_selection(es.pane);
        assert(expected == actual);
    }
}

void test_xls_xml_view_frozen_pane()
{
    string path(SRCDIR"/test/xls-xml/view/frozen-pane.xml");

    spreadsheet::document doc;
    spreadsheet::view view(doc);
    spreadsheet::import_factory factory(doc, view);
    orcus_xls_xml app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    // NB : the resolver type is set to R1C1 for Excel XML 2003.
    spreadsheet::iface::import_reference_resolver* resolver = factory.get_reference_resolver();
    assert(resolver);

    // Sheet3 should be active.
    assert(view.get_active_sheet() == 2);

    const spreadsheet::sheet_view* sv = view.get_sheet_view(0);
    assert(sv);

    {
        // Sheet1 is vertically frozen between columns A and B.
        const spreadsheet::frozen_pane_t& fp = sv->get_frozen_pane();
        assert(fp.top_left_cell == resolver->resolve_address(ORCUS_ASCII("R1C2")));
        assert(fp.visible_columns == 1);
        assert(fp.visible_rows == 0);
        assert(sv->get_active_pane() == spreadsheet::sheet_pane_t::top_right);
    }

    sv = view.get_sheet_view(1);
    assert(sv);

    {
        // Sheet2 is horizontally frozen between rows 1 and 2.
        const spreadsheet::frozen_pane_t& fp = sv->get_frozen_pane();
        assert(fp.top_left_cell == resolver->resolve_address(ORCUS_ASCII("R2C1")));
        assert(fp.visible_columns == 0);
        assert(fp.visible_rows == 1);
        assert(sv->get_active_pane() == spreadsheet::sheet_pane_t::bottom_left);
    }

    sv = view.get_sheet_view(2);
    assert(sv);

    {
        // Sheet3 is frozen both horizontally and vertically.
        const spreadsheet::frozen_pane_t& fp = sv->get_frozen_pane();
        assert(fp.top_left_cell == resolver->resolve_address(ORCUS_ASCII("R9C5")));
        assert(fp.visible_columns == 4);
        assert(fp.visible_rows == 8);
        assert(sv->get_active_pane() == spreadsheet::sheet_pane_t::bottom_right);
    }
}

}

int main()
{
    test_config.debug = true;
    test_config.structure_check = true;

    test_xls_xml_import();
    test_xls_xml_merged_cells();
    test_xls_xml_date_time();
    test_xls_xml_bold_and_italic();
    test_xls_xml_colored_text();
    test_xls_xml_column_width_row_height();
    test_xls_xml_background_fill();

    // view import
    test_xls_xml_view_cursor_per_sheet();
    test_xls_xml_view_cursor_split_pane();
    test_xls_xml_view_frozen_pane();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

