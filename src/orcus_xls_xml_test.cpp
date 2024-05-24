/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_test_global.hpp"
#include "filesystem_env.hpp"

#include "orcus/orcus_xls_xml.hpp"
#include <orcus/format_detection.hpp>
#include "orcus/stream.hpp"
#include "orcus/config.hpp"
#include <orcus/parser_global.hpp>
#include "orcus/yaml_document_tree.hpp"
#include "orcus/spreadsheet/factory.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/view.hpp"
#include "orcus/spreadsheet/sheet.hpp"
#include "orcus/spreadsheet/shared_strings.hpp"
#include "orcus/spreadsheet/styles.hpp"
#include "orcus/spreadsheet/config.hpp"

#include <ixion/model_context.hpp>
#include <ixion/address.hpp>
#include <ixion/cell.hpp>

#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>

using namespace orcus;
namespace ss = orcus::spreadsheet;

namespace {

config test_config(format_t::xls_xml);

const std::vector<fs::path> dirs = {
    SRCDIR"/test/xls-xml/basic/",
    SRCDIR"/test/xls-xml/basic-utf-16-be/",
    SRCDIR"/test/xls-xml/basic-utf-16-le/",
    SRCDIR"/test/xls-xml/bold-and-italic/",
    SRCDIR"/test/xls-xml/colored-text/",
    SRCDIR"/test/xls-xml/empty-rows/",
    SRCDIR"/test/xls-xml/formula-array-1/",
    SRCDIR"/test/xls-xml/formula-cells-1/",
    SRCDIR"/test/xls-xml/formula-cells-2/",
    SRCDIR"/test/xls-xml/formula-cells-3/",
    SRCDIR"/test/xls-xml/invalid-sub-structure/",
    SRCDIR"/test/xls-xml/leading-whitespace/",
    SRCDIR"/test/xls-xml/merged-cells/",
    SRCDIR"/test/xls-xml/named-colors/",
    SRCDIR"/test/xls-xml/named-expression/",
    SRCDIR"/test/xls-xml/named-expression-sheet-local/",
    SRCDIR"/test/xls-xml/raw-values-1/",
    SRCDIR"/test/xls-xml/table-offset/",
    SRCDIR"/test/xls-xml/unnamed-parent-styles/",
};

std::unique_ptr<spreadsheet::document> load_doc_from_filepath(
    const std::string& path,
    bool recalc=true,
    ss::formula_error_policy_t error_policy=ss::formula_error_policy_t::fail)
{
    std::cout << path << std::endl;

    spreadsheet::range_size_t ss{1048576, 16384};
    std::unique_ptr<spreadsheet::document> doc = std::make_unique<spreadsheet::document>(ss);
    spreadsheet::import_factory factory(*doc);
    factory.set_recalc_formula_cells(recalc);
    factory.set_formula_error_policy(error_policy);
    orcus_xls_xml app(&factory);
    app.set_config(test_config);
    app.read_file(path.c_str());

    return doc;
}

std::unique_ptr<spreadsheet::document> load_doc_from_stream(const std::string& path)
{
    spreadsheet::range_size_t ss{1048576, 16384};
    std::unique_ptr<spreadsheet::document> doc = std::make_unique<spreadsheet::document>(ss);
    spreadsheet::import_factory factory(*doc);
    orcus_xls_xml app(&factory);

    std::ifstream ifs(path, std::ios::binary | std::ios::ate);
    std::streamsize n = ifs.tellg();
    ifs.seekg(0);
    std::string content(n, '\0');
    if (ifs.read(content.data(), n))
    {
        app.read_stream(content);
        doc->recalc_formula_cells();
    }

    return doc;
}

class doc_loader
{
    spreadsheet::document m_doc;
    spreadsheet::import_factory m_factory;

public:
    doc_loader(std::string_view path) :
        m_doc({1048576, 16384}), m_factory(m_doc)
    {
        std::cout << path << std::endl;
        orcus_xls_xml app(&m_factory);
        app.read_file(path.data());
    }

    spreadsheet::document& get_doc()
    {
        return m_doc;
    }

    spreadsheet::import_factory& get_factory()
    {
        return m_factory;
    }
};

void update_config(spreadsheet::document& doc, const std::string& path)
{
    try
    {
        spreadsheet::document_config cfg = doc.get_config();

        yaml::document_tree config;
        file_content content(path.data());
        config.load(content.str());
        yaml::const_node root = config.get_document_root(0);
        std::vector<yaml::const_node> keys = root.keys();
        for (size_t i = 0; i < keys.size(); ++i)
        {
            const yaml::const_node& key = keys[i];
            if (key.type() == yaml::node_t::string && key.string_value() == "output-precision")
            {
                yaml::const_node child = root.child(i);
                if (child.type() == yaml::node_t::number)
                    cfg.output_precision = child.numeric_value();
            }
        }

        doc.set_config(cfg);
    }
    catch (const std::exception&)
    {
        // Do nothing.
    }
}

void test_xls_xml_detection()
{
    ORCUS_TEST_FUNC_SCOPE;

    for (const auto& dir : dirs)
    {
        fs::path filepath = dir / "input.xml";
        file_content fc(filepath.string());
        assert(!fc.empty());

        format_t detected = detect(fc.str());
        assert(detected == format_t::xls_xml);
    }
}

void test_xls_xml_create_filter()
{
    ORCUS_TEST_FUNC_SCOPE;

    ss::range_size_t ssize{1048576, 16384};
    std::unique_ptr<ss::document> doc = std::make_unique<ss::document>(ssize);
    ss::import_factory factory(*doc);

    auto f = create_filter(format_t::xls_xml, &factory);
    assert(f);
    assert(f->get_name() == "xls-xml");
}

void test_xls_xml_import()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto verify = [](spreadsheet::document& doc, const fs::path& dir)
    {
        auto path = dir / "config.yaml";
        update_config(doc, path.string());

        // Dump the content of the model.
        std::ostringstream os;
        doc.dump_check(os);
        std::string check = os.str();

        // Check that against known control.
        path = dir / "check.txt";
        file_content control(path.string());

        assert(!check.empty());
        assert(!control.empty());

        std::string_view s1(check.data(), check.size());
        std::string_view s2 = control.str();
        s1 = orcus::trim(s1);
        s2 = orcus::trim(s2);

        if (s1 != s2)
        {
            size_t offset = locate_first_different_char(s1, s2);
            auto line1 = locate_line_with_offset(s1, offset);
            auto line2 = locate_line_with_offset(s2, offset);
            std::cout << "expected: " << line2.line << std::endl;
            std::cout << "observed: " << line1.line << std::endl;
            assert(!"content verification failed");
        }
    };

