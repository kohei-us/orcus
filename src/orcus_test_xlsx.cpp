/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_xlsx.hpp"
#include "pstring.hpp"
#include "orcus/global.hpp"
#include "orcus/stream.hpp"
#include "orcus/config.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/view.hpp"
#include "orcus/spreadsheet/sheet.hpp"
#include "orcus/spreadsheet/auto_filter.hpp"
#include "orcus/spreadsheet/pivot.hpp"
#include "orcus/spreadsheet/styles.hpp"

#include <cstdlib>
#include <cassert>
#include <string>
#include <sstream>
#include <set>
#include <cmath>
#include <vector>
#include <iostream>

#include <ixion/model_context.hpp>
#include <ixion/address.hpp>
#include <ixion/formula_name_resolver.hpp>

#include <boost/filesystem.hpp>

using namespace orcus;
using namespace orcus::spreadsheet;
namespace ss = orcus::spreadsheet;
namespace fs = boost::filesystem;

using namespace std;

namespace {

config test_config(format_t::xlsx);

std::unique_ptr<spreadsheet::document> load_doc(const pstring& path, bool recalc = true)
{
    spreadsheet::range_size_t ss{1048576, 16384};
    std::unique_ptr<spreadsheet::document> doc = std::make_unique<spreadsheet::document>(ss);
    spreadsheet::import_factory factory(*doc);
    orcus_xlsx app(&factory);
    app.read_file(path.str());
    app.set_config(test_config);
    if (recalc)
        doc->recalc_formula_cells();

    return doc;
}

/**
 * Convenience function to retrieve a pivot cache instance from textural
 * sheet name and range name.
 */
const pivot_cache* get_pivot_cache(
    const pivot_collection& pc, const pstring& sheet_name, const pstring& range_name)
{
    std::unique_ptr<ixion::formula_name_resolver> resolver =
        ixion::formula_name_resolver::get(
            ixion::formula_name_resolver_t::excel_a1, nullptr);

    if (!resolver)
        return nullptr;

    ixion::abs_address_t origin(0,0,0);

    ixion::formula_name_t fn =
        resolver->resolve({range_name.get(), range_name.size()}, origin);

    if (fn.type != ixion::formula_name_t::range_reference)
        return nullptr;

    ixion::abs_range_t range = std::get<ixion::range_t>(fn.value).to_abs(origin);
    return pc.get_cache(sheet_name, range);
}

std::vector<fs::path> dirs_recalc = {
    SRCDIR"/test/xlsx/raw-values-1",
    SRCDIR"/test/xlsx/boolean-values",
    SRCDIR"/test/xlsx/empty-shared-strings",
    SRCDIR"/test/xlsx/formula-array-1",
    SRCDIR"/test/xlsx/formula-cells",
    SRCDIR"/test/xlsx/formula-shared",
    SRCDIR"/test/xlsx/formula-with-string-results",
    SRCDIR"/test/xlsx/named-expression",
    SRCDIR"/test/xlsx/named-expression-sheet-local",
};

std::vector<fs::path> dirs_non_recalc = {
    SRCDIR"/test/xlsx/formula-array-1",
    SRCDIR"/test/xlsx/formula-cells",
    SRCDIR"/test/xlsx/formula-shared",
    SRCDIR"/test/xlsx/formula-with-string-results",
};

/**
 * Semi-automated import test that goes through all specified directories,
 * and in each directory, reads the input.xlsx file, dumps its output and
 * checks it against the check.txt content.
 */
void test_xlsx_import()
{
    auto run_check = [](const fs::path& dir, bool recalc)
    {
        // Read the input.xlsx document.
        fs::path filepath = dir / "input.xlsx";
        auto doc = load_doc(filepath.string(), recalc);

        // Dump the content of the model.
        ostringstream os;
        doc->dump_check(os);
        string check = os.str();

        // Check that against known control.
        filepath = dir / "check.txt";
        file_content control(filepath.string().data());

        assert(!check.empty());
        assert(!control.empty());

        pstring s1(&check[0], check.size());
        pstring s2 = control.str();
        assert(s1.trim() == s2.trim());
    };

    for (const fs::path& dir : dirs_recalc)
    {
        run_check(dir, true);
    }

    for (const fs::path& dir : dirs_non_recalc)
    {
        run_check(dir, false);
    }
}

void test_xlsx_table_autofilter()
{
    string path(SRCDIR"/test/xlsx/table/autofilter.xlsx");
    spreadsheet::range_size_t ss{1048576, 16384};
    document doc{ss};
    import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.read_file(path.c_str());

    const sheet* sh = doc.get_sheet(0);
    assert(sh);
    const auto_filter_t* af = sh->get_auto_filter_data();
    assert(af);

    // Autofilter is over B2:C11.
    assert(af->range.first.column == 1);
    assert(af->range.first.row == 1);
    assert(af->range.last.column == 2);
    assert(af->range.last.row == 10);

    // Check the match values of the 1st column filter criterion.
    auto_filter_t::columns_type::const_iterator it = af->columns.find(0);
    assert(it != af->columns.end());

    const auto_filter_column_t* afc = &it->second;
    assert(afc->match_values.count("A") > 0);
    assert(afc->match_values.count("C") > 0);

    // And the 2nd column.
    it = af->columns.find(1);
    assert(it != af->columns.end());
    afc = &it->second;
    assert(afc->match_values.count("1") > 0);
}

void test_xlsx_table()
{
    string path(SRCDIR"/test/xlsx/table/table-1.xlsx");
    document doc{{1048576, 16384}};
    import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.read_file(path.c_str());

    pstring name("Table1");
    const table_t* p = doc.get_table(name);
    assert(p);
    assert(p->identifier == 1);
    assert(p->name == name);
    assert(p->display_name == name);
    assert(p->totals_row_count == 1);

    // Table range is C3:D9.
    ixion::abs_range_t range;
    range.first.column = 2;
    range.first.row = 2;
    range.first.sheet = 0;
    range.last.column = 3;
    range.last.row = 8;
    range.last.sheet = 0;
    assert(p->range == range);

    // Table1 has 2 table columns.
    assert(p->columns.size() == 2);

    const table_column_t* tcol = &p->columns[0];
    assert(tcol);
    assert(tcol->identifier == 1);
    assert(tcol->name == "Category");
    assert(tcol->totals_row_label == "Total");
    assert(tcol->totals_row_function == totals_row_function_t::none);

    tcol = &p->columns[1];
    assert(tcol);
    assert(tcol->identifier == 2);
    assert(tcol->name == "Value");
    assert(tcol->totals_row_label.empty());
    assert(tcol->totals_row_function == totals_row_function_t::sum);

    const auto_filter_t& filter = p->filter;

    // Auto filter range is C3:D8.
    range.last.row = 7;
    assert(filter.range == range);

    assert(filter.columns.size() == 1);
    const auto_filter_column_t& afc = filter.columns.begin()->second;
    assert(afc.match_values.size() == 4);
    assert(afc.match_values.count("A") > 0);
    assert(afc.match_values.count("C") > 0);
    assert(afc.match_values.count("D") > 0);
    assert(afc.match_values.count("E") > 0);

    // Check table style.
    const table_style_t& style = p->style;
    assert(style.name == "TableStyleLight9");
    assert(style.show_first_column == false);
    assert(style.show_last_column == false);
    assert(style.show_row_stripes == true);
    assert(style.show_column_stripes == false);
}

void test_xlsx_merged_cells()
{
    string path(SRCDIR"/test/xlsx/merged-cells/simple.xlsx");

    spreadsheet::range_size_t ss{1048576, 16384};
    document doc{ss};
    import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const sheet* sheet1 = doc.get_sheet("Sheet1");
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

void test_xlsx_date_time()
{
    string path(SRCDIR"/test/xlsx/date-time/input.xlsx");

    document doc{{1048576, 16384}};
    import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const sheet* sheet1 = doc.get_sheet("Sheet1");
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

void test_xlsx_background_fill()
{
    pstring path(SRCDIR"/test/xlsx/background-color/standard.xlsx");
    std::unique_ptr<spreadsheet::document> doc = load_doc(path);

    spreadsheet::styles& styles = doc->get_styles();

    spreadsheet::sheet* sh = doc->get_sheet(0);
    assert(sh);

    struct check
    {
        spreadsheet::row_t row;
        spreadsheet::col_t col;
        spreadsheet::fill_pattern_t pattern_type;
        spreadsheet::color_t fg_color;
    };

    std::vector<check> checks =
    {
        {  1, 0, spreadsheet::fill_pattern_t::solid, { 255, 192,   0,   0 } }, // A2  - dark red
        {  2, 0, spreadsheet::fill_pattern_t::solid, { 255, 255,   0,   0 } }, // A3  - red
        {  3, 0, spreadsheet::fill_pattern_t::solid, { 255, 255, 192,   0 } }, // A4  - orange
        {  4, 0, spreadsheet::fill_pattern_t::solid, { 255, 255, 255,   0 } }, // A5  - yellow
        {  5, 0, spreadsheet::fill_pattern_t::solid, { 255, 146, 208,  80 } }, // A6  - light green
        {  6, 0, spreadsheet::fill_pattern_t::solid, { 255,   0, 176,  80 } }, // A7  - green
        {  7, 0, spreadsheet::fill_pattern_t::solid, { 255,   0, 176, 240 } }, // A8  - light blue
        {  8, 0, spreadsheet::fill_pattern_t::solid, { 255,   0, 112, 192 } }, // A9  - blue
        {  9, 0, spreadsheet::fill_pattern_t::solid, { 255,   0,  32,  96 } }, // A10 - dark blue
        { 10, 0, spreadsheet::fill_pattern_t::solid, { 255, 112,  48, 160 } }, // A11 - purple
    };

    for (const check& c : checks)
    {
        size_t xf = sh->get_cell_format(c.row, c.col);

        const spreadsheet::cell_format_t* cf = styles.get_cell_format(xf);
        assert(cf);

        const spreadsheet::fill_t* fill_data = styles.get_fill(cf->fill);
        assert(fill_data);
        assert(fill_data->pattern_type == c.pattern_type);
        assert(fill_data->fg_color == c.fg_color);
    }
}

void test_xlsx_number_format()
{
    pstring path(SRCDIR"/test/xlsx/number-format/date-time.xlsx");
    std::unique_ptr<spreadsheet::document> doc = load_doc(path);

    spreadsheet::sheet* sh = doc->get_sheet(0);
    assert(sh);

    struct check
    {
        ss::row_t row;
        ss::col_t col;
        pstring expected;
    };

    std::vector<check> checks =
    {
        { 1, 1, "[$-F800]dddd\\,\\ mmmm\\ dd\\,\\ yyyy" },
        { 2, 1, "[ENG][$-409]mmmm\\ d\\,\\ yyyy;@" },
        { 3, 1, "m/d/yy;@" },
    };

// TODO : We need to build our own internal number format code table for
// xlsx.  Firstly, xlsx uses numFmtId explicitly to link between the xf and
// the format string and the ID's are not sequential. Secondly, there is a
// set of built-in format strings for ID < 164, and that information is not
// stored in the file.
#if 0
    spreadsheet::styles& styles = doc->get_styles();

    for (const check& c : checks)
    {
        size_t xf = sh->get_cell_format(c.row, c.col);
        const spreadsheet::cell_format_t* cf = styles.get_cell_format(xf);
        assert(cf);

        const spreadsheet::number_format_t* nf = styles.get_number_format(cf->number_format);
        assert(nf);
        assert(nf->format_string == c.expected);
    }
#endif
}

void test_xlsx_text_alignment()
{
    pstring path(SRCDIR"/test/xlsx/text-alignment/input.xlsx");
    std::unique_ptr<spreadsheet::document> doc = load_doc(path);

    spreadsheet::styles& styles = doc->get_styles();

    spreadsheet::sheet* sh = doc->get_sheet(0);
    assert(sh);

    struct check
    {
        spreadsheet::row_t row;
        spreadsheet::col_t col;
        bool apply_align;
        spreadsheet::hor_alignment_t hor_align;
        spreadsheet::ver_alignment_t ver_align;
    };

    std::vector<check> checks =
    {
        {  1, 2, false, spreadsheet::hor_alignment_t::unknown,     spreadsheet::ver_alignment_t::unknown     }, // C2
        {  2, 2,  true, spreadsheet::hor_alignment_t::left,        spreadsheet::ver_alignment_t::bottom      }, // C3
        {  3, 2,  true, spreadsheet::hor_alignment_t::center,      spreadsheet::ver_alignment_t::bottom      }, // C4
        {  4, 2,  true, spreadsheet::hor_alignment_t::right,       spreadsheet::ver_alignment_t::bottom      }, // C5
        {  5, 2,  true, spreadsheet::hor_alignment_t::left,        spreadsheet::ver_alignment_t::bottom      }, // C6
        {  6, 2,  true, spreadsheet::hor_alignment_t::left,        spreadsheet::ver_alignment_t::bottom      }, // C7
        {  7, 2,  true, spreadsheet::hor_alignment_t::right,       spreadsheet::ver_alignment_t::bottom      }, // C8
        {  8, 2,  true, spreadsheet::hor_alignment_t::right,       spreadsheet::ver_alignment_t::bottom      }, // C9
        {  9, 2,  true, spreadsheet::hor_alignment_t::unknown,     spreadsheet::ver_alignment_t::middle      }, // C10
        { 10, 2,  true, spreadsheet::hor_alignment_t::left,        spreadsheet::ver_alignment_t::middle      }, // C11
        { 11, 2,  true, spreadsheet::hor_alignment_t::center,      spreadsheet::ver_alignment_t::middle      }, // C12
        { 12, 2,  true, spreadsheet::hor_alignment_t::right,       spreadsheet::ver_alignment_t::middle      }, // C13
        { 13, 2,  true, spreadsheet::hor_alignment_t::left,        spreadsheet::ver_alignment_t::middle      }, // C14
        { 14, 2,  true, spreadsheet::hor_alignment_t::left,        spreadsheet::ver_alignment_t::middle      }, // C15
        { 15, 2,  true, spreadsheet::hor_alignment_t::right,       spreadsheet::ver_alignment_t::middle      }, // C16
        { 16, 2,  true, spreadsheet::hor_alignment_t::right,       spreadsheet::ver_alignment_t::middle      }, // C17
        { 17, 2,  true, spreadsheet::hor_alignment_t::unknown,     spreadsheet::ver_alignment_t::top         }, // C18
        { 18, 2,  true, spreadsheet::hor_alignment_t::left,        spreadsheet::ver_alignment_t::top         }, // C19
        { 19, 2,  true, spreadsheet::hor_alignment_t::center,      spreadsheet::ver_alignment_t::top         }, // C20
        { 20, 2,  true, spreadsheet::hor_alignment_t::right,       spreadsheet::ver_alignment_t::top         }, // C21
        { 21, 2,  true, spreadsheet::hor_alignment_t::left,        spreadsheet::ver_alignment_t::top         }, // C22
        { 22, 2,  true, spreadsheet::hor_alignment_t::left,        spreadsheet::ver_alignment_t::top         }, // C23
        { 23, 2,  true, spreadsheet::hor_alignment_t::right,       spreadsheet::ver_alignment_t::top         }, // C24
        { 24, 2,  true, spreadsheet::hor_alignment_t::right,       spreadsheet::ver_alignment_t::top         }, // C25
        { 25, 2,  true, spreadsheet::hor_alignment_t::unknown,     spreadsheet::ver_alignment_t::justified   }, // C26
        { 26, 2,  true, spreadsheet::hor_alignment_t::justified,   spreadsheet::ver_alignment_t::bottom      }, // C27
        { 27, 2,  true, spreadsheet::hor_alignment_t::distributed, spreadsheet::ver_alignment_t::distributed }, // C28
    };

    for (const check& c : checks)
    {
        size_t xf = sh->get_cell_format(c.row, c.col);

        const spreadsheet::cell_format_t* cf = styles.get_cell_format(xf);
        assert(cf);
        assert(c.apply_align == cf->apply_alignment);

        if (!cf->apply_alignment)
            continue;

        assert(c.hor_align == cf->hor_align);
        assert(c.ver_align == cf->ver_align);
    }
}

void test_xlsx_cell_borders_single_cells()
{
    pstring path(SRCDIR"/test/xlsx/borders/single-cells.xlsx");
    std::unique_ptr<spreadsheet::document> doc = load_doc(path);

    spreadsheet::styles& styles = doc->get_styles();

    spreadsheet::sheet* sh = doc->get_sheet(0);
    assert(sh);

    struct check
    {
        spreadsheet::row_t row;
        spreadsheet::col_t col;
        spreadsheet::border_style_t style;
    };

    std::vector<check> checks =
    {
        {  3, 1, border_style_t::hair                },
        {  5, 1, border_style_t::dotted              },
        {  7, 1, border_style_t::dash_dot_dot        },
        {  9, 1, border_style_t::dash_dot            },
        { 11, 1, border_style_t::dashed              },
        { 13, 1, border_style_t::thin                },
        {  1, 3, border_style_t::medium_dash_dot_dot },
        {  3, 3, border_style_t::slant_dash_dot      },
        {  5, 3, border_style_t::medium_dash_dot     },
        {  7, 3, border_style_t::medium_dashed       },
        {  9, 3, border_style_t::medium              },
        { 11, 3, border_style_t::thick               },
        { 13, 3, border_style_t::double_border       },
    };

    for (const check& c : checks)
    {
        size_t xf = sh->get_cell_format(c.row, c.col);
        const spreadsheet::cell_format_t* cf = styles.get_cell_format(xf);
        assert(cf);
        assert(cf->apply_border);

        const spreadsheet::border_t* border = styles.get_border(cf->border);
        assert(border);
        assert(border->top.style    == c.style);
        assert(border->bottom.style == c.style);
        assert(border->left.style   == c.style);
        assert(border->right.style  == c.style);
    }
}

void test_xlsx_cell_borders_directions()
{
    pstring path(SRCDIR"/test/xlsx/borders/directions.xlsx");
    std::unique_ptr<spreadsheet::document> doc = load_doc(path);

    spreadsheet::styles& styles = doc->get_styles();

    spreadsheet::sheet* sh = doc->get_sheet(0);
    assert(sh);

    struct check
    {
        spreadsheet::row_t row;
        spreadsheet::col_t col;
        spreadsheet::border_direction_t dir;
    };

    std::vector<check> checks =
    {
        {  1, 1, spreadsheet::border_direction_t::top            },
        {  3, 1, spreadsheet::border_direction_t::left           },
        {  5, 1, spreadsheet::border_direction_t::right          },
        {  7, 1, spreadsheet::border_direction_t::bottom         },
        {  9, 1, spreadsheet::border_direction_t::diagonal_tl_br },
        { 11, 1, spreadsheet::border_direction_t::diagonal_bl_tr },
        { 13, 1, spreadsheet::border_direction_t::diagonal       },
    };

    for (const check& c : checks)
    {
        size_t xf = sh->get_cell_format(c.row, c.col);
        const spreadsheet::cell_format_t* cf = styles.get_cell_format(xf);
        assert(cf);
        assert(cf->apply_border);

        const spreadsheet::border_t* border = styles.get_border(cf->border);
        assert(border);

        switch (c.dir)
        {
            case spreadsheet::border_direction_t::top:
                assert(border->top.style            == spreadsheet::border_style_t::thin);
                assert(border->bottom.style         == spreadsheet::border_style_t::unknown);
                assert(border->left.style           == spreadsheet::border_style_t::unknown);
                assert(border->right.style          == spreadsheet::border_style_t::unknown);
                assert(border->diagonal.style       == spreadsheet::border_style_t::unknown);
                assert(border->diagonal_bl_tr.style == spreadsheet::border_style_t::unknown);
                assert(border->diagonal_tl_br.style == spreadsheet::border_style_t::unknown);
                break;
            case spreadsheet::border_direction_t::left:
                assert(border->top.style            == spreadsheet::border_style_t::unknown);
                assert(border->bottom.style         == spreadsheet::border_style_t::unknown);
                assert(border->left.style           == spreadsheet::border_style_t::thin);
                assert(border->right.style          == spreadsheet::border_style_t::unknown);
                assert(border->diagonal.style       == spreadsheet::border_style_t::unknown);
                assert(border->diagonal_bl_tr.style == spreadsheet::border_style_t::unknown);
                assert(border->diagonal_tl_br.style == spreadsheet::border_style_t::unknown);
                break;
            case spreadsheet::border_direction_t::right:
                assert(border->top.style            == spreadsheet::border_style_t::unknown);
                assert(border->bottom.style         == spreadsheet::border_style_t::unknown);
                assert(border->left.style           == spreadsheet::border_style_t::unknown);
                assert(border->right.style          == spreadsheet::border_style_t::thin);
                assert(border->diagonal.style       == spreadsheet::border_style_t::unknown);
                assert(border->diagonal_bl_tr.style == spreadsheet::border_style_t::unknown);
                assert(border->diagonal_tl_br.style == spreadsheet::border_style_t::unknown);
                break;
            case spreadsheet::border_direction_t::bottom:
                assert(border->top.style            == spreadsheet::border_style_t::unknown);
                assert(border->bottom.style         == spreadsheet::border_style_t::thin);
                assert(border->left.style           == spreadsheet::border_style_t::unknown);
                assert(border->right.style          == spreadsheet::border_style_t::unknown);
                assert(border->diagonal.style       == spreadsheet::border_style_t::unknown);
                assert(border->diagonal_bl_tr.style == spreadsheet::border_style_t::unknown);
                assert(border->diagonal_tl_br.style == spreadsheet::border_style_t::unknown);
                break;
            case spreadsheet::border_direction_t::diagonal:
                assert(border->top.style            == spreadsheet::border_style_t::unknown);
                assert(border->bottom.style         == spreadsheet::border_style_t::unknown);
                assert(border->left.style           == spreadsheet::border_style_t::unknown);
                assert(border->right.style          == spreadsheet::border_style_t::unknown);
                assert(border->diagonal.style       == spreadsheet::border_style_t::thin);
                assert(border->diagonal_bl_tr.style == spreadsheet::border_style_t::unknown);
                assert(border->diagonal_tl_br.style == spreadsheet::border_style_t::unknown);
                break;
            case spreadsheet::border_direction_t::diagonal_tl_br:
                assert(border->top.style            == spreadsheet::border_style_t::unknown);
                assert(border->bottom.style         == spreadsheet::border_style_t::unknown);
                assert(border->left.style           == spreadsheet::border_style_t::unknown);
                assert(border->right.style          == spreadsheet::border_style_t::unknown);
                assert(border->diagonal.style       == spreadsheet::border_style_t::unknown);
                assert(border->diagonal_bl_tr.style == spreadsheet::border_style_t::unknown);
                assert(border->diagonal_tl_br.style == spreadsheet::border_style_t::thin);
                break;
            case spreadsheet::border_direction_t::diagonal_bl_tr:
                assert(border->top.style            == spreadsheet::border_style_t::unknown);
                assert(border->bottom.style         == spreadsheet::border_style_t::unknown);
                assert(border->left.style           == spreadsheet::border_style_t::unknown);
                assert(border->right.style          == spreadsheet::border_style_t::unknown);
                assert(border->diagonal.style       == spreadsheet::border_style_t::unknown);
                assert(border->diagonal_bl_tr.style == spreadsheet::border_style_t::thin);
                assert(border->diagonal_tl_br.style == spreadsheet::border_style_t::unknown);
                break;
            default:
                assert(!"unhandled direction!");
        }
    }
}

void test_xlsx_cell_borders_colors()
{
    pstring path(SRCDIR"/test/xlsx/borders/colors.xlsx");
    std::unique_ptr<spreadsheet::document> doc = load_doc(path);

    spreadsheet::styles& styles = doc->get_styles();

    spreadsheet::sheet* sh = doc->get_sheet(0);
    assert(sh);

    struct check
    {
        spreadsheet::row_t row;
        spreadsheet::col_t col;
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

        const spreadsheet::cell_format_t* cf = styles.get_cell_format(xf);
        assert(cf);
        assert(cf->apply_border);

        const spreadsheet::border_t* border = styles.get_border(cf->border);
        assert(border);

        assert(border->left.style   == border_style_t::unknown);
        assert(border->right.style  == border_style_t::thick);
        assert(border->top.style    == border_style_t::unknown);
        assert(border->bottom.style == border_style_t::unknown);

        assert(border->right.border_color == c.color);
    }

    // B7 contains yellow left border, purple right border, and light blue
    // diagonal borders.

    size_t xf = sh->get_cell_format(6, 1); // B7

    const spreadsheet::cell_format_t* cf = styles.get_cell_format(xf);
    assert(cf);
    assert(cf->apply_border);

    const spreadsheet::border_t* border = styles.get_border(cf->border);
    assert(border);

    assert(border->left.style == border_style_t::thick);
    assert(border->left.border_color == color_t(0xFF, 0xFF, 0xFF, 0)); // yellow

    assert(border->right.style == border_style_t::thick);
    assert(border->right.border_color == color_t(0xFF, 0x70, 0x30, 0xA0)); // purple

    assert(border->diagonal.style == border_style_t::thick);
    assert(border->diagonal.border_color == color_t(0xFF, 0x00, 0xB0, 0xF0)); // light blue

    // B7 also contains multi-line string.  Test that as well.
    ixion::model_context& model = doc->get_model_context();
    ixion::string_id_t sid = model.get_string_identifier(ixion::abs_address_t(0,6,1));
    const std::string* s = model.get_string(sid);
    assert(s);
    assert(*s == "<- Yellow\nPurple ->\nLight Blue \\");
}

void test_xlsx_hidden_rows_columns()
{
    pstring path(SRCDIR"/test/xlsx/hidden-rows-columns/input.xlsx");
    std::unique_ptr<spreadsheet::document> doc = load_doc(path);

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

void test_xlsx_pivot_two_pivot_caches()
{
    string path(SRCDIR"/test/xlsx/pivot-table/two-pivot-caches.xlsx");

    document doc{{1048576, 16384}};
    import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const pivot_collection& pc = doc.get_pivot_collection();
    assert(pc.get_cache_count() == 2);

    // B2:C6 on sheet 'Data'.
    const pivot_cache* cache = get_pivot_cache(pc, "Data", "B2:C6");
    assert(cache);
    assert(cache->get_field_count() == 2);

    // Test the content of this cache.
    const pivot_cache_field_t* fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "F1");

    {
        // This field should contain 4 string items 'A', 'B', 'C' and 'D'.
        std::set<pivot_cache_item_t> expected =
        {
            pivot_cache_item_t(std::string_view{"A"}),
            pivot_cache_item_t(std::string_view{"B"}),
            pivot_cache_item_t(std::string_view{"C"}),
            pivot_cache_item_t(std::string_view{"D"}),
        };

        std::set<pivot_cache_item_t> actual(fld->items.begin(), fld->items.end());
        assert(actual == expected);
    }

    fld = cache->get_field(1);
    assert(fld);
    assert(fld->name == "D1");

    // This is a pure numeric field with min and max values specified.
    assert(fld->min_value && *fld->min_value == 1.0);
    assert(fld->max_value && *fld->max_value == 4.0);
    assert(fld->items.empty());

    {
        // Check the records.
        pivot_cache::records_type expected =
        {
            { pivot_cache_record_value_t(size_t(0)), pivot_cache_record_value_t(1.0) },
            { pivot_cache_record_value_t(size_t(1)), pivot_cache_record_value_t(2.0) },
            { pivot_cache_record_value_t(size_t(2)), pivot_cache_record_value_t(3.0) },
            { pivot_cache_record_value_t(size_t(3)), pivot_cache_record_value_t(4.0) },
        };

        assert(expected == cache->get_all_records());
    }

    // F10:G14 on the same sheet.
    cache = get_pivot_cache(pc, "Data", "F10:G14");
    assert(cache);
    assert(cache->get_field_count() == 2);

    // This field should contain 4 string items 'W', 'X', 'Y' and 'Z' but not
    // necessarily in this order.
    fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "F2");

    {
        std::set<pivot_cache_item_t> expected =
        {
            pivot_cache_item_t(std::string_view{"W"}),
            pivot_cache_item_t(std::string_view{"X"}),
            pivot_cache_item_t(std::string_view{"Y"}),
            pivot_cache_item_t(std::string_view{"Z"}),
        };

        std::set<pivot_cache_item_t> actual;
        actual.insert(fld->items.begin(), fld->items.end());
        assert(actual == expected);
    }

    fld = cache->get_field(1);
    assert(fld);
    assert(fld->name == "D2");

    // This is a pure numeric field with min and max values specified.
    assert(fld->min_value && *fld->min_value == 1.0);
    assert(fld->max_value && *fld->max_value == 4.0);
    assert(fld->items.empty());

    {
        // Check the records.
        pivot_cache::records_type expected =
        {
            { pivot_cache_record_value_t(size_t(0)), pivot_cache_record_value_t(4.0) },
            { pivot_cache_record_value_t(size_t(1)), pivot_cache_record_value_t(3.0) },
            { pivot_cache_record_value_t(size_t(2)), pivot_cache_record_value_t(2.0) },
            { pivot_cache_record_value_t(size_t(3)), pivot_cache_record_value_t(1.0) },
        };

        assert(expected == cache->get_all_records());
    }
}

void test_xlsx_pivot_mixed_type_field()
{
    string path(SRCDIR"/test/xlsx/pivot-table/mixed-type-field.xlsx");

    document doc{{1048576, 16384}};
    import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const pivot_collection& pc = doc.get_pivot_collection();
    assert(pc.get_cache_count() == 2);

    // B2:C7 on sheet 'Data'.
    const pivot_cache* cache = get_pivot_cache(pc, "Data", "B2:C7");
    assert(cache);
    assert(cache->get_field_count() == 2);

    // 1st field
    const pivot_cache_field_t* fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "F1");
    assert(fld->min_value && fld->min_value == 1.0);
    assert(fld->max_value && fld->max_value == 2.0);

    {
        // This field should contain 3 string items 'A', 'B', 'C' and 2 numeric
        // items 1 and 2.
        std::set<pivot_cache_item_t> expected =
        {
            pivot_cache_item_t(std::string_view{"A"}),
            pivot_cache_item_t(std::string_view{"B"}),
            pivot_cache_item_t(std::string_view{"C"}),
            pivot_cache_item_t(1.0),
            pivot_cache_item_t(2.0),
        };

        std::set<pivot_cache_item_t> actual(fld->items.begin(), fld->items.end());
        assert(actual == expected);
    }

    // 2nd field should be a nuemric field between 1.1 and 1.5.
    fld = cache->get_field(1);
    assert(fld);
    assert(fld->name == "V1");
    assert(fld->items.empty());

    assert(fld->min_value);
    assert(std::round(*fld->min_value * 100.0) == 110.0); // min = 1.1
    assert(fld->max_value);
    assert(std::round(*fld->max_value * 100.0) == 150.0); // max = 1.5

    {
        // Check the records.
        pivot_cache::records_type expected =
        {
            { pivot_cache_record_value_t(size_t(0)), pivot_cache_record_value_t(1.1) },
            { pivot_cache_record_value_t(size_t(1)), pivot_cache_record_value_t(1.2) },
            { pivot_cache_record_value_t(size_t(2)), pivot_cache_record_value_t(1.3) },
            { pivot_cache_record_value_t(size_t(3)), pivot_cache_record_value_t(1.4) },
            { pivot_cache_record_value_t(size_t(4)), pivot_cache_record_value_t(1.5) },
        };

        assert(expected == cache->get_all_records());
    }

    // B10:C17 on sheet 'Data'.
    cache = get_pivot_cache(pc, "Data", "B10:C17");
    assert(cache);
    assert(cache->get_field_count() == 2);

    // 1st field
    fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "F2");
    assert(fld->min_value && fld->min_value == 1.0);
    assert(fld->max_value && fld->max_value == 5.0);

