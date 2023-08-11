/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include "orcus/orcus_xlsx.hpp"
#include "orcus/stream.hpp"
#include "orcus/config.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/view.hpp"
#include "orcus/spreadsheet/sheet.hpp"
#include "orcus/spreadsheet/auto_filter.hpp"
#include "orcus/spreadsheet/pivot.hpp"
#include "orcus/spreadsheet/styles.hpp"
#include <orcus/parser_global.hpp>

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

#ifdef HAVE_FILESYSTEM
#include <filesystem>
namespace fs = std::filesystem;
#else
#ifdef HAVE_EXPERIMENTAL_FILESYSTEM
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#endif
#endif

using namespace orcus;
namespace ss = orcus::spreadsheet;

namespace {

config test_config(format_t::xlsx);

std::unique_ptr<spreadsheet::document> load_doc(const std::string_view& path, bool recalc = true)
{
    spreadsheet::range_size_t ss{1048576, 16384};
    std::unique_ptr<spreadsheet::document> doc = std::make_unique<spreadsheet::document>(ss);
    spreadsheet::import_factory factory(*doc);
    orcus_xlsx app(&factory);
    app.read_file(std::string{path});
    app.set_config(test_config);
    if (recalc)
        doc->recalc_formula_cells();

    return doc;
}

/**
 * Convenience function to retrieve a pivot cache instance from textural
 * sheet name and range name.
 */
const ss::pivot_cache* get_pivot_cache(
    const ss::pivot_collection& pc, const std::string_view& sheet_name, std::string_view range_name)
{
    std::unique_ptr<ixion::formula_name_resolver> resolver =
        ixion::formula_name_resolver::get(
            ixion::formula_name_resolver_t::excel_a1, nullptr);

    if (!resolver)
        return nullptr;

    ixion::abs_address_t origin(0,0,0);

    ixion::formula_name_t fn =
        resolver->resolve(range_name, origin);

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
    test::stack_printer __sp__(__func__);

    auto run_check = [](const fs::path& dir, bool recalc)
    {
        // Read the input.xlsx document.
        fs::path filepath = dir / "input.xlsx";
        auto doc = load_doc(filepath.string(), recalc);

        // Dump the content of the model.
        std::ostringstream os;
        doc->dump_check(os);
        std::string check = os.str();

        // Check that against known control.
        filepath = dir / "check.txt";
        file_content control(filepath.string().data());

        assert(!check.empty());
        assert(!control.empty());

        std::string_view s1(&check[0], check.size());
        std::string_view s2 = control.str();
        assert(orcus::trim(s1) == orcus::trim(s2));
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
    test::stack_printer __sp__(__func__);

    std::string path(SRCDIR"/test/xlsx/table/autofilter.xlsx");
    spreadsheet::range_size_t ss{1048576, 16384};
    ss::document doc{ss};
    ss::import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.read_file(path.c_str());

    const ss::sheet* sh = doc.get_sheet(0);
    assert(sh);
    const ss::auto_filter_t* af = sh->get_auto_filter_data();
    assert(af);

    // Autofilter is over B2:C11.
    assert(af->range.first.column == 1);
    assert(af->range.first.row == 1);
    assert(af->range.last.column == 2);
    assert(af->range.last.row == 10);

    // Check the match values of the 1st column filter criterion.
    auto it = af->columns.find(0);
    assert(it != af->columns.end());

    const ss::auto_filter_column_t* afc = &it->second;
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
    test::stack_printer __sp__(__func__);

    std::string path(SRCDIR"/test/xlsx/table/table-1.xlsx");
    ss::document doc{{1048576, 16384}};
    ss::import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.read_file(path.c_str());

    std::string_view name("Table1");
    const ss::table_t* p = doc.get_table(name);
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

    const ss::table_column_t* tcol = &p->columns[0];
    assert(tcol);
    assert(tcol->identifier == 1);
    assert(tcol->name == "Category");
    assert(tcol->totals_row_label == "Total");
    assert(tcol->totals_row_function == ss::totals_row_function_t::none);

    tcol = &p->columns[1];
    assert(tcol);
    assert(tcol->identifier == 2);
    assert(tcol->name == "Value");
    assert(tcol->totals_row_label.empty());
    assert(tcol->totals_row_function == ss::totals_row_function_t::sum);

    const auto& filter = p->filter;

    // Auto filter range is C3:D8.
    range.last.row = 7;
    assert(filter.range == range);

    assert(filter.columns.size() == 1);
    const ss::auto_filter_column_t& afc = filter.columns.begin()->second;
    assert(afc.match_values.size() == 4);
    assert(afc.match_values.count("A") > 0);
    assert(afc.match_values.count("C") > 0);
    assert(afc.match_values.count("D") > 0);
    assert(afc.match_values.count("E") > 0);

    // Check table style.
    const ss::table_style_t& style = p->style;
    assert(style.name == "TableStyleLight9");
    assert(style.show_first_column == false);
    assert(style.show_last_column == false);
    assert(style.show_row_stripes == true);
    assert(style.show_column_stripes == false);
}

void test_xlsx_merged_cells()
{
    test::stack_printer __sp__(__func__);

    std::string path(SRCDIR"/test/xlsx/merged-cells/simple.xlsx");

    spreadsheet::range_size_t ss{1048576, 16384};
    ss::document doc{ss};
    ss::import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const ss::sheet* sheet1 = doc.get_sheet("Sheet1");
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
    test::stack_printer __sp__(__func__);

    std::string path(SRCDIR"/test/xlsx/date-time/input.xlsx");

    ss::document doc{{1048576, 16384}};
    ss::import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const ss::sheet* sheet1 = doc.get_sheet("Sheet1");
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
    test::stack_printer __sp__(__func__);

    std::string_view path(SRCDIR"/test/xlsx/background-color/standard.xlsx");
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
    test::stack_printer __sp__(__func__);

    std::string_view path(SRCDIR"/test/xlsx/number-format/date-time.xlsx");
    std::unique_ptr<spreadsheet::document> doc = load_doc(path);

    spreadsheet::sheet* sh = doc->get_sheet(0);
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

        const spreadsheet::number_format_t* nf = styles.start_number_format(cf->number_format);
        assert(nf);
        assert(nf->format_string == c.expected);
    }
#endif
}

void test_xlsx_text_alignment()
{
    test::stack_printer __sp__(__func__);

    std::string_view path(SRCDIR"/test/xlsx/text-alignment/input.xlsx");
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
    test::stack_printer __sp__(__func__);

    std::string_view path(SRCDIR"/test/xlsx/borders/single-cells.xlsx");
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
    test::stack_printer __sp__(__func__);

    std::string_view path(SRCDIR"/test/xlsx/borders/directions.xlsx");
    std::unique_ptr<ss::document> doc = load_doc(path);

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
                assert(border->top.style == ss::border_style_t::thin);
                assert(!border->bottom.style);
                assert(!border->left.style);
                assert(!border->right.style);
                assert(!border->diagonal.style);
                assert(!border->diagonal_bl_tr.style);
                assert(!border->diagonal_tl_br.style);
                break;
            case ss::border_direction_t::left:
                assert(!border->top.style);
                assert(!border->bottom.style);
                assert(border->left.style);
                assert(*border->left.style == ss::border_style_t::thin);
                assert(!border->right.style);
                assert(!border->diagonal.style);
                assert(!border->diagonal_bl_tr.style);
                assert(!border->diagonal_tl_br.style);
                break;
            case ss::border_direction_t::right:
                assert(!border->top.style);
                assert(!border->bottom.style);
                assert(!border->left.style);
                assert(border->right.style);
                assert(*border->right.style == ss::border_style_t::thin);
                assert(!border->diagonal.style);
                assert(!border->diagonal_bl_tr.style);
                assert(!border->diagonal_tl_br.style);
                break;
            case ss::border_direction_t::bottom:
                assert(!border->top.style);
                assert(border->bottom.style);
                assert(*border->bottom.style == ss::border_style_t::thin);
                assert(!border->left.style);
                assert(!border->right.style);
                assert(!border->diagonal.style);
                assert(!border->diagonal_bl_tr.style);
                assert(!border->diagonal_tl_br.style);
                break;
            case ss::border_direction_t::diagonal:
                assert(!border->top.style);
                assert(!border->bottom.style);
                assert(!border->left.style);
                assert(!border->right.style);
                assert(border->diagonal.style);
                assert(*border->diagonal.style == ss::border_style_t::thin);
                assert(!border->diagonal_bl_tr.style);
                assert(!border->diagonal_tl_br.style);
                break;
            case ss::border_direction_t::diagonal_tl_br:
                assert(!border->top.style);
                assert(!border->bottom.style);
                assert(!border->left.style);
                assert(!border->right.style);
                assert(!border->diagonal.style);
                assert(!border->diagonal_bl_tr.style);
                assert(border->diagonal_tl_br.style);
                assert(border->diagonal_tl_br.style == ss::border_style_t::thin);
                break;
            case ss::border_direction_t::diagonal_bl_tr:
                assert(!border->top.style);
                assert(!border->bottom.style);
                assert(!border->left.style);
                assert(!border->right.style);
                assert(!border->diagonal.style);
                assert(border->diagonal_bl_tr.style);
                assert(*border->diagonal_bl_tr.style == ss::border_style_t::thin);
                assert(!border->diagonal_tl_br.style);
                break;
            default:
                assert(!"unhandled direction!");
        }
    }
}