    for (const auto& dir : dirs)
    {
        std::cout << dir << std::endl;

        // Read the input.xml document.
        fs::path filepath = dir / "input.xml";
        std::unique_ptr<spreadsheet::document> doc = load_doc_from_filepath(filepath.string());
        verify(*doc, dir);

        doc = load_doc_from_stream(filepath.string());
        verify(*doc, dir);
    }
}

void test_xls_xml_merged_cells()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/merged-cells/input.xml");

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
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/date-time/input.xml");

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
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/bold-and-italic/input.xml");

    const spreadsheet::sheet* sheet1 = doc->get_sheet("Sheet1");
    assert(sheet1);

    const spreadsheet::shared_strings& ss = doc->get_shared_strings();

    const spreadsheet::styles& styles = doc->get_styles();

    // A1 contains unformatted text.
    size_t si = sheet1->get_string_identifier(0, 0);
    const std::string* sp = ss.get_string(si);
    assert(sp);
    assert(*sp == "Normal Text");

    // A2 contains bold text.
    si = sheet1->get_string_identifier(1, 0);
    sp = ss.get_string(si);
    assert(sp);
    assert(*sp == "Bold Text");

    size_t xfi = sheet1->get_cell_format(1, 0);
    const spreadsheet::cell_format_t* cf = styles.get_cell_format(xfi);
    assert(cf);
    const spreadsheet::font_t* font = styles.get_font(cf->font);
    assert(font);
    assert(*font->bold);
    assert(!*font->italic);

    // A3 contains italic text.
    si = sheet1->get_string_identifier(2, 0);
    sp = ss.get_string(si);
    assert(sp);
    assert(*sp == "Italic Text");

    xfi = sheet1->get_cell_format(2, 0);
    cf = styles.get_cell_format(xfi);
    assert(cf);
    font = styles.get_font(cf->font);
    assert(font);
    assert(!*font->bold);
    assert(*font->italic);

    // A4 contains bold and italic text.
    si = sheet1->get_string_identifier(3, 0);
    sp = ss.get_string(si);
    assert(sp);
    assert(*sp == "Bold and Italic Text");

    xfi = sheet1->get_cell_format(3, 0);
    cf = styles.get_cell_format(xfi);
    assert(cf);
    font = styles.get_font(cf->font);
    assert(font);
    assert(font->bold);
    assert(font->italic);

    // A5 contains a mixed format text.
    si = sheet1->get_string_identifier(4, 0);
    sp = ss.get_string(si);
    assert(sp);
    assert(*sp == "Bold and Italic mixed");

    // The string contains 4 formatted segments.
    const spreadsheet::format_runs_t* fmt_runs = ss.get_format_runs(si);
    assert(fmt_runs);
    assert(fmt_runs->size() == 4);

    // First formatted segment is bold.
    const spreadsheet::format_run_t* fmt_run = &fmt_runs->at(0);
    assert(fmt_run->pos == 0);
    assert(fmt_run->size == 4);
    assert(fmt_run->bold && fmt_run->bold.value());
    assert(!fmt_run->italic.has_value() || !fmt_run->italic.value());

    // Third formatted segment is italic.
    fmt_run = &fmt_runs->at(2);
    assert(fmt_run->pos == 9);
    assert(fmt_run->size == 6);
    assert(!fmt_run->bold.has_value() || !fmt_run->bold.value());
    assert(fmt_run->italic && fmt_run->italic.value());
}

void test_xls_xml_colored_text()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/colored-text/input.xml");

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

    // Cell B2 contains mix-colored text.
    size_t si = sheet1->get_string_identifier(1, 1);
    const std::string* s = ss.get_string(si);
    assert(s);
    assert(*s == "Red and Blue");
    const spreadsheet::format_runs_t* fmt_runs = ss.get_format_runs(si);
    assert(fmt_runs);

    // There should be 2 segments that are color-formatted and one that is not.
    assert(fmt_runs->size() == 3);

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
    fmt = &fmt_runs->at(2);
    assert(fmt->color);
    assert(fmt->color->alpha == 0xFF);
    assert(fmt->color->red == 0);
    assert(fmt->color->green == 0x70);
    assert(fmt->color->blue == 0xC0);
    assert(fmt->pos == 8);
    assert(fmt->size == 4);
}