    {
        // This field should contain 3 string items 'A', 'B', 'C' and 4 numeric
        // items 1, 2, 3.5 and 5.
        std::set<pivot_cache_item_t> expected =
        {
            pivot_cache_item_t(std::string_view{"A"}),
            pivot_cache_item_t(std::string_view{"B"}),
            pivot_cache_item_t(std::string_view{"C"}),
            pivot_cache_item_t(1.0),
            pivot_cache_item_t(2.0),
            pivot_cache_item_t(3.5),
            pivot_cache_item_t(5.0),
        };

        std::set<pivot_cache_item_t> actual;
        actual.insert(fld->items.begin(), fld->items.end());
        assert(actual == expected);
    }

    // 2nd field
    fld = cache->get_field(1);
    assert(fld);
    assert(fld->name == "V2");
    assert(fld->items.empty());

    assert(fld->min_value);
    assert(std::round(*fld->min_value * 100.0) == 110.0); // min = 1.1
    assert(fld->max_value);
    assert(std::round(*fld->max_value * 100.0) == 220.0); // max = 2.2

    {
        // Check the records.
        pivot_cache::records_type expected =
        {
            { pivot_cache_record_value_t(size_t(0)), pivot_cache_record_value_t(1.1) },
            { pivot_cache_record_value_t(size_t(1)), pivot_cache_record_value_t(1.2) },
            { pivot_cache_record_value_t(size_t(2)), pivot_cache_record_value_t(1.3) },
            { pivot_cache_record_value_t(size_t(3)), pivot_cache_record_value_t(1.4) },
            { pivot_cache_record_value_t(size_t(4)), pivot_cache_record_value_t(1.5) },
            { pivot_cache_record_value_t(size_t(5)), pivot_cache_record_value_t(1.8) },
            { pivot_cache_record_value_t(size_t(6)), pivot_cache_record_value_t(2.2) },
        };

        assert(expected == cache->get_all_records());
    }
}

