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
    SRCDIR"/test/xls-xml/formula-cells-1/",
    SRCDIR"/test/xls-xml/formula-cells-2/",
    SRCDIR"/test/xls-xml/leading-whitespace/",
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

class doc_loader
{
    spreadsheet::document m_doc;
    spreadsheet::import_factory m_factory;

public:
    doc_loader(const pstring& path) :
        m_doc(), m_factory(m_doc)
    {
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

void update_config(spreadsheet::document& doc, const string& path)
{
    try
    {
        spreadsheet::document_config cfg = doc.get_config();

        yaml::document_tree config;
        config.load(load_file_content(path.data()));
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

void test_xls_xml_import()
{
    for (const char* dir : dirs)
    {
        cout << dir << endl;

        string path(dir);

        // Read the input.xml document.
        path.append("input.xml");
        std::unique_ptr<spreadsheet::document> doc = load_doc(path);

        path = dir;
        path.append("config.yaml");
        update_config(*doc, path);

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

    const spreadsheet::styles& styles = doc->get_styles();

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
    const spreadsheet::cell_format_t* cf = styles.get_cell_format(xfi);
    assert(cf);
    const spreadsheet::font_t* font = styles.get_font(cf->font);
    assert(font);
    assert(font->bold);
    assert(!font->italic);

    // A3 contains italic text.
    si = sheet1->get_string_identifier(2, 0);
    sp = ss->get_string(si);
    assert(sp);
    assert(*sp == "Italic Text");

    xfi = sheet1->get_cell_format(2, 0);
    cf = styles.get_cell_format(xfi);
    assert(cf);
    font = styles.get_font(cf->font);
    assert(font);
    assert(!font->bold);
    assert(font->italic);

    // A4 contains bold and italic text.
    si = sheet1->get_string_identifier(3, 0);
    sp = ss->get_string(si);
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

void test_xls_xml_text_alignment()
{
    pstring path(SRCDIR"/test/xls-xml/text-alignment/input.xml");
    std::unique_ptr<spreadsheet::document> doc = load_doc(path.str());

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

void test_xls_xml_cell_borders_single_cells()
{
    pstring path(SRCDIR"/test/xls-xml/borders/single-cells.xml");
    cout << path << endl;
    std::unique_ptr<spreadsheet::document> doc = load_doc(path.str());

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
        cout << "(row: " << c.row << "; col: " << c.col << "; expected: " << int(c.style) << ")" << endl;
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
    pstring path(SRCDIR"/test/xls-xml/borders/directions.xml");
    cout << path << endl;
    std::unique_ptr<spreadsheet::document> doc = load_doc(path.str());

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
                assert(border->diagonal.style       == spreadsheet::border_style_t::unknown);
                assert(border->diagonal_bl_tr.style == spreadsheet::border_style_t::thin);
                assert(border->diagonal_tl_br.style == spreadsheet::border_style_t::thin);
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

void test_xls_xml_cell_borders_colors()
{
    using spreadsheet::color_t;
    using spreadsheet::border_style_t;

    pstring path(SRCDIR"/test/xls-xml/borders/colors.xml");
    cout << path << endl;
    std::unique_ptr<spreadsheet::document> doc = load_doc(path.str());

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
    pstring path(SRCDIR"/test/xls-xml/hidden-rows-columns/input.xml");
    cout << path << endl;
    std::unique_ptr<spreadsheet::document> doc = load_doc(path.str());

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
    assert(row_end == sh->row_size()); // the end position is non-inclusive.

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
    assert(col_end == sh->col_size()); // non-inclusive
}

void test_xls_xml_character_set()
{
    pstring path(SRCDIR"/test/xls-xml/character-set/input.xml");
    cout << path << endl;
    doc_loader loader(path);

    assert(loader.get_factory().get_character_set() == character_set_t::windows_1252);
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
    test_xls_xml_text_alignment();
    test_xls_xml_cell_borders_single_cells();
    test_xls_xml_cell_borders_directions();
    test_xls_xml_cell_borders_colors();
    test_xls_xml_hidden_rows_columns();
    test_xls_xml_character_set();

    // view import
    test_xls_xml_view_cursor_per_sheet();
    test_xls_xml_view_cursor_split_pane();
    test_xls_xml_view_frozen_pane();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