void test_xls_xml_formatted_text_basic()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/formatted-text/basic.xml");
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

    {
        const spreadsheet::sheet* sheet = doc->get_sheet("Text Properties");
        assert(sheet);

        ss::row_t row = 0;
        ss::col_t col = 0;

        // A1 - unformatted
        assert(check_cell_text(*sheet, row, col, "Normal Text"));
        assert(check_cell_bold(*sheet, row, col, false));
        assert(check_cell_italic(*sheet, row, col, false));

        // A2 - bold
        row = 1;
        assert(check_cell_text(*sheet, row, col, "Bold Text"));
        assert(check_cell_bold(*sheet, row, col, true));
        assert(check_cell_italic(*sheet, row, col, false));

        // A3 - italic
        row = 2;
        assert(check_cell_text(*sheet, row, col, "Italic Text"));
        assert(check_cell_bold(*sheet, row, col, false));
        assert(check_cell_italic(*sheet, row, col, true));

        // A4 - bold and italic
        row = 3;
        assert(check_cell_text(*sheet, row, col, "Bold and Italic Text"));
        assert(check_cell_bold(*sheet, row, col, true));
        assert(check_cell_italic(*sheet, row, col, true));

        // A5 - bold and italic mixed. Excel creates format runs even for
        // non-formatted segments.
        row = 4;
        assert(check_cell_text(*sheet, row, col, "Bold and Italic mixed"));

        std::size_t si = sheet->get_string_identifier(row, col);
        const ss::format_runs_t* runs = doc->get_shared_strings().get_format_runs(si);
        assert(runs);
        assert(runs->size() == 4u); // only 0 and 2 are formatted

        // Bold and ...
        // ^^^^
        assert(runs->at(0).pos == 0);
        assert(runs->at(0).size == 4);
        assert(test::set(runs->at(0).bold));
        assert(!test::set(runs->at(0).italic));

        // Bold and Italic
        //          ^^^^^^
        assert(runs->at(2).pos == 9);
        assert(runs->at(2).size == 6);
        assert(!test::set(runs->at(2).bold));
        assert(test::set(runs->at(2).italic));

        // A6
        row = 5;
        assert(check_cell_text(*sheet, row, col, "Bold base with non-bold part"));
        assert(check_cell_bold(*sheet, row, col, true));
        assert(check_cell_italic(*sheet, row, col, false));

        si = sheet->get_string_identifier(row, col);
        runs = doc->get_shared_strings().get_format_runs(si);
        assert(runs);
        assert(runs->size() == 3u);

        // Bold base with non-bold part
        // ^^^^^^^^^^^^^^^
        assert(runs->at(0).pos == 0);
        assert(runs->at(0).size == 15);
        assert(test::set(runs->at(0).bold));

        // Bold base with non-bold part
        //                ^^^^^^^^
        assert(runs->at(1).pos == 15);
        assert(runs->at(1).size == 8);
        assert(!test::set(runs->at(1).bold));

        // Bold base with non-bold part
        //                        ^^^^^
        assert(runs->at(2).pos == 23);
        assert(runs->at(2).size == 5);
        assert(test::set(runs->at(2).bold));

        // A7 - TODO: check format
        row = 6;
        assert(check_cell_text(*sheet, row, col, "Only partially underlined"));

        // A8
        row = 7;
        assert(check_cell_text(*sheet, row, col, "All Underlined"));
        const ss::font_t* font = get_font(*sheet, row, col);
        assert(font->underline.style);
        assert(*font->underline.style == ss::underline_style_t::solid);
        assert(font->underline.count);
        assert(*font->underline.count == ss::underline_count_t::single_count);

        // A9
        row = 8;
        assert(check_cell_text(*sheet, row, col, "Bold and underlined"));
        assert(check_cell_bold(*sheet, row, col, true));
        font = get_font(*sheet, row, col);
        assert(font->underline.style);
        assert(*font->underline.style == ss::underline_style_t::solid);
        assert(font->underline.count);
        assert(*font->underline.count == ss::underline_count_t::single_count);

        row = 9;
        assert(check_cell_text(*sheet, row, col, "All Strikethrough"));
        font = get_font(*sheet, row, col);
        assert(test::strikethrough_set(font->strikethrough));

        // A11
        row = 10;
        assert(check_cell_text(*sheet, row, col, "Partial strikethrough"));
        si = sheet->get_string_identifier(row, col);
        runs = doc->get_shared_strings().get_format_runs(si);
        assert(runs);
        assert(runs->size() == 2u);
        // Partial strikethrough
        // ^^^^^^^^
        assert(runs->at(0).pos == 0);
        assert(runs->at(0).size == 8);
        assert(!test::strikethrough_set(runs->at(0).strikethrough));

        // Partial strikethrough
        //         ^^^^^^^^^^^^^
        assert(runs->at(1).pos == 8);
        assert(runs->at(1).size == 13);
        assert(test::strikethrough_set(runs->at(1).strikethrough));

        // A12:A15 - TODO: check format
        row = 11;
        assert(check_cell_text(*sheet, row, col, "Superscript"));
        row = 12;
        assert(check_cell_text(*sheet, row, col, "Subscript"));
        row = 13;
        assert(check_cell_text(*sheet, row, col, "x2 + y2 = 102"));

        si = sheet->get_string_identifier(row, col);
        runs = doc->get_shared_strings().get_format_runs(si);
        assert(runs);
        assert(runs->size() == 6u);
        assert(runs->at(0).pos == 0);
        assert(runs->at(0).size == 1);
        assert(!test::set(runs->at(0).superscript));
        assert(runs->at(1).pos == 1);
        assert(runs->at(1).size == 1);
        assert(test::set(runs->at(1).superscript));
        assert(runs->at(2).pos == 2);
        assert(runs->at(2).size == 4);
        assert(!test::set(runs->at(2).superscript));
        assert(runs->at(3).pos == 6);
        assert(runs->at(3).size == 1);
        assert(test::set(runs->at(3).superscript));
        assert(runs->at(4).pos == 7);
        assert(runs->at(4).size == 5);
        assert(!test::set(runs->at(4).superscript));
        assert(runs->at(5).pos == 12);
        assert(runs->at(5).size == 1);
        assert(test::set(runs->at(5).superscript));

        row = 14;
        assert(check_cell_text(*sheet, row, col, "xi = yi + zi"));

        si = sheet->get_string_identifier(row, col);
        runs = doc->get_shared_strings().get_format_runs(si);
        assert(runs);
        assert(runs->size() == 6u);
        assert(runs->at(0).pos == 0);
        assert(runs->at(0).size == 1);
        assert(!test::set(runs->at(0).subscript));
        assert(runs->at(1).pos == 1);
        assert(runs->at(1).size == 1);
        assert(test::set(runs->at(1).subscript));
        assert(runs->at(2).pos == 2);
        assert(runs->at(2).size == 4);
        assert(!test::set(runs->at(2).subscript));
        assert(runs->at(3).pos == 6);
        assert(runs->at(3).size == 1);
        assert(test::set(runs->at(3).subscript));
        assert(runs->at(4).pos == 7);
        assert(runs->at(4).size == 4);
        assert(!test::set(runs->at(4).subscript));
        assert(runs->at(5).pos == 11);
        assert(runs->at(5).size == 1);
        assert(test::set(runs->at(5).subscript));
    }

    {
        const spreadsheet::sheet* sheet = doc->get_sheet("Fonts");
        assert(sheet);

        struct check
        {
            ss::row_t row;
            std::string_view font_name;
            double font_unit;
        };

        check checks[] = {
            { 0, "Calibri Light", 12.0 },
            { 1, "Arial", 18.0 },
            { 2, "Times New Roman", 14.0 },
            { 3, "Consolas", 9.0 },
            { 4, "Bookman Old Style", 20.0 },
        };

        for (const auto& c : checks)
        {
            std::size_t xf = sheet->get_cell_format(c.row, 0);
            const ss::cell_format_t* cell_format = styles_pool.get_cell_format(xf);
            assert(cell_format);
            const ss::font_t* font = styles_pool.get_font(cell_format->font);
            assert(font);
            assert(font->name == c.font_name);
            assert(font->size == c.font_unit);

            // Columns A and B should have the same font.
            xf = sheet->get_cell_format(c.row, 1);
            cell_format = styles_pool.get_cell_format(xf);
            assert(cell_format);
            font = styles_pool.get_font(cell_format->font);
            assert(font);
            assert(font->name == c.font_name);
            assert(font->size == c.font_unit);
        }
    }

    {
        const spreadsheet::sheet* sheet = doc->get_sheet("Mixed Fonts");
        assert(sheet);

        // A1
        ss::row_t row = 0;
        ss::col_t col = 0;
        assert(check_cell_text(*sheet, row, col, "C++ has class and struct as keywords."));

        // Base cell has Serif 12-pt font applied
        auto xf = sheet->get_cell_format(row, col);
        const ss::cell_format_t* fmt = styles_pool.get_cell_format(xf);
        assert(fmt);
        const ss::font_t* font = styles_pool.get_font(fmt->font);
        assert(font);
        assert(font->name == "Calibri");
        assert(font->size == 11.0);

        // Two segments has Liberation Mono font applied (runs 1 and 3) whereas
        // runs 0, 2 and 4 are unformatted.
        std::size_t si = sheet->get_string_identifier(row, col);
        const ss::format_runs_t* runs = doc->get_shared_strings().get_format_runs(si);
        assert(runs);
        assert(runs->size() == 5u);

        // C++ has class ...
        //         ^^^^^
        assert(runs->at(1).pos == 8);
        assert(runs->at(1).size == 5);
        assert(runs->at(1).font == "Liberation Mono");

        // ... and struct as ...
        //         ^^^^^^
        assert(runs->at(3).pos == 18);
        assert(runs->at(3).size == 6);
        assert(runs->at(3).font == "Liberation Mono");

        // A2
        row = 1;
        assert(check_cell_text(*sheet, row, col, "Text with 12-point font, 24-point font, and 36-point font mixed."));
        si = sheet->get_string_identifier(row, col);
        runs = doc->get_shared_strings().get_format_runs(si);
        assert(runs);
        assert(runs->size() == 10u);

        // with 12-point font, ...
        //      ^^
        assert(runs->at(1).pos == 10);
        assert(runs->at(1).size == 2);
        assert(runs->at(1).font_size == 12.0f);
        assert(runs->at(1).color == ss::color_t(0xFF, 0xFF, 0, 0)); // red

        // with 12-point font, ...
        //        ^^^^^^
        assert(runs->at(2).pos == 12);
        assert(runs->at(2).size == 6);
        assert(runs->at(2).font_size == 12.0f);

        // 24-point font,
        // ^^
        assert(runs->at(4).pos == 25);
        assert(runs->at(4).size == 2);
        assert(runs->at(4).font_size == 24.0f);
        assert(runs->at(4).color == ss::color_t(0xFF, 0xFF, 0, 0)); // red

        // 24-point font,
        //   ^^^^^^
        assert(runs->at(5).pos == 27);
        assert(runs->at(5).size == 6);
        assert(runs->at(5).font_size == 24.0f);

        // and 36-point font
        //     ^^
        assert(runs->at(7).pos == 44);
        assert(runs->at(7).size == 2);
        assert(runs->at(7).font_size == 36.0f);
        assert(runs->at(7).color == ss::color_t(0xFF, 0xFF, 0, 0)); // red

        // and 36-point font
        //       ^^^^^^
        assert(runs->at(8).pos == 46);
        assert(runs->at(8).size == 6);
        assert(runs->at(8).font_size == 36.0f);
    }
}