void test_xlsx_pivot_group_field()
{
    string path(SRCDIR"/test/xlsx/pivot-table/group-field.xlsx");

    document doc{{1048576, 16384}};
    import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const pivot_collection& pc = doc.get_pivot_collection();
    assert(pc.get_cache_count() == 1);

    // B2:C6 on sheet 'Sheet1'.
    const pivot_cache* cache = get_pivot_cache(pc, "Sheet1", "B2:C6");
    assert(cache);
    assert(cache->get_field_count() == 3);

    // First field is labeled 'Key'.
    const pivot_cache_field_t* fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "Key");

    {
        // This field should contain 4 string items 'A', 'B', 'C' and 'D'.
        std::set<pivot_cache_item_t> expected =
        {
            pivot_cache_item_t(std::string_view{"A"}),
            pivot_cache_item_t(std::string_view{"B"}),
            pivot_cache_item_t(std::string_view{"C"}),
            pivot_cache_item_t(std::string_view{"D"}),
        };

        std::set<pivot_cache_item_t> actual(fld->items.begin(), fld->items.end());
        assert(actual == expected);
    }

    // 2nd field is 'Value' and is a numeric field.
    fld = cache->get_field(1);
    assert(fld);
    assert(fld->name == "Value");
    assert(fld->items.empty());

    assert(fld->min_value);
    assert(*fld->min_value == 1.0);
    assert(fld->max_value);
    assert(*fld->max_value == 4.0);

    // 3rd field is a group field labeled 'Key2'.
    fld = cache->get_field(2);
    assert(fld);
    assert(fld->name == "Key2");
    assert(fld->items.empty());

    const pivot_cache_group_data_t* gd = fld->group_data.get();
    assert(gd);
    assert(gd->base_field == 0);
    assert(gd->items.size() == 2);

    {
        // It should have two items - Group1 and Group2.
        std::set<pivot_cache_item_t> expected =
        {
            pivot_cache_item_t(std::string_view{"Group1"}),
            pivot_cache_item_t(std::string_view{"Group2"}),
        };

        std::set<pivot_cache_item_t> actual;
        actual.insert(gd->items.begin(), gd->items.end());
        assert(actual == expected);
    }

    // Group1 should group 'A' and 'B' from the 1st field, and Group2 should
    // group 'C' and 'D'.

    pivot_cache_indices_t expected_group = { 0, 0, 1, 1 };
    assert(gd->base_to_group_indices == expected_group);

    {
        // Check the records.
        pivot_cache::records_type expected =
        {
            { pivot_cache_record_value_t(size_t(0)), pivot_cache_record_value_t(1.0) },
            { pivot_cache_record_value_t(size_t(1)), pivot_cache_record_value_t(2.0) },
            { pivot_cache_record_value_t(size_t(2)), pivot_cache_record_value_t(3.0) },
            { pivot_cache_record_value_t(size_t(3)), pivot_cache_record_value_t(4.0) },
        };

        assert(expected == cache->get_all_records());
    }
}