void test_xlsx_cell_borders_colors()
{
    test::stack_printer __sp__(__func__);

    std::string_view path(SRCDIR"/test/xlsx/borders/colors.xlsx");
    std::unique_ptr<spreadsheet::document> doc = load_doc(path);

    spreadsheet::styles& styles = doc->get_styles();

    spreadsheet::sheet* sh = doc->get_sheet(0);
    assert(sh);

    struct check
    {
        ss::row_t row;
        ss::col_t col;
        ss::color_t color;
    };

    std::vector<check> checks =
    {
        { 2, 1, ss::color_t(0xFF, 0xFF,    0,    0) }, // B3 - red
        { 3, 1, ss::color_t(0xFF,    0, 0x70, 0xC0) }, // B4 - blue
        { 4, 1, ss::color_t(0xFF,    0, 0xB0, 0x50) }, // B5 - green
    };

    for (const check& c : checks)
    {
        size_t xf = sh->get_cell_format(c.row, c.col); // B3

        const spreadsheet::cell_format_t* cf = styles.get_cell_format(xf);
        assert(cf);
        assert(cf->apply_border);

        const spreadsheet::border_t* border = styles.get_border(cf->border);
        assert(border);

        assert(!border->left.style);
        assert(border->right.style);
        assert(*border->right.style == ss::border_style_t::thick);
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

    assert(border->left.style == ss::border_style_t::thick);
    assert(border->left.border_color == ss::color_t(0xFF, 0xFF, 0xFF, 0)); // yellow

    assert(border->right.style == ss::border_style_t::thick);
    assert(border->right.border_color == ss::color_t(0xFF, 0x70, 0x30, 0xA0)); // purple

    assert(border->diagonal.style == ss::border_style_t::thick);
    assert(border->diagonal.border_color == ss::color_t(0xFF, 0x00, 0xB0, 0xF0)); // light blue

    // B7 also contains multi-line string.  Test that as well.
    ixion::model_context& model = doc->get_model_context();
    ixion::string_id_t sid = model.get_string_identifier(ixion::abs_address_t(0,6,1));
    const std::string* s = model.get_string(sid);
    assert(s);
    assert(*s == "<- Yellow\nPurple ->\nLight Blue \\");
}

void test_xlsx_hidden_rows_columns()
{
    test::stack_printer __sp__(__func__);

    std::string_view path(SRCDIR"/test/xlsx/hidden-rows-columns/input.xlsx");
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

void test_xlsx_cell_properties()
{
    test::stack_printer __sp__(__func__);

    fs::path path{SRCDIR"/test/xlsx/cell-properties/wrap-and-shrink.xlsx"};
    std::unique_ptr<spreadsheet::document> doc = load_doc(path.string());

    const ss::styles& styles = doc->get_styles();
    const ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

    auto xfid = sh->get_cell_format(0, 1); // B1
    const auto* xf = styles.get_cell_format(xfid);
    assert(xf);
    assert(!xf->wrap_text);
    assert(!xf->shrink_to_fit);

    xfid = sh->get_cell_format(1, 1); // B2
    xf = styles.get_cell_format(xfid);
    assert(xf);
    assert(xf->wrap_text);
    assert(*xf->wrap_text);
    assert(xf->shrink_to_fit);
    assert(!*xf->shrink_to_fit);

    xfid = sh->get_cell_format(2, 1); // B3
    xf = styles.get_cell_format(xfid);
    assert(xf);
    assert(xf->wrap_text);
    assert(!*xf->wrap_text);
    assert(xf->shrink_to_fit);
    assert(*xf->shrink_to_fit);
}

void test_xlsx_styles_direct_format()
{
    test::stack_printer __sp__(__func__);

    fs::path path{SRCDIR"/test/xlsx/styles/direct-format.xlsx"};
    std::unique_ptr<spreadsheet::document> doc = load_doc(path.string());
    assert(doc);

    const auto& model = doc->get_model_context();

    {
        // Check cell string values first.

        struct check_type
        {
            ixion::abs_address_t address;
            std::string_view str;
        };

        const check_type checks[] = {
            // sheet, row, column, expected cell string value
            { { 0, 1, 1 }, "Bold and underlined" },
            { { 0, 3, 1 }, "Yellow background\nand\nright aligned" },
            { { 0, 5, 3 }, "Named Format (Good)" },
            { { 0, 7, 3 }, "Named Format (Good) plus direct format on top" },
        };

        for (const auto& c : checks)
        {
            ixion::string_id_t sid = model.get_string_identifier(c.address);
            const std::string* s = model.get_string(sid);
            assert(s);
            assert(*s == c.str);
        }
    }

    const ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

    const ss::styles& styles = doc->get_styles();

    // Text in B2 is bold, underlined, and horizontally and vertically centered.
    auto xfid = sh->get_cell_format(1, 1);
    const ss::cell_format_t* xf = styles.get_cell_format(xfid);
    assert(xf);

    const ss::font_t* font = styles.get_font(xf->font);
    assert(font);
    assert(font->bold);
    assert(*font->bold);

    const ss::border_t* border = styles.get_border(xf->border);
    assert(border);

    // "Continuous" with a weight of 1 is mapped to 'thin' border style.
    assert(border->bottom.style);
    assert(*border->bottom.style == ss::border_style_t::thin);

    assert(xf->hor_align == ss::hor_alignment_t::center);
    assert(xf->ver_align == ss::ver_alignment_t::middle);

    // B4 has yellow background, has "Calibri" font at 14 pt etc
    xfid = sh->get_cell_format(3, 1);

    xf = styles.get_cell_format(xfid);
    assert(xf);

    font = styles.get_font(xf->font);
    assert(font);
    assert(font->name);
    assert(*font->name == "Calibri");
    assert(font->size);
    assert(*font->size == 14.0);
#if 0
    // TODO: xlsx stores this color as a theme index which we don't yet support
    assert(font->first.color == ss::color_t(0xFF, 0x37, 0x56, 0x23));
    assert(font->second.color);
#endif

    // B4 has yellow background
    const ss::fill_t* fill = styles.get_fill(xf->fill);
    assert(fill);
    assert(fill->pattern_type);
    assert(*fill->pattern_type == ss::fill_pattern_t::solid);
    assert(fill->fg_color);
    assert(*fill->fg_color == ss::color_t(0xFF, 0xFF, 0xFF, 0x00));

    // B4 is horizontally right-aligned and vertically bottom-aligned
    assert(xf->hor_align == ss::hor_alignment_t::right);
    assert(xf->ver_align == ss::ver_alignment_t::bottom);

    // B4 has wrap text on
    assert(xf->wrap_text && *xf->wrap_text);

    // D6 only uses "Good" named cell style with no direct formatting
    xfid = sh->get_cell_format(5, 3);
    xf = styles.get_cell_format(xfid);
    assert(xf);

    const auto xfid_style_good = xf->style_xf;
    const ss::cell_style_t* xstyle = styles.get_cell_style_by_xf(xf->style_xf);
    assert(xstyle);
    assert(xstyle->name == "Good");

    // Check the format detail of the "Good" style
    xf = styles.get_cell_style_format(xstyle->xf);
    assert(xf);

    font = styles.get_font(xf->font);
    assert(font);
    assert(font->name);
    assert(*font->name == "Calibri");
    assert(font->size);
    assert(*font->size == 11.0);
    assert(font->color);
    assert(*font->color == ss::color_t(0xFF, 0x00, 0x61, 0x00));

    fill = styles.get_fill(xf->fill);
    assert(fill);
    assert(fill->pattern_type);
    assert(*fill->pattern_type == ss::fill_pattern_t::solid);
    assert(fill->fg_color);
    assert(*fill->fg_color == ss::color_t(0xFF, 0xC6, 0xEF, 0xCE));

    // D8 has some direct formats applied on top of "Good" named style
    xfid = sh->get_cell_format(7, 3);
    xf = styles.get_cell_format(xfid);
    assert(xf);

    // Make sure it has the "Good" style as its basis
    assert(xf->style_xf == xfid_style_good);
    xstyle = styles.get_cell_style_by_xf(xf->style_xf);
    assert(xstyle);
    assert(xstyle->name == "Good");

    // Format directly applied to D8 on top of "Good" style
    assert(xf->hor_align == ss::hor_alignment_t::center);
    assert(xf->ver_align == ss::ver_alignment_t::bottom);
    assert(xf->wrap_text);
    assert(*xf->wrap_text);
    font = styles.get_font(xf->font);
    assert(font);
    assert(font->bold);
    assert(*font->bold);
}

void test_xlsx_styles_column_styles()
{
    test::stack_printer __sp__(__func__);

    fs::path path{SRCDIR"/test/xlsx/styles/column-styles.xlsx"};
    std::unique_ptr<spreadsheet::document> doc = load_doc(path.string());
    assert(doc);

    auto doc_size = doc->get_sheet_size();

    const ss::styles& styles = doc->get_styles();

    const ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

    {
        // On Sheet1, check the named styles applied on columns B:D and F.
        // Columns A and E should have Normal style applied.
        const std::tuple<ss::row_t, ss::col_t, std::string> checks[] = {
            { 0, 0, "Normal" },
            { 0, 1, "Bad" },
            { 0, 2, "Good" },
            { 0, 3, "Neutral" },
            { 0, 4, "Normal" },
            { 0, 5, "Note" },
            { doc_size.rows - 1, 0, "Normal" },
            { doc_size.rows - 1, 1, "Bad" },
            { doc_size.rows - 1, 2, "Good" },
            { doc_size.rows - 1, 3, "Neutral" },
            { doc_size.rows - 1, 4, "Normal" },
            { doc_size.rows - 1, 5, "Note" },
        };

        for (const auto& check : checks)
        {
            ss::row_t r = std::get<0>(check);
            ss::col_t c = std::get<1>(check);
            std::string_view name = std::get<2>(check);

            std::size_t xfid = sh->get_cell_format(r, c);
            std::cout << "row=" << r << "; column=" << c << "; xfid=" << xfid << std::endl;
            const ss::cell_format_t* xf = styles.get_cell_format(xfid);
            assert(xf);
            std::cout << "style xfid=" << xf->style_xf << std::endl;

            const ss::cell_style_t* xstyle = styles.get_cell_style_by_xf(xf->style_xf);
            assert(xstyle);
            if (xstyle->name != name)
                std::cout << "names differ! (expected=" << name << "; actual=" << xstyle->name << ")" << std::endl;

            assert(xstyle->name == name);
        }
    }

    {
        // Row 10 has green background, and row 11 has orange background.
        const std::tuple<ss::row_t, ss::color_t> checks[] = {
            { 9, {0xFF, 0x92, 0xD0, 0x50} },
            { 10, {0xFF, 0xFF, 0xC0, 0x00} },
        };

        for (const auto& check : checks)
        {
            const ss::row_t row = std::get<0>(check);
            const ss::color_t color = std::get<1>(check);

            for (ss::col_t col = 0; col <= 6; ++col)
            {
                std::size_t xfid = sh->get_cell_format(row, col);
                std::cout << "row=" << row << "; column=" << col << "; xfid=" << xfid << std::endl;
                const ss::cell_format_t* xf = styles.get_cell_format(xfid);
                assert(xf);

                const ss::fill_t* fill = styles.get_fill(xf->fill);
                assert(fill);

                assert(fill->pattern_type);
                assert(*fill->pattern_type == ss::fill_pattern_t::solid);

                assert(fill->fg_color);
                assert(*fill->fg_color == color);
            }
        }
    }

    sh = doc->get_sheet(1);
    assert(sh);

    // Columns B:D should have "Good" named style applied.
    {
        const std::pair<ss::row_t, ss::col_t> cells[] = {
            { 0, 1 },
            { 0, 3 },
            { doc_size.rows - 1, 1 },
            { doc_size.rows - 1, 3 },
        };

        for (const auto& cell : cells)
        {
            std::size_t xfid = sh->get_cell_format(cell.first, cell.second);
            const ss::cell_format_t* xf = styles.get_cell_format(xfid);
            assert(xf);

            const ss::cell_style_t* xstyle = styles.get_cell_style_by_xf(xf->style_xf);
            assert(xstyle);
            assert(xstyle->name == "Good");
        }
    }
}

void test_xlsx_pivot_two_pivot_caches()
{
    test::stack_printer __sp__(__func__);

    std::string path(SRCDIR"/test/xlsx/pivot-table/two-pivot-caches.xlsx");

    ss::document doc{{1048576, 16384}};
    ss::import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const ss::pivot_collection& pc = doc.get_pivot_collection();
    assert(pc.get_cache_count() == 2);

    // B2:C6 on sheet 'Data'.
    const ss::pivot_cache* cache = get_pivot_cache(pc, "Data", "B2:C6");
    assert(cache);
    assert(cache->get_field_count() == 2);

    // Test the content of this cache.
    const ss::pivot_cache_field_t* fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "F1");

    {
        // This field should contain 4 string items 'A', 'B', 'C' and 'D'.
        std::set<ss::pivot_cache_item_t> expected =
        {
            std::string_view{"A"},
            std::string_view{"B"},
            std::string_view{"C"},
            std::string_view{"D"},
        };

        std::set<ss::pivot_cache_item_t> actual(fld->items.begin(), fld->items.end());
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
        ss::pivot_cache::records_type expected =
        {
            { std::size_t(0), 1.0 },
            { std::size_t(1), 2.0 },
            { std::size_t(2), 3.0 },
            { std::size_t(3), 4.0 },
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
        std::set<ss::pivot_cache_item_t> expected =
        {
            std::string_view{"W"},
            std::string_view{"X"},
            std::string_view{"Y"},
            std::string_view{"Z"},
        };

        std::set<ss::pivot_cache_item_t> actual;
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
        ss::pivot_cache::records_type expected =
        {
            { std::size_t(0), 4.0 },
            { std::size_t(1), 3.0 },
            { std::size_t(2), 2.0 },
            { std::size_t(3), 1.0 },
        };

        assert(expected == cache->get_all_records());
    }
}

void test_xlsx_pivot_mixed_type_field()
{
    test::stack_printer __sp__(__func__);

    std::string path(SRCDIR"/test/xlsx/pivot-table/mixed-type-field.xlsx");

    ss::document doc{{1048576, 16384}};
    ss::import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const ss::pivot_collection& pc = doc.get_pivot_collection();
    assert(pc.get_cache_count() == 2);

    // B2:C7 on sheet 'Data'.
    const ss::pivot_cache* cache = get_pivot_cache(pc, "Data", "B2:C7");
    assert(cache);
    assert(cache->get_field_count() == 2);

    // 1st field
    const ss::pivot_cache_field_t* fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "F1");
    assert(fld->min_value && fld->min_value == 1.0);
    assert(fld->max_value && fld->max_value == 2.0);

    {
        // This field should contain 3 string items 'A', 'B', 'C' and 2 numeric
        // items 1 and 2.
        std::set<ss::pivot_cache_item_t> expected =
        {
            std::string_view{"A"},
            std::string_view{"B"},
            std::string_view{"C"},
            1.0,
            2.0,
        };

        std::set<ss::pivot_cache_item_t> actual(fld->items.begin(), fld->items.end());
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
        ss::pivot_cache::records_type expected =
        {
            { size_t(0), 1.1 },
            { size_t(1), 1.2 },
            { size_t(2), 1.3 },
            { size_t(3), 1.4 },
            { size_t(4), 1.5 },
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
        std::set<ss::pivot_cache_item_t> expected =
        {
            std::string_view{"A"},
            std::string_view{"B"},
            std::string_view{"C"},
            1.0,
            2.0,
            3.5,
            5.0,
        };

        std::set<ss::pivot_cache_item_t> actual;
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
        ss::pivot_cache::records_type expected =
        {
            { std::size_t(0), 1.1 },
            { std::size_t(1), 1.2 },
            { std::size_t(2), 1.3 },
            { std::size_t(3), 1.4 },
            { std::size_t(4), 1.5 },
            { std::size_t(5), 1.8 },
            { std::size_t(6), 2.2 },
        };

        assert(expected == cache->get_all_records());
    }
}

void test_xlsx_pivot_group_field()
{
    test::stack_printer __sp__(__func__);

    std::string path(SRCDIR"/test/xlsx/pivot-table/group-field.xlsx");

    ss::document doc{{1048576, 16384}};
    ss::import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const ss::pivot_collection& pc = doc.get_pivot_collection();
    assert(pc.get_cache_count() == 1);

    // B2:C6 on sheet 'Sheet1'.
    const ss::pivot_cache* cache = get_pivot_cache(pc, "Sheet1", "B2:C6");
    assert(cache);
    assert(cache->get_field_count() == 3);

    // First field is labeled 'Key'.
    const ss::pivot_cache_field_t* fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "Key");

    {
        // This field should contain 4 string items 'A', 'B', 'C' and 'D'.
        std::set<ss::pivot_cache_item_t> expected =
        {
            std::string_view{"A"},
            std::string_view{"B"},
            std::string_view{"C"},
            std::string_view{"D"},
        };

        std::set<ss::pivot_cache_item_t> actual(fld->items.begin(), fld->items.end());
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

    const ss::pivot_cache_group_data_t* gd = fld->group_data.get();
    assert(gd);
    assert(gd->base_field == 0);
    assert(gd->items.size() == 2);

    {
        // It should have two items - Group1 and Group2.
        std::set<ss::pivot_cache_item_t> expected =
        {
            std::string_view{"Group1"},
            std::string_view{"Group2"},
        };

        std::set<ss::pivot_cache_item_t> actual;
        actual.insert(gd->items.begin(), gd->items.end());
        assert(actual == expected);
    }

    // Group1 should group 'A' and 'B' from the 1st field, and Group2 should
    // group 'C' and 'D'.

    ss::pivot_cache_indices_t expected_group = { 0, 0, 1, 1 };
    assert(gd->base_to_group_indices == expected_group);

    {
        // Check the records.
        ss::pivot_cache::records_type expected =
        {
            { std::size_t(0), 1.0 },
            { std::size_t(1), 2.0 },
            { std::size_t(2), 3.0 },
            { std::size_t(3), 4.0 },
        };

        assert(expected == cache->get_all_records());
    }
}

void test_xlsx_pivot_group_by_numbers()
{
    test::stack_printer __sp__(__func__);

    std::string path(SRCDIR"/test/xlsx/pivot-table/group-by-numbers.xlsx");

    ss::document doc{{1048576, 16384}};
    ss::import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const ss::pivot_collection& pc = doc.get_pivot_collection();
    assert(pc.get_cache_count() == 1);

    // B2:C13 on sheet 'Sheet1'.
    const ss::pivot_cache* cache = get_pivot_cache(pc, "Sheet1", "B2:C13");
    assert(cache);
    assert(cache->get_field_count() == 2);

    // First field is a field with numeric grouping with intervals.
    const ss::pivot_cache_field_t* fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "V1");

    // There should be 11 raw values ranging from 9.78E-2 to 9.82.
    assert(fld->items.size() == 11);
    assert(fld->min_value);
    assert(fld->max_value);
    assert(std::round(*fld->min_value*10000.0) == 978.00); // 9.78E-2
    assert(std::round(*fld->max_value*100.0) == 982.00);   // 9.82

    // We'll just make sure that all 11 items are of numeric type.

    for (const auto& item : fld->items)
    {
        assert(item.type == ss::pivot_cache_item_t::item_type::numeric);
        assert(*fld->min_value <= std::get<double>(item.value));
        assert(std::get<double>(item.value) <= *fld->max_value);
    }

    // This field is also gruop field with 7 numeric intervals of width 2.
    assert(fld->group_data);
    const ss::pivot_cache_group_data_t& grp = *fld->group_data;
    assert(grp.items.size() == 7);

    ss::pivot_cache_items_t expected =
    {
        std::string_view{"<0"},
        std::string_view{"0-2"},
        std::string_view{"2-4"},
        std::string_view{"4-6"},
        std::string_view{"6-8"},
        std::string_view{"8-10"},
        std::string_view{">10"},
    };

    assert(grp.items == expected);

    // Check the numeric range properties.
    assert(grp.range_grouping);
    assert(grp.range_grouping->group_by == ss::pivot_cache_group_by_t::range);
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
    test::stack_printer __sp__(__func__);

    std::string path(SRCDIR"/test/xlsx/pivot-table/group-by-dates.xlsx");

    ss::document doc{{1048576, 16384}};
    ss::import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const ss::pivot_collection& pc = doc.get_pivot_collection();
    assert(pc.get_cache_count() == 1);

    const ss::pivot_cache* cache = get_pivot_cache(pc, "Sheet1", "B2:C14");
    assert(cache);

    // First field is a date field.
    const ss::pivot_cache_field_t* fld = cache->get_field(0);
    assert(fld);
    assert(fld->name == "Date");

    // Minimum and maximum date values.
    assert(fld->min_date);
    assert(*fld->min_date == date_time_t(2014, 1, 1));
    assert(fld->max_date);
    assert(*fld->max_date == date_time_t(2014, 12, 2));

    ss::pivot_cache_items_t expected =
    {
        date_time_t(2014, 1, 1),
        date_time_t(2014, 2, 1),
        date_time_t(2014, 3, 1),
        date_time_t(2014, 4, 1),
        date_time_t(2014, 5, 1),
        date_time_t(2014, 6, 1),
        date_time_t(2014, 7, 1),
        date_time_t(2014, 8, 1),
        date_time_t(2014, 9, 1),
        date_time_t(2014, 10, 1),
        date_time_t(2014, 11, 1),
        date_time_t(2014, 12, 1),
    };

    ss::pivot_cache_items_t actual(fld->items.begin(), fld->items.end());
    assert(actual == expected);

    // This field is grouped by month.

    assert(fld->group_data);
    const ss::pivot_cache_group_data_t& gd = *fld->group_data;

    expected =
    {
        std::string_view{"<1/1/2014"},
        std::string_view{"Jan"},
        std::string_view{"Feb"},
        std::string_view{"Mar"},
        std::string_view{"Apr"},
        std::string_view{"May"},
        std::string_view{"Jun"},
        std::string_view{"Jul"},
        std::string_view{"Aug"},
        std::string_view{"Sep"},
        std::string_view{"Oct"},
        std::string_view{"Nov"},
        std::string_view{"Dec"},
        std::string_view{">12/2/2014"},
    };

    assert(gd.items == expected);

    assert(gd.range_grouping);
    assert(gd.range_grouping->group_by == ss::pivot_cache_group_by_t::months);

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
    const ss::pivot_cache_group_data_t& gd_qtrs = *fld->group_data;
    assert(gd_qtrs.base_field == 0);

    assert(gd_qtrs.range_grouping);
    assert(gd_qtrs.range_grouping->group_by == ss::pivot_cache_group_by_t::quarters);
    assert(gd_qtrs.range_grouping->start_date == date_time_t(2014,1,1));
    assert(gd_qtrs.range_grouping->end_date == date_time_t(2014,12,2));

    expected =
    {
        std::string_view{"<1/1/2014"},
        std::string_view{"Qtr1"},
        std::string_view{"Qtr2"},
        std::string_view{"Qtr3"},
        std::string_view{"Qtr4"},
        std::string_view{">12/2/2014"},
    };

    assert(gd_qtrs.items == expected);
}

void test_xlsx_pivot_error_values()
{
    test::stack_printer __sp__(__func__);

    std::string path(SRCDIR"/test/xlsx/pivot-table/error-values.xlsx");

    ss::document doc{{1048576, 16384}};
    ss::import_factory factory(doc);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    const ss::pivot_collection& pc = doc.get_pivot_collection();
    assert(pc.get_cache_count() == 1);

    const ss::pivot_cache* cache = get_pivot_cache(pc, "Sheet1", "B2:C6");
    assert(cache);

    const ss::pivot_cache_field_t* fld = cache->get_field(0);

    assert(fld);
    assert(fld->name == "F1");

    // This field should contain 4 string items 'A', 'B', 'C' and 'D'.
    std::set<ss::pivot_cache_item_t> expected =
    {
        std::string_view{"A"},
        std::string_view{"B"},
        std::string_view{"C"},
        std::string_view{"D"},
    };

    std::set<ss::pivot_cache_item_t> actual(fld->items.begin(), fld->items.end());
    assert(actual == expected);

    fld = cache->get_field(1);

    assert(fld);
    assert(fld->name == "F2");

    expected =
    {
        spreadsheet::error_value_t::div0,
        spreadsheet::error_value_t::name,
    };

    actual.clear();
    actual.insert(fld->items.begin(), fld->items.end());

    assert(actual == expected);
}

void test_xlsx_view_cursor_per_sheet()
{
    test::stack_printer __sp__(__func__);

    std::string path(SRCDIR"/test/xlsx/view/cursor-per-sheet.xlsx");

    ss::document doc{{1048576, 16384}};
    ss::view view(doc);
    ss::import_factory factory(doc, view);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    // Sheet3 should be active.
    assert(view.get_active_sheet() == 2);

    const ss::sheet_view* sv = view.get_sheet_view(0);
    assert(sv);

    ss::iface::import_reference_resolver* resolver =
        factory.get_reference_resolver(ss::formula_ref_context_t::global);
    assert(resolver);

    // On Sheet1, the cursor should be set to C4.
    ss::range_t expected = to_rc_range(resolver->resolve_range("C4"));
    ss::range_t actual = sv->get_selection(ss::sheet_pane_t::top_left);
    assert(expected == actual);

    sv = view.get_sheet_view(1);
    assert(sv);

    // On Sheet2, the cursor should be set to D8.
    expected = to_rc_range(resolver->resolve_range("D8"));
    actual = sv->get_selection(ss::sheet_pane_t::top_left);
    assert(expected == actual);

    sv = view.get_sheet_view(2);
    assert(sv);

    // On Sheet3, the cursor should be set to D2.
    expected = to_rc_range(resolver->resolve_range("D2"));
    actual = sv->get_selection(ss::sheet_pane_t::top_left);
    assert(expected == actual);

    sv = view.get_sheet_view(3);
    assert(sv);

    // On Sheet4, the cursor should be set to C5:E8.
    expected = to_rc_range(resolver->resolve_range("C5:E8"));
    actual = sv->get_selection(ss::sheet_pane_t::top_left);
    assert(expected == actual);
}

struct expected_selection
{
    ss::sheet_pane_t pane;
    std::string_view sel;
};

void test_xlsx_view_cursor_split_pane()
{
    test::stack_printer __sp__(__func__);

    std::string path(SRCDIR"/test/xlsx/view/cursor-split-pane.xlsx");

    ss::document doc{{1048576, 16384}};
    ss::view view(doc);
    ss::import_factory factory(doc, view);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    ss::iface::import_reference_resolver* resolver =
        factory.get_reference_resolver(ss::formula_ref_context_t::global);
    assert(resolver);

    // Sheet4 should be active.
    assert(view.get_active_sheet() == 3);

    const ss::sheet_view* sv = view.get_sheet_view(0);
    assert(sv);

    // On Sheet1, the view is split into 4.
    assert(sv->get_active_pane() == ss::sheet_pane_t::bottom_left);
    assert(sv->get_split_pane().hor_split == 5190.0);
    assert(sv->get_split_pane().ver_split == 1800.0);

    {
        ss::address_t expected = to_rc_address(resolver->resolve_address("F6"));
        ss::address_t actual = sv->get_split_pane().top_left_cell;
        assert(expected == actual);
    }

    std::vector<expected_selection> expected_selections =
    {
        { ss::sheet_pane_t::top_left,     "E4"  },
        { ss::sheet_pane_t::top_right,    "J2"  },
        { ss::sheet_pane_t::bottom_left,  "A8"  },
        { ss::sheet_pane_t::bottom_right, "J17" },
    };

    for (const expected_selection& es : expected_selections)
    {
        // cursor in the top-left pane.
        ss::range_t expected = to_rc_range(resolver->resolve_range(es.sel));
        ss::range_t actual = sv->get_selection(es.pane);
        assert(expected == actual);
    }

    sv = view.get_sheet_view(1);
    assert(sv);

    // Sheet2 is also split into 4 views.
    assert(sv->get_active_pane() == ss::sheet_pane_t::top_right);
    assert(sv->get_split_pane().hor_split == 5190.0);
    assert(sv->get_split_pane().ver_split == 2400.0);

    {
        ss::address_t expected = to_rc_address(resolver->resolve_address("F8"));
        ss::address_t actual = sv->get_split_pane().top_left_cell;
        assert(expected == actual);
    }

    expected_selections =
    {
        { ss::sheet_pane_t::top_left,     "C2:C6"   },
        { ss::sheet_pane_t::top_right,    "H2:L2"   },
        { ss::sheet_pane_t::bottom_left,  "B18:C23" },
        { ss::sheet_pane_t::bottom_right, "H11:J13" },
    };

    for (const expected_selection& es : expected_selections)
    {
        // cursor in the top-left pane.
        ss::range_t expected = to_rc_range(resolver->resolve_range(es.sel));
        ss::range_t actual = sv->get_selection(es.pane);
        assert(expected == actual);
    }

    sv = view.get_sheet_view(2);
    assert(sv);

    // Sheet3 is horizontally split into top and bottom views (top-left and bottom-left).
    assert(sv->get_active_pane() == ss::sheet_pane_t::bottom_left);
    assert(sv->get_split_pane().hor_split == 0.0);
    assert(sv->get_split_pane().ver_split == 1500.0);

    {
        ss::address_t expected = to_rc_address(resolver->resolve_address("A5"));
        ss::address_t actual = sv->get_split_pane().top_left_cell;
        assert(expected == actual);
    }

    expected_selections =
    {
        { ss::sheet_pane_t::top_left,     "D2" },
        { ss::sheet_pane_t::bottom_left,  "C9" },
    };

    for (const expected_selection& es : expected_selections)
    {
        // cursor in the top-left pane.
        ss::range_t expected = to_rc_range(resolver->resolve_range(es.sel));
        ss::range_t actual = sv->get_selection(es.pane);
        assert(expected == actual);
    }

    sv = view.get_sheet_view(3);
    assert(sv);

    // Sheet4 is vertically split into left and right views (top-left and top-right).
    assert(sv->get_active_pane() == ss::sheet_pane_t::top_left);
    assert(sv->get_split_pane().hor_split == 4230.0);
    assert(sv->get_split_pane().ver_split == 0.0);

    {
        ss::address_t expected = to_rc_address(resolver->resolve_address("E1"));
        ss::address_t actual = sv->get_split_pane().top_left_cell;
        assert(expected == actual);
    }

    expected_selections =
    {
        { ss::sheet_pane_t::top_left,  "B18" },
        { ss::sheet_pane_t::top_right, "I11" },
    };

    for (const expected_selection& es : expected_selections)
    {
        // cursor in the top-left pane.
        ss::range_t expected = to_rc_range(resolver->resolve_range(es.sel));
        ss::range_t actual = sv->get_selection(es.pane);
        assert(expected == actual);
    }
}

void test_xlsx_view_frozen_pane()
{
    test::stack_printer __sp__(__func__);

    std::string path(SRCDIR"/test/xlsx/view/frozen-pane.xlsx");

    ss::document doc{{1048576, 16384}};
    ss::view view(doc);
    ss::import_factory factory(doc, view);
    orcus_xlsx app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    ss::iface::import_reference_resolver* resolver =
        factory.get_reference_resolver(ss::formula_ref_context_t::global);
    assert(resolver);

    // Sheet3 should be active.
    assert(view.get_active_sheet() == 2);

    const ss::sheet_view* sv = view.get_sheet_view(0);
    assert(sv);

    {
        // Sheet1 is vertically frozen between columns A and B.
        const ss::frozen_pane_t& fp = sv->get_frozen_pane();
        assert(fp.top_left_cell == to_rc_address(resolver->resolve_address("B1")));
        assert(fp.visible_columns == 1);
        assert(fp.visible_rows == 0);
        assert(sv->get_active_pane() == ss::sheet_pane_t::top_right);
    }

    sv = view.get_sheet_view(1);
    assert(sv);

    {
        // Sheet2 is horizontally frozen between rows 1 and 2.
        const ss::frozen_pane_t& fp = sv->get_frozen_pane();
        assert(fp.top_left_cell == to_rc_address(resolver->resolve_address("A2")));
        assert(fp.visible_columns == 0);
        assert(fp.visible_rows == 1);
        assert(sv->get_active_pane() == ss::sheet_pane_t::bottom_left);
    }

    sv = view.get_sheet_view(2);
    assert(sv);

    {
        // Sheet3 is frozen both horizontally and vertically.
        const ss::frozen_pane_t& fp = sv->get_frozen_pane();
        assert(fp.top_left_cell == to_rc_address(resolver->resolve_address("E9")));
        assert(fp.visible_columns == 4);
        assert(fp.visible_rows == 8);
        assert(sv->get_active_pane() == ss::sheet_pane_t::bottom_right);
    }
}

void test_xlsx_doc_structure_unordered_sheet_positions()
{
    test::stack_printer __sp__(__func__);

    std::string_view path(SRCDIR"/test/xlsx/doc-structure/unordered-sheet-positions.xlsx");
    std::unique_ptr<ss::document> doc = load_doc(path);

    // There should be 9 sheets named S1, S2, ..., S9.
    std::vector<std::string_view> expected_sheet_names = {
        "S1", "S2", "S3", "S4", "S5", "S6", "S7", "S8", "S9"
    };

    assert(doc->get_sheet_count() == expected_sheet_names.size());

    ss::sheet_t n = expected_sheet_names.size();
    for (ss::sheet_t i = 0; i < n; ++i)
    {
        std::string_view sheet_name = doc->get_sheet_name(i);
        assert(sheet_name == expected_sheet_names[i]);
    }
}

}

int main()
{
    test_config.debug = false;
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
    test_xlsx_cell_properties();
    test_xlsx_styles_direct_format();
    test_xlsx_styles_column_styles();

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