void test_xls_xml_column_width_row_height()
{
    ORCUS_TEST_FUNC_SCOPE;

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

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/column-width-row-height/input.xml");

    // Column widths and row heights are stored in twips. Convert them to
    // points so that we can compare them with the values stored in the source
    // file.

    {
        // Sheet1
        const spreadsheet::sheet* sheet = doc->get_sheet(0);
        assert(sheet);

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
            spreadsheet::col_width_t cw = sheet->get_col_width(check.col, nullptr, nullptr);
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
            spreadsheet::row_height_t rh = sheet->get_row_height(check.row, nullptr, nullptr);
            double pt = convert(rh, length_unit_t::twip, length_unit_t::point);
            test::verify_value_to_decimals(__FILE__, __LINE__, check.height, pt, check.decimals);
        }
    }

    {
        // Sheet2
        const spreadsheet::sheet* sheet = doc->get_sheet(1);
        assert(sheet);

        std::vector<cw_check> cw_checks =
        {
            { 1, 119.25, 2 },
            { 3, 234.75, 2 },
        };

        for (const cw_check& check : cw_checks)
        {
            spreadsheet::col_width_t cw = sheet->get_col_width(check.col, nullptr, nullptr);
            double pt = convert(cw, length_unit_t::twip, length_unit_t::point);
            test::verify_value_to_decimals(__FILE__, __LINE__, check.width, pt, check.decimals);
        }

        std::vector<rh_check> rh_checks =
        {
            {  2, 40.0, 0 },
            {  4, 60.0, 0 },
        };

        for (const rh_check& check : rh_checks)
        {
            spreadsheet::row_height_t rh = sheet->get_row_height(check.row, nullptr, nullptr);
            double pt = convert(rh, length_unit_t::twip, length_unit_t::point);
            test::verify_value_to_decimals(__FILE__, __LINE__, check.height, pt, check.decimals);
        }
    }
}

void test_xls_xml_background_fill()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/background-color/standard.xml");

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

    spreadsheet::color_t color_white(255, 255, 255, 255);

    for (const check& c : checks)
    {
        size_t xf = sh->get_cell_format(c.row, c.col);

        const spreadsheet::cell_format_t* cf = styles.get_cell_format(xf);
        assert(cf);

        const spreadsheet::fill_t* fill_data = styles.get_fill(cf->fill);
        assert(fill_data);
        assert(fill_data->pattern_type == c.pattern_type);
        assert(fill_data->fg_color == c.fg_color);

        // The font colors are all white in the colored cells.
        const spreadsheet::font_t* font_data = styles.get_font(cf->font);
        assert(font_data);

        assert(font_data->color == color_white);
    }
}