void test_xlsx_pivot_group_by_numbers()
{
    string path(SRCDIR"/test/xlsx/pivot-table/group-by-numbers.xlsx");

    document doc{{1048576, 16384}};
    import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const pivot_collection& pc = doc.get_pivot_collection();
    assert(pc.get_cache_count() == 1);

    // B2:C13 on sheet 'Sheet1'.
    const pivot_cache* cache = get_pivot_cache(pc, "Sheet1", "B2:C13");
    assert(cache);
    assert(cache->get_field_count() == 2);

    // First field is a field with numeric grouping with intervals.
    const pivot_cache_field_t* fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "V1");

    // There should be 11 raw values ranging from 9.78E-2 to 9.82.
    assert(fld->items.size() == 11);
    assert(fld->min_value);
    assert(fld->max_value);
    assert(std::round(*fld->min_value*10000.0) == 978.00); // 9.78E-2
    assert(std::round(*fld->max_value*100.0) == 982.00);   // 9.82

    // We'll just make sure that all 11 items are of numeric type.

    for (const pivot_cache_item_t& item : fld->items)
    {
        assert(item.type == pivot_cache_item_t::item_type::numeric);
        assert(*fld->min_value <= std::get<double>(item.value));
        assert(std::get<double>(item.value) <= *fld->max_value);
    }

    // This field is also gruop field with 7 numeric intervals of width 2.
    assert(fld->group_data);
    const pivot_cache_group_data_t& grp = *fld->group_data;
    assert(grp.items.size() == 7);

    pivot_cache_items_t expected =
    {
        pivot_cache_item_t(std::string_view{"<0"}),
        pivot_cache_item_t(std::string_view{"0-2"}),
        pivot_cache_item_t(std::string_view{"2-4"}),
        pivot_cache_item_t(std::string_view{"4-6"}),
        pivot_cache_item_t(std::string_view{"6-8"}),
        pivot_cache_item_t(std::string_view{"8-10"}),
        pivot_cache_item_t(std::string_view{">10"}),
    };

    assert(grp.items == expected);

    // Check the numeric range properties.
    assert(grp.range_grouping);
    assert(grp.range_grouping->group_by == pivot_cache_group_by_t::range);
    assert(!grp.range_grouping->auto_start);
    assert(!grp.range_grouping->auto_end);
    assert(grp.range_grouping->start == 0.0);
    assert(grp.range_grouping->end == 10.0);
    assert(grp.range_grouping->interval == 2.0);

    // Test the 2nd field. This field is purely a numeric field with no
    // discrete items.

    fld = cache->get_field(1);
    assert(fld);
    assert(fld->name == "V2");
    assert(!fld->group_data);
    assert(fld->items.empty());
    assert(fld->min_value);
    assert(fld->min_value == 1.0);
    assert(fld->max_value);
    assert(fld->max_value == 11.0);
}

