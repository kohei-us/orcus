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
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/sheet.hpp"
#include "orcus/spreadsheet/shared_strings.hpp"
#include "orcus/spreadsheet/styles.hpp"

#include <cassert>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <iostream>

using namespace orcus;
using namespace std;

namespace {

std::vector<const char*> dirs = {
    SRCDIR"/test/xls-xml/basic/",
    SRCDIR"/test/xls-xml/bold-and-italic/",
    SRCDIR"/test/xls-xml/empty-rows/",
    SRCDIR"/test/xls-xml/merged-cells/",
    SRCDIR"/test/xls-xml/named-expression/",
    SRCDIR"/test/xls-xml/named-expression-sheet-local/",
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

}

int main()
{
    test_xls_xml_import();
    test_xls_xml_merged_cells();
    test_xls_xml_date_time();
    test_xls_xml_bold_and_italic();

    return EXIT_SUCCESS;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