void test_xls_xml_named_colors()
{
    ORCUS_TEST_FUNC_SCOPE;

    constexpr std::string_view paths[] = {
        SRCDIR"/test/xls-xml/named-colors/input.xml",
        SRCDIR"/test/xls-xml/named-colors/input-upper.xml"
    };

    for (auto path : paths)
    {
        std::cout << path << std::endl;
        std::unique_ptr<spreadsheet::document> doc = load_doc_from_filepath(std::string{path});

        spreadsheet::styles& styles = doc->get_styles();
        const ixion::model_context& model = doc->get_model_context();

        spreadsheet::sheet* sh = doc->get_sheet(0);
        assert(sh);

        for (ss::row_t row = 1; row < 141; ++row)
        {
            // Column B stores the expected RGB value in hex.
            size_t sid = model.get_string_identifier(ixion::abs_address_t(sh->get_index(), row, 1));
            const std::string* s = model.get_string(sid);
            assert(s);
            spreadsheet::color_rgb_t expected = spreadsheet::to_color_rgb(*s);

            size_t xf = sh->get_cell_format(row, 0);
            const ss::fill_t* fill_data = styles.get_fill(xf);
            assert(fill_data->fg_color);
            const ss::color_t& actual = *fill_data->fg_color;
            assert(expected.red == actual.red);
            assert(expected.green == actual.green);
            assert(expected.blue == actual.blue);
        }
    }
}

void test_xls_xml_text_alignment()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/text-alignment/input.xml");

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
        {  1, 2,  true, spreadsheet::hor_alignment_t::unknown,     spreadsheet::ver_alignment_t::bottom      }, // C2
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
        std::cout << "row=" << c.row << "; col=" << c.col << std::endl;
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

void test_xls_xml_cell_borders_single_cells()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/borders/single-cells.xml");

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
        {  3, 1, spreadsheet::border_style_t::hair                },
        {  5, 1, spreadsheet::border_style_t::dotted              },
        {  7, 1, spreadsheet::border_style_t::dash_dot_dot        },
        {  9, 1, spreadsheet::border_style_t::dash_dot            },
        { 11, 1, spreadsheet::border_style_t::dashed              },
        { 13, 1, spreadsheet::border_style_t::thin                },
        {  1, 3, spreadsheet::border_style_t::medium_dash_dot_dot },
        {  3, 3, spreadsheet::border_style_t::slant_dash_dot      },
        {  5, 3, spreadsheet::border_style_t::medium_dash_dot     },
        {  7, 3, spreadsheet::border_style_t::medium_dashed       },
        {  9, 3, spreadsheet::border_style_t::medium              },
        { 11, 3, spreadsheet::border_style_t::thick               },
        { 13, 3, spreadsheet::border_style_t::double_border       },
    };

    for (const check& c : checks)
    {
        std::cout << "(row: " << c.row << "; col: " << c.col << "; expected: " << int(c.style) << ")" << std::endl;
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

void test_xls_xml_cell_borders_directions()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/borders/directions.xml");

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
                assert(*border->top.style == ss::border_style_t::thin);
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
            case ss::border_direction_t::bottom:
                assert(!border->top.style);
                assert(!border->top.border_color);
                assert(!border->top.border_width);
                assert(border->bottom.style);
                assert(*border->bottom.style == ss::border_style_t::thin);
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
            case spreadsheet::border_direction_t::diagonal:
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
                assert(!border->diagonal_bl_tr.border_color);
                assert(!border->diagonal_bl_tr.border_width);
                assert(border->diagonal_tl_br.style);
                assert(*border->diagonal_tl_br.style == ss::border_style_t::thin);
                assert(!border->diagonal_tl_br.border_color);
                assert(!border->diagonal_tl_br.border_width);
                break;
            case spreadsheet::border_direction_t::diagonal_tl_br:
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
                assert(!border->diagonal_tl_br.border_color);
                assert(!border->diagonal_tl_br.border_width);
                break;
            case spreadsheet::border_direction_t::diagonal_bl_tr:
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
                assert(!border->diagonal_bl_tr.border_color);
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

void test_xls_xml_cell_borders_colors()
{
    ORCUS_TEST_FUNC_SCOPE;

    using spreadsheet::color_t;
    using spreadsheet::border_style_t;

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/borders/colors.xml");

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

    const spreadsheet::cell_format_t* cf = styles.get_cell_format(xf);
    assert(cf);
    assert(cf->apply_border);

    const spreadsheet::border_t* border = styles.get_border(cf->border);
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

void test_xls_xml_hidden_rows_columns()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/hidden-rows-columns/input.xml");

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

void test_xls_xml_character_set()
{
    ORCUS_TEST_FUNC_SCOPE;

    doc_loader loader(SRCDIR"/test/xls-xml/character-set/input.xml");
    assert(loader.get_factory().get_character_set() == character_set_t::windows_1252);
}

void test_xls_xml_number_format()
{
    ORCUS_TEST_FUNC_SCOPE;

    doc_loader loader(SRCDIR"/test/xls-xml/number-format/date-time.xml");

    const spreadsheet::document& doc = loader.get_doc();
    const spreadsheet::styles& styles = doc.get_styles();

    const spreadsheet::sheet* sh = doc.get_sheet(0);
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
        { 4, 1, "m/d/yyyy h:mm" }, // General Date
        { 5, 1, "d-mmm-yy" }, // Medium Date
        { 6, 1, "m/d/yyyy" }, // Short Date
        { 7, 1, "h:mm:ss AM/PM" }, // Long Time
        { 8, 1, "h:mm AM/PM" }, // Medium Time
        { 9, 1, "h:mm" }, // Short Time
        { 10, 1, "0.00" }, // Fixed
        { 11, 1, "#,##0.00" }, // Standard
        { 12, 1, "0.00%" }, // Percent
        { 13, 1, "0.00E+00" }, // Scientific
        { 14, 1, "\"Yes\";\"Yes\";\"No\"" }, // Yes/No
        { 15, 1, "\"True\";\"True\";\"False\"" }, // True/False
        { 16, 1, "\"On\";\"On\";\"Off\"" }, // On/Off
        { 17, 1, "$#,##0.00_);[Red]($#,##0.00)" }, // Currency
        { 18, 1, "[$\xe2\x82\xac-x-euro2] #,##0.00_);[Red]([$\xe2\x82\xac-x-euro2] #,##0.00)" }, // Euro Currency
    };

    for (const check& c : checks)
    {
        size_t xf = sh->get_cell_format(c.row, c.col);
        const spreadsheet::cell_format_t* cf = styles.get_cell_format(xf);
        assert(cf);

        const spreadsheet::number_format_t* nf = styles.get_number_format(cf->number_format);
        assert(nf);
        assert(nf->format_string == c.expected);
    }
}

void test_xls_xml_cell_properties_wrap_and_shrink()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/cell-properties/wrap-and-shrink.xml");

    const ss::styles& styles = doc->get_styles();
    const ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

    std::size_t xfid = sh->get_cell_format(0, 1); // B1
    const ss::cell_format_t* xf = styles.get_cell_format(xfid);
    assert(xf);
    assert(xf->wrap_text);
    assert(!*xf->wrap_text);
    assert(xf->shrink_to_fit);
    assert(!*xf->shrink_to_fit);

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

void test_xls_xml_cell_properties_default_style()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/cell-properties/default-style.xml");

    const ss::color_t black{255, 0, 0, 0};

    const ss::styles& styles = doc->get_styles();
    const ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

    std::size_t xfid_default = sh->get_cell_format(0, 0); // A1
    const ss::cell_format_t* xf = styles.get_cell_format(xfid_default);
    assert(xf);

    // alignments
    assert(xf->hor_align == ss::hor_alignment_t::center);
    assert(xf->ver_align == ss::ver_alignment_t::bottom);

    // font
    const ss::font_t* font_style = styles.get_font(xf->font);
    assert(font_style);
    assert(font_style->name == "DejaVu Sans");
    assert(font_style->size == 12.0);
    assert(font_style->color == black);

    // fill
    const ss::fill_t* fill_style = styles.get_fill(xf->fill);
    assert(fill_style);
    assert(fill_style->pattern_type == ss::fill_pattern_t::solid);
    const ss::color_t fill_color_fg{255, 0xE2, 0xEF, 0xDA};
    assert(fill_style->fg_color == fill_color_fg);

    // border
    const ss::border_t* border_style = styles.get_border(xf->border);
    assert(border_style);

    assert(border_style->bottom.style == ss::border_style_t::dotted);

    // number format
    const ss::number_format_t* numfmt = styles.get_number_format(xf->number_format);
    assert(numfmt);
    assert(numfmt->format_string == "0.0000");

    // protection
    const ss::protection_t* prot = styles.get_protection(xf->protection);
    assert(prot);
    assert(prot->formula_hidden);

    // A1:G6 should all use the default cell style.
    for (ss::row_t row = 0; row <= 5; ++row)
    {
        for (ss::col_t col = 0; col <= 6; ++col)
        {
            assert(sh->get_cell_format(row, col) == xfid_default);
        }
    }
}