void test_xlsx_pivot_group_by_dates()
{
    string path(SRCDIR"/test/xlsx/pivot-table/group-by-dates.xlsx");

    document doc{{1048576, 16384}};
    import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const pivot_collection& pc = doc.get_pivot_collection();
    assert(pc.get_cache_count() == 1);

    const pivot_cache* cache = get_pivot_cache(pc, "Sheet1", "B2:C14");
    assert(cache);

    // First field is a date field.
    const pivot_cache_field_t* fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "Date");

    // Minimum and maximum date values.
    assert(fld->min_date);
    assert(*fld->min_date == date_time_t(2014, 1, 1));
    assert(fld->max_date);
    assert(*fld->max_date == date_time_t(2014, 12, 2));

    pivot_cache_items_t expected =
    {
        pivot_cache_item_t(date_time_t(2014, 1, 1)),
        pivot_cache_item_t(date_time_t(2014, 2, 1)),
        pivot_cache_item_t(date_time_t(2014, 3, 1)),
        pivot_cache_item_t(date_time_t(2014, 4, 1)),
        pivot_cache_item_t(date_time_t(2014, 5, 1)),
        pivot_cache_item_t(date_time_t(2014, 6, 1)),
        pivot_cache_item_t(date_time_t(2014, 7, 1)),
        pivot_cache_item_t(date_time_t(2014, 8, 1)),
        pivot_cache_item_t(date_time_t(2014, 9, 1)),
        pivot_cache_item_t(date_time_t(2014, 10, 1)),
        pivot_cache_item_t(date_time_t(2014, 11, 1)),
        pivot_cache_item_t(date_time_t(2014, 12, 1)),
    };

    pivot_cache_items_t actual(fld->items.begin(), fld->items.end());
    assert(actual == expected);

    // This field is grouped by month.

    assert(fld->group_data);
    const pivot_cache_group_data_t& gd = *fld->group_data;

    expected =
    {
        pivot_cache_item_t(std::string_view{"<1/1/2014"}),
        pivot_cache_item_t(std::string_view{"Jan"}),
        pivot_cache_item_t(std::string_view{"Feb"}),
        pivot_cache_item_t(std::string_view{"Mar"}),
        pivot_cache_item_t(std::string_view{"Apr"}),
        pivot_cache_item_t(std::string_view{"May"}),
        pivot_cache_item_t(std::string_view{"Jun"}),
        pivot_cache_item_t(std::string_view{"Jul"}),
        pivot_cache_item_t(std::string_view{"Aug"}),
        pivot_cache_item_t(std::string_view{"Sep"}),
        pivot_cache_item_t(std::string_view{"Oct"}),
        pivot_cache_item_t(std::string_view{"Nov"}),
        pivot_cache_item_t(std::string_view{"Dec"}),
        pivot_cache_item_t(std::string_view{">12/2/2014"}),
    };

    assert(gd.items == expected);

    assert(gd.range_grouping);
    assert(gd.range_grouping->group_by == pivot_cache_group_by_t::months);

    assert(gd.range_grouping->start_date == date_time_t(2014,1,1));
    assert(gd.range_grouping->end_date == date_time_t(2014,12,2));

    // The 2nd field is a simple numeric field.
    fld = cache->get_field(1);
    assert(fld);
    assert(fld->name == "Value");
    assert(fld->min_value == 1.0);
    assert(fld->max_value == 12.0);

    // The 3rd field is an extra group field.
    fld = cache->get_field(2);
    assert(fld);
    assert(fld->name == "Quarters");
    assert(fld->group_data);
    const pivot_cache_group_data_t& gd_qtrs = *fld->group_data;
    assert(gd_qtrs.base_field == 0);

    assert(gd_qtrs.range_grouping);
    assert(gd_qtrs.range_grouping->group_by == pivot_cache_group_by_t::quarters);
    assert(gd_qtrs.range_grouping->start_date == date_time_t(2014,1,1));
    assert(gd_qtrs.range_grouping->end_date == date_time_t(2014,12,2));

    expected =
    {
        pivot_cache_item_t(std::string_view{"<1/1/2014"}),
        pivot_cache_item_t(std::string_view{"Qtr1"}),
        pivot_cache_item_t(std::string_view{"Qtr2"}),
        pivot_cache_item_t(std::string_view{"Qtr3"}),
        pivot_cache_item_t(std::string_view{"Qtr4"}),
        pivot_cache_item_t(std::string_view{">12/2/2014"}),
    };

    assert(gd_qtrs.items == expected);
}

void test_xlsx_pivot_error_values()
{
    string path(SRCDIR"/test/xlsx/pivot-table/error-values.xlsx");

    document doc{{1048576, 16384}};
    import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const pivot_collection& pc = doc.get_pivot_collection();
    assert(pc.get_cache_count() == 1);

    const pivot_cache* cache = get_pivot_cache(pc, "Sheet1", "B2:C6");
    assert(cache);

    const pivot_cache_field_t* fld = cache->get_field(0);

    assert(fld);
    assert(fld->name == "F1");

    // This field should contain 4 string items 'A', 'B', 'C' and 'D'.
    std::set<pivot_cache_item_t> expected =
    {
        pivot_cache_item_t(std::string_view{"A"}),
        pivot_cache_item_t(std::string_view{"B"}),
        pivot_cache_item_t(std::string_view{"C"}),
        pivot_cache_item_t(std::string_view{"D"}),
    };

    std::set<pivot_cache_item_t> actual(fld->items.begin(), fld->items.end());
    assert(actual == expected);

    fld = cache->get_field(1);

    assert(fld);
    assert(fld->name == "F2");

    expected =
    {
        pivot_cache_item_t(spreadsheet::error_value_t::div0),
        pivot_cache_item_t(spreadsheet::error_value_t::name),
    };

    actual.clear();
    actual.insert(fld->items.begin(), fld->items.end());

    assert(actual == expected);
}