void test_xls_xml_cell_properties_locked_and_hidden()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/cell-properties/locked-and-hidden.xml");
    const ixion::model_context& model = doc->get_model_context();

    const ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

    {
        // Check cell string values first.

        struct check_type
        {
            ixion::abs_address_t address;
            std::string_view str;
        };

        const check_type checks[] = {
            // sheet, row, column, expected cell string value
            { { 0, 0, 0 }, "Default (Should be locked but not hidden)" },
            { { 0, 0, 1 }, "Not Locked and not hidden" },
            { { 0, 1, 0 }, "Locked and hidden" },
            { { 0, 1, 1 }, "Not locked and hidden" },
        };

        for (const auto& c : checks)
        {
            ixion::string_id_t sid = model.get_string_identifier(c.address);
            const std::string* s = model.get_string(sid);
            assert(s);
            assert(*s == c.str);
        }
    }

    {
        // Check the cell protection attributes.

        struct check_type
        {
            ss::row_t row;
            ss::col_t col;
            bool locked;
            bool formula_hidden;
        };

        const check_type checks[] = {
            // row, column, locked, formula-hidden
            { 0, 0, true, false },
            { 0, 1, false, false },
            { 1, 0, true, true },
            { 1, 1, false, true },
        };

        const ss::styles& styles = doc->get_styles();

        for (const auto& c : checks)
        {
            std::cout << "row=" << c.row << "; col=" << c.col << std::endl;

            std::size_t xfid = sh->get_cell_format(c.row, c.col);
            const ss::cell_format_t* xf = styles.get_cell_format(xfid);
            assert(xf);

            const ss::protection_t* prot = styles.get_protection(xf->protection);
            assert(prot);
            assert(prot->locked);
            assert(prot->formula_hidden);
            std::cout << "  * locked: expected=" << c.locked << "; actual=" << *prot->locked << std::endl;
            assert(*prot->locked == c.locked);
            std::cout << "  * formula-hidden: expected=" << c.formula_hidden << "; actual=" << *prot->formula_hidden << std::endl;
            assert(*prot->formula_hidden == c.formula_hidden);
        }
    }
}

void test_xls_xml_styles_direct_format()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string path{SRCDIR"/test/xls-xml/styles/direct-format.xml"};
    auto doc = load_doc_from_filepath(path, false, ss::formula_error_policy_t::fail);
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

    const auto* border = styles.get_border(xf->border);
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
    assert(font->color);
    assert(*font->color == ss::color_t(0xFF, 0x37, 0x56, 0x23));

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

void test_xls_xml_styles_column_styles()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string path{SRCDIR"/test/xls-xml/styles/column-styles.xml"};
    auto doc = load_doc_from_filepath(path, false, ss::formula_error_policy_t::fail);
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