void test_xlsx_view_cursor_per_sheet()
{
    string path(SRCDIR"/test/xlsx/view/cursor-per-sheet.xlsx");

    document doc{{1048576, 16384}};
    spreadsheet::view view(doc);
    spreadsheet::import_factory factory(doc, view);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    // Sheet3 should be active.
    assert(view.get_active_sheet() == 2);

    const spreadsheet::sheet_view* sv = view.get_sheet_view(0);
    assert(sv);

    spreadsheet::iface::import_reference_resolver* resolver =
        factory.get_reference_resolver(spreadsheet::formula_ref_context_t::global);
    assert(resolver);

    // On Sheet1, the cursor should be set to C4.
    spreadsheet::range_t expected = to_rc_range(resolver->resolve_range("C4"));
    spreadsheet::range_t actual = sv->get_selection(spreadsheet::sheet_pane_t::top_left);
    assert(expected == actual);

    sv = view.get_sheet_view(1);
    assert(sv);

    // On Sheet2, the cursor should be set to D8.
    expected = to_rc_range(resolver->resolve_range("D8"));
    actual = sv->get_selection(spreadsheet::sheet_pane_t::top_left);
    assert(expected == actual);

    sv = view.get_sheet_view(2);
    assert(sv);

    // On Sheet3, the cursor should be set to D2.
    expected = to_rc_range(resolver->resolve_range("D2"));
    actual = sv->get_selection(spreadsheet::sheet_pane_t::top_left);
    assert(expected == actual);

    sv = view.get_sheet_view(3);
    assert(sv);

    // On Sheet4, the cursor should be set to C5:E8.
    expected = to_rc_range(resolver->resolve_range("C5:E8"));
    actual = sv->get_selection(spreadsheet::sheet_pane_t::top_left);
    assert(expected == actual);
}

struct expected_selection
{
    spreadsheet::sheet_pane_t pane;
    const char* sel;
    size_t sel_n;
};