void test_xls_xml_styles_data_offset()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string path{SRCDIR"/test/xls-xml/styles/data-offset.xml"};
    auto doc = load_doc_from_filepath(path, false, ss::formula_error_policy_t::fail);
    assert(doc);

    const ss::styles& styles = doc->get_styles();

    const ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

    auto check_font_style1 = [sh, &styles](ss::row_t row, ss::col_t col)
    {
        std::size_t xfid = sh->get_cell_format(row, col);
        const ss::cell_format_t* xf = styles.get_cell_format(xfid);
        assert(xf);

        const ss::font_t* font = styles.get_font(xf->font);
        assert(font);
        assert(font->bold);
        assert(font->color);
        assert(*font->color == ss::color_t(0xFF, 0x00, 0x80, 0x00));
        const ss::number_format_t* numfmt = styles.get_number_format(xf->number_format);
        assert(numfmt);
        assert(numfmt->format_string);
        assert(*numfmt->format_string == "0.00_ ;[Red]\\-0.00\\ ");
    };

    auto check_font_style2 = [sh, &styles](ss::row_t row, ss::col_t col)
    {
        const ss::color_t red{0xFF, 0xFF, 0x00, 0x00};

        std::size_t xfid = sh->get_cell_format(row, col);
        const ss::cell_format_t* xf = styles.get_cell_format(xfid);
        assert(xf);

        const ss::font_t* font = styles.get_font(xf->font);
        assert(font);
        assert(font->color);
        assert(*font->color == red);
    };

    auto check_font_style3 = [sh, &styles](ss::row_t row, ss::col_t col)
    {
        const ss::color_t blue{0xFF, 0x00, 0x00, 0xFF};

        std::size_t xfid = sh->get_cell_format(row, col);
        const ss::cell_format_t* xf = styles.get_cell_format(xfid);
        assert(xf);

        const ss::font_t* font = styles.get_font(xf->font);
        assert(font);
        assert(font->color);
        assert(*font->color == blue);

        const ss::number_format_t* numfmt = styles.get_number_format(xf->number_format);
        assert(numfmt);
        assert(numfmt->format_string);
        assert(*numfmt->format_string == "yyyy/mm\\-dd;@");
    };

    // Column B and row 2 should have font with bold, green-ish with number format applied
    check_font_style1(0, 1);
    check_font_style1(1, 1);
    check_font_style1(2, 1);
    check_font_style1(3, 1);

    // row 2
    check_font_style1(1, 0);
    check_font_style1(1, 1);
    check_font_style1(1, 2);
    check_font_style1(1, 3);

    // Column C should have red font (except for row 2)
    check_font_style2(0, 2);
    check_font_style2(2, 2);
    check_font_style2(3, 2);

    // Column D should have blue font with custom number format applied (except for row 2)
    check_font_style3(0, 3);
    check_font_style3(2, 3);
    check_font_style3(3, 3);
}

void test_xls_xml_view_cursor_per_sheet()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string path(SRCDIR"/test/xls-xml/view/cursor-per-sheet.xml");

    spreadsheet::range_size_t ss{1048576, 16384};
    spreadsheet::document doc{ss};
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
    spreadsheet::iface::import_reference_resolver* resolver =
        factory.get_reference_resolver(spreadsheet::formula_ref_context_t::global);
    assert(resolver);

    // On Sheet1, the cursor should be set to C4.
    spreadsheet::range_t expected = to_rc_range(resolver->resolve_range("R4C3"));
    spreadsheet::range_t actual = sv->get_selection(spreadsheet::sheet_pane_t::top_left);
    assert(expected == actual);

    sv = view.get_sheet_view(1);
    assert(sv);

    // On Sheet2, the cursor should be set to D8.
    expected = to_rc_range(resolver->resolve_range("R8C4"));
    actual = sv->get_selection(spreadsheet::sheet_pane_t::top_left);
    assert(expected == actual);

    sv = view.get_sheet_view(2);
    assert(sv);

    // On Sheet3, the cursor should be set to D2.
    expected = to_rc_range(resolver->resolve_range("R2C4"));
    actual = sv->get_selection(spreadsheet::sheet_pane_t::top_left);
    assert(expected == actual);

    sv = view.get_sheet_view(3);
    assert(sv);

    // On Sheet4, the cursor should be set to C5:E8.
    expected = to_rc_range(resolver->resolve_range("R5C3:R8C5"));
    actual = sv->get_selection(spreadsheet::sheet_pane_t::top_left);
    assert(expected == actual);
}

struct expected_selection
{
    spreadsheet::sheet_pane_t pane;
    std::string_view sel;
};

void test_xls_xml_view_cursor_split_pane()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string path(SRCDIR"/test/xls-xml/view/cursor-split-pane.xml");

    spreadsheet::range_size_t ss{1048576, 16384};
    spreadsheet::document doc{ss};
    spreadsheet::view view(doc);
    spreadsheet::import_factory factory(doc, view);
    orcus_xls_xml app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    // NB : the resolver type is set to R1C1 for Excel XML 2003.
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
        spreadsheet::address_t expected = to_rc_address(resolver->resolve_address("R6C6"));
        spreadsheet::address_t actual = sv->get_split_pane().top_left_cell;
        assert(expected == actual);
    }

    std::vector<expected_selection> expected_selections =
    {
        { spreadsheet::sheet_pane_t::top_left,     "R4C5"   },
        { spreadsheet::sheet_pane_t::top_right,    "R2C10"  },
        { spreadsheet::sheet_pane_t::bottom_left,  "R8C1"   },
        { spreadsheet::sheet_pane_t::bottom_right, "R17C10" },
    };

    for (const expected_selection& es : expected_selections)
    {
        // cursor in the top-left pane.
        spreadsheet::range_t expected = to_rc_range(resolver->resolve_range(es.sel));
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
        spreadsheet::address_t expected = to_rc_address(resolver->resolve_address("R8C6"));
        spreadsheet::address_t actual = sv->get_split_pane().top_left_cell;
        assert(expected == actual);
    }

    expected_selections =
    {
        { spreadsheet::sheet_pane_t::top_left,     "R2C3:R6C3"    },
        { spreadsheet::sheet_pane_t::top_right,    "R2C8:R2C12"   },
        { spreadsheet::sheet_pane_t::bottom_left,  "R18C2:R23C3"  },
        { spreadsheet::sheet_pane_t::bottom_right, "R11C8:R13C10" },
    };

    for (const expected_selection& es : expected_selections)
    {
        // cursor in the top-left pane.
        spreadsheet::range_t expected = to_rc_range(resolver->resolve_range(es.sel));
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
        spreadsheet::address_t expected = to_rc_address(resolver->resolve_address("R5C1"));
        spreadsheet::address_t actual = sv->get_split_pane().top_left_cell;
        assert(expected == actual);
    }

    expected_selections =
    {
        { spreadsheet::sheet_pane_t::top_left,     "R2C4" },
        { spreadsheet::sheet_pane_t::bottom_left,  "R9C3" },
    };

    for (const expected_selection& es : expected_selections)
    {
        // cursor in the top-left pane.
        spreadsheet::range_t expected = to_rc_range(resolver->resolve_range(es.sel));
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
        spreadsheet::address_t expected = to_rc_address(resolver->resolve_address("R1C5"));
        spreadsheet::address_t actual = sv->get_split_pane().top_left_cell;
        assert(expected == actual);
    }

    expected_selections =
    {
        { spreadsheet::sheet_pane_t::top_left,  "R18C2" },
        { spreadsheet::sheet_pane_t::top_right, "R11C9" },
    };

    for (const expected_selection& es : expected_selections)
    {
        // cursor in the top-left pane.
        spreadsheet::range_t expected = to_rc_range(resolver->resolve_range(es.sel));
        spreadsheet::range_t actual = sv->get_selection(es.pane);
        assert(expected == actual);
    }
}