void test_xlsx_view_cursor_split_pane()
{
    string path(SRCDIR"/test/xlsx/view/cursor-split-pane.xlsx");

    document doc{{1048576, 16384}};
    spreadsheet::view view(doc);
    spreadsheet::import_factory factory(doc, view);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    spreadsheet::iface::import_reference_resolver* resolver =
        factory.get_reference_resolver(spreadsheet::formula_ref_context_t::global);
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
        spreadsheet::address_t expected = to_rc_address(resolver->resolve_address("F6"));
        spreadsheet::address_t actual = sv->get_split_pane().top_left_cell;
        assert(expected == actual);
    }

    std::vector<expected_selection> expected_selections =
    {
        { spreadsheet::sheet_pane_t::top_left,     ORCUS_ASCII("E4")  },
        { spreadsheet::sheet_pane_t::top_right,    ORCUS_ASCII("J2")  },
        { spreadsheet::sheet_pane_t::bottom_left,  ORCUS_ASCII("A8")  },
        { spreadsheet::sheet_pane_t::bottom_right, ORCUS_ASCII("J17") },
    };

    for (const expected_selection& es : expected_selections)
    {
        // cursor in the top-left pane.
        spreadsheet::range_t expected = to_rc_range(resolver->resolve_range({es.sel, es.sel_n}));
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
        spreadsheet::address_t expected = to_rc_address(resolver->resolve_address("F8"));
        spreadsheet::address_t actual = sv->get_split_pane().top_left_cell;
        assert(expected == actual);
    }

    expected_selections =
    {
        { spreadsheet::sheet_pane_t::top_left,     ORCUS_ASCII("C2:C6")   },
        { spreadsheet::sheet_pane_t::top_right,    ORCUS_ASCII("H2:L2")   },
        { spreadsheet::sheet_pane_t::bottom_left,  ORCUS_ASCII("B18:C23") },
        { spreadsheet::sheet_pane_t::bottom_right, ORCUS_ASCII("H11:J13") },
    };

    for (const expected_selection& es : expected_selections)
    {
        // cursor in the top-left pane.
        spreadsheet::range_t expected = to_rc_range(resolver->resolve_range({es.sel, es.sel_n}));
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
        spreadsheet::address_t expected = to_rc_address(resolver->resolve_address("A5"));
        spreadsheet::address_t actual = sv->get_split_pane().top_left_cell;
        assert(expected == actual);
    }

    expected_selections =
    {
        { spreadsheet::sheet_pane_t::top_left,     ORCUS_ASCII("D2") },
        { spreadsheet::sheet_pane_t::bottom_left,  ORCUS_ASCII("C9") },
    };

    for (const expected_selection& es : expected_selections)
    {
        // cursor in the top-left pane.
        spreadsheet::range_t expected = to_rc_range(resolver->resolve_range({es.sel, es.sel_n}));
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
        spreadsheet::address_t expected = to_rc_address(resolver->resolve_address("E1"));
        spreadsheet::address_t actual = sv->get_split_pane().top_left_cell;
        assert(expected == actual);
    }

    expected_selections =
    {
        { spreadsheet::sheet_pane_t::top_left,  ORCUS_ASCII("B18") },
        { spreadsheet::sheet_pane_t::top_right, ORCUS_ASCII("I11") },
    };

    for (const expected_selection& es : expected_selections)
    {
        // cursor in the top-left pane.
        spreadsheet::range_t expected = to_rc_range(resolver->resolve_range({es.sel, es.sel_n}));
        spreadsheet::range_t actual = sv->get_selection(es.pane);
        assert(expected == actual);
    }
}

void test_xlsx_view_frozen_pane()
{
    string path(SRCDIR"/test/xlsx/view/frozen-pane.xlsx");

    document doc{{1048576, 16384}};
    spreadsheet::view view(doc);
    spreadsheet::import_factory factory(doc, view);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    spreadsheet::iface::import_reference_resolver* resolver =
        factory.get_reference_resolver(spreadsheet::formula_ref_context_t::global);
    assert(resolver);

    // Sheet3 should be active.
    assert(view.get_active_sheet() == 2);

    const spreadsheet::sheet_view* sv = view.get_sheet_view(0);
    assert(sv);

    {
        // Sheet1 is vertically frozen between columns A and B.
        const spreadsheet::frozen_pane_t& fp = sv->get_frozen_pane();
        assert(fp.top_left_cell == to_rc_address(resolver->resolve_address("B1")));
        assert(fp.visible_columns == 1);
        assert(fp.visible_rows == 0);
        assert(sv->get_active_pane() == spreadsheet::sheet_pane_t::top_right);
    }

    sv = view.get_sheet_view(1);
    assert(sv);

    {
        // Sheet2 is horizontally frozen between rows 1 and 2.
        const spreadsheet::frozen_pane_t& fp = sv->get_frozen_pane();
        assert(fp.top_left_cell == to_rc_address(resolver->resolve_address("A2")));
        assert(fp.visible_columns == 0);
        assert(fp.visible_rows == 1);
        assert(sv->get_active_pane() == spreadsheet::sheet_pane_t::bottom_left);
    }

    sv = view.get_sheet_view(2);
    assert(sv);

    {
        // Sheet3 is frozen both horizontally and vertically.
        const spreadsheet::frozen_pane_t& fp = sv->get_frozen_pane();
        assert(fp.top_left_cell == to_rc_address(resolver->resolve_address("E9")));
        assert(fp.visible_columns == 4);
        assert(fp.visible_rows == 8);
        assert(sv->get_active_pane() == spreadsheet::sheet_pane_t::bottom_right);
    }
}

void test_xlsx_doc_structure_unordered_sheet_positions()
{
    pstring path(SRCDIR"/test/xlsx/doc-structure/unordered-sheet-positions.xlsx");
    std::unique_ptr<spreadsheet::document> doc = load_doc(path);

    // There should be 9 sheets named S1, S2, ..., S9.
    std::vector<pstring> expected_sheet_names = {
        "S1", "S2", "S3", "S4", "S5", "S6", "S7", "S8", "S9"
    };

    assert(doc->get_sheet_count() == expected_sheet_names.size());

    sheet_t n = expected_sheet_names.size();
    for (sheet_t i = 0; i < n; ++i)
    {
        pstring sheet_name = doc->get_sheet_name(i);
        assert(sheet_name == expected_sheet_names[i]);
    }
}

}

int main()
{
    test_config.debug = true;
    test_config.structure_check = true;

    test_xlsx_import();
    test_xlsx_table_autofilter();
    test_xlsx_table();
    test_xlsx_merged_cells();
    test_xlsx_date_time();
    test_xlsx_background_fill();
    test_xlsx_number_format();
    test_xlsx_text_alignment();
    test_xlsx_cell_borders_single_cells();
    test_xlsx_cell_borders_directions();
    test_xlsx_cell_borders_colors();
    test_xlsx_hidden_rows_columns();

    // pivot table
    test_xlsx_pivot_two_pivot_caches();
    test_xlsx_pivot_mixed_type_field();
    test_xlsx_pivot_group_field();
    test_xlsx_pivot_group_by_numbers();
    test_xlsx_pivot_group_by_dates();
    test_xlsx_pivot_error_values();

    // view import
    test_xlsx_view_cursor_per_sheet();
    test_xlsx_view_cursor_split_pane();
    test_xlsx_view_frozen_pane();

    // document structure
    test_xlsx_doc_structure_unordered_sheet_positions();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