void test_xls_xml_view_frozen_pane()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string path(SRCDIR"/test/xls-xml/view/frozen-pane.xml");

    spreadsheet::range_size_t ss{1048576, 16384};
    spreadsheet::document doc{ss};
    spreadsheet::view view(doc);
    spreadsheet::import_factory factory(doc, view);
    orcus_xls_xml app(&factory);
    app.set_config(test_config);

    app.read_file(path.c_str());

    // NB : the resolver type is set to R1C1 for Excel XML 2003.
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
        assert(fp.top_left_cell == to_rc_address(resolver->resolve_address("R1C2")));
        assert(fp.visible_columns == 1);
        assert(fp.visible_rows == 0);
        assert(sv->get_active_pane() == spreadsheet::sheet_pane_t::top_right);
    }

    sv = view.get_sheet_view(1);
    assert(sv);

    {
        // Sheet2 is horizontally frozen between rows 1 and 2.
        const spreadsheet::frozen_pane_t& fp = sv->get_frozen_pane();
        assert(fp.top_left_cell == to_rc_address(resolver->resolve_address("R2C1")));
        assert(fp.visible_columns == 0);
        assert(fp.visible_rows == 1);
        assert(sv->get_active_pane() == spreadsheet::sheet_pane_t::bottom_left);
    }

    sv = view.get_sheet_view(2);
    assert(sv);

    {
        // Sheet3 is frozen both horizontally and vertically.
        const spreadsheet::frozen_pane_t& fp = sv->get_frozen_pane();
        assert(fp.top_left_cell == to_rc_address(resolver->resolve_address("R9C5")));
        assert(fp.visible_columns == 4);
        assert(fp.visible_rows == 8);
        assert(sv->get_active_pane() == spreadsheet::sheet_pane_t::bottom_right);
    }
}

void test_xls_xml_skip_error_cells()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string path(SRCDIR"/test/xls-xml/formula-cells-parse-error/input.xml");

    try
    {
        auto doc = load_doc_from_filepath(path, false, ss::formula_error_policy_t::fail);
        (void)doc;
        assert(!"exception was expected, but was not thrown.");
    }
    catch (const std::exception&)
    {
        // works as expected
    }

    auto doc = load_doc_from_filepath(path, false, ss::formula_error_policy_t::skip);
    const ixion::model_context& cxt = doc->get_model_context();

    auto is_formula_cell_with_error = [&cxt](const ixion::abs_address_t& pos) -> bool
    {
        const ixion::formula_cell* fc = cxt.get_formula_cell(pos);
        if (!fc)
            return false;

        const ixion::formula_tokens_t& tokens = fc->get_tokens()->get();
        if (tokens.empty())
            return false;

        return tokens[0].opcode == ixion::fop_invalid_formula;
    };

    // Make sure these two cells have been imported as formula cells with error.
    assert(is_formula_cell_with_error(ixion::abs_address_t(0, 1, 1)));
    assert(is_formula_cell_with_error(ixion::abs_address_t(0, 4, 0)));
}

/**
 * The test input file starts with double BOM's.
 */
void test_xls_xml_double_bom()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string path(SRCDIR"/test/xls-xml/double-bom/input.xml");

    // Make sure the test file does contain double BOM's.
    orcus::file_content content{path};
    auto content_s = content.str();

    constexpr std::string_view BOM = "\xef\xbb\xbf";
    assert(content_s.substr(0, 3) == BOM);
    assert(content_s.substr(3, 3) == BOM);

    auto doc = load_doc_from_filepath(path, false, ss::formula_error_policy_t::fail);

    assert(doc->get_sheet_count() == 5u);
    assert(doc->get_sheet_name(0) == "Holdings");
    assert(doc->get_sheet_name(1) == "Overview");
    assert(doc->get_sheet_name(2) == "Historical");
    assert(doc->get_sheet_name(3) == "Performance");
    assert(doc->get_sheet_name(4) == "Distributions");
}

} // anonymous namespace

int main()
{
    test_config.debug = false;
    test_config.structure_check = true;

    test_xls_xml_detection();
    test_xls_xml_create_filter();
    test_xls_xml_import();
    test_xls_xml_merged_cells();
    test_xls_xml_date_time();
    test_xls_xml_bold_and_italic();
    test_xls_xml_colored_text();
    test_xls_xml_formatted_text_basic();
    test_xls_xml_column_width_row_height();
    test_xls_xml_background_fill();
    test_xls_xml_named_colors();
    test_xls_xml_text_alignment();
    test_xls_xml_cell_borders_single_cells();
    test_xls_xml_cell_borders_directions();
    test_xls_xml_cell_borders_colors();
    test_xls_xml_hidden_rows_columns();
    test_xls_xml_character_set();
    test_xls_xml_number_format();
    test_xls_xml_cell_properties_wrap_and_shrink();
    test_xls_xml_cell_properties_default_style();
    test_xls_xml_cell_properties_locked_and_hidden();
    test_xls_xml_styles_direct_format();
    test_xls_xml_styles_column_styles();
    test_xls_xml_styles_data_offset();

    // view import
    test_xls_xml_view_cursor_per_sheet();
    test_xls_xml_view_cursor_split_pane();
    test_xls_xml_view_frozen_pane();

    test_xls_xml_skip_error_cells();
    test_xls_xml_double_bom();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

