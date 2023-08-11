/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include <orcus/orcus_import_ods.hpp>

#include <orcus/measurement.hpp>
#include <orcus/spreadsheet/factory.hpp>
#include <orcus/spreadsheet/styles.hpp>
#include <orcus/spreadsheet/types.hpp>
#include <orcus/stream.hpp>
#include <orcus/string_pool.hpp>

#include <cassert>
#include <iostream>
#include <sstream>

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

namespace ss = orcus::spreadsheet;

using orcus::test::stack_printer;

namespace {

struct test_model
{
    orcus::string_pool pool;
    orcus::file_content content;
    ss::styles styles;
    ss::import_styles istyles;

    test_model() : styles(), istyles(styles, pool) {}

    void load(const fs::path& input_path)
    {
        if (!fs::is_regular_file(input_path))
        {
            std::ostringstream os;
            os << input_path << " is not a regular file.";
            throw std::runtime_error(os.str());
        }

        styles.clear();
        content.load(input_path.string());
        orcus::import_ods::read_styles(content.str(), &istyles);
    }
};

bool verify_font_attrs(const ss::font_t& expected, const ss::font_t& actual)
{
    if (expected.name != actual.name)
    {
        std::cerr << "font name states differ!" << std::endl;
        return false;
    }

    if (expected.name && *expected.name != *actual.name)
    {
        std::cerr << "font names differ!" << std::endl;
        return false;
    }

    if (expected.size != actual.size)
    {
        std::cerr << "font size states differ!" << std::endl;
        return false;
    }

    if (expected.size && *expected.size != *actual.size)
    {
        std::cerr << "font sizes differ!" << std::endl;
        return false;
    }

    if (expected.bold != actual.bold)
    {
        std::cerr << "font bold states differ!" << std::endl;
        return false;
    }

    if (expected.bold && *expected.bold != *actual.bold)
    {
        std::cerr << "font bold values differ!" << std::endl;
        return false;
    }

    if (expected.italic != actual.italic)
    {
        std::cerr << "font italic states differ!" << std::endl;
        return false;
    }

    if (expected.italic && *expected.italic != *actual.italic)
    {
        std::cerr << "font italic values differ!" << std::endl;
        return false;
    }

    if (expected.underline_style != actual.underline_style)
    {
        std::cerr << "underline_style states differ!" << std::endl;
        return false;
    }

    if (expected.underline_style && *expected.underline_style != *actual.underline_style)
    {
        std::cerr << "underline_style values differ!" << std::endl;
        return false;
    }

    if (expected.underline_width != actual.underline_width)
    {
        std::cerr << "underline_width states differ!" << std::endl;
        return false;
    }

    if (expected.underline_width && *expected.underline_width != *actual.underline_width)
    {
        std::cerr << "underline_width values differ!" << std::endl;
        return false;
    }

    if (expected.underline_mode != actual.underline_mode)
    {
        std::cerr << "underline_mode states differ!" << std::endl;
        return false;
    }

    if (expected.underline_mode && *expected.underline_mode != *actual.underline_mode)
    {
        std::cerr << "underline_mode values differ!" << std::endl;
        return false;
    }

    if (expected.underline_type != actual.underline_type)
    {
        std::cerr << "underline_type states differ!" << std::endl;
        return false;
    }

    if (expected.underline_type && *expected.underline_type != *actual.underline_type)
    {
        std::cerr << "underline_type values differ!" << std::endl;
        return false;
    }

    if (expected.underline_color != actual.underline_color)
    {
        std::cerr << "underline_color states differ!" << std::endl;
        return false;
    }

    if (expected.underline_color && *expected.underline_color != *actual.underline_color)
    {
        std::cerr << "underline_color values differ!" << std::endl;
        return false;
    }

    if (expected.color != actual.color)
    {
        std::cerr << "font color states differ!" << std::endl;
        return false;
    }

    if (expected.color && *expected.color != *actual.color)
    {
        std::cerr << "font color values differ!" << std::endl;
        return false;
    }

    if (expected.strikethrough_style != actual.strikethrough_style)
    {
        std::cerr << "strikethrough_style states differ!" << std::endl;
        return false;
    }

    if (expected.strikethrough_style && *expected.strikethrough_style != *actual.strikethrough_style)
    {
        std::cerr << "strikethrough_style values differ!" << std::endl;
        return false;
    }

    if (expected.strikethrough_width != actual.strikethrough_width)
    {
        std::cerr << "strikethrough_width states differ!" << std::endl;
        return false;
    }

    if (expected.strikethrough_width && *expected.strikethrough_width != *actual.strikethrough_width)
    {
        std::cerr << "strikethrough_width values differ!" << std::endl;
        return false;
    }

    if (expected.strikethrough_type != actual.strikethrough_type)
    {
        std::cerr << "strikethrough_type states differ!" << std::endl;
        return false;
    }

    if (expected.strikethrough_type && *expected.strikethrough_type != *actual.strikethrough_type)
    {
        std::cerr << "strikethrough_type values differ!" << std::endl;
        return false;
    }

    if (expected.strikethrough_text != actual.strikethrough_text)
    {
        std::cerr << "strikethrough_text states differ!" << std::endl;
        return false;
    }

    if (expected.strikethrough_text && *expected.strikethrough_text != *actual.strikethrough_text)
    {
        std::cerr << "strikethrough_text values differ!" << std::endl;
        return false;
    }

    return true;
}

bool verify_fill_attrs(const ss::fill_t& expected, const ss::fill_t& actual)
{
    if (expected.pattern_type != actual.pattern_type)
    {
        std::cerr << "pattern_type states differ!" << std::endl;
        return false;
    }

    if (expected.pattern_type && *expected.pattern_type != *actual.pattern_type)
    {
        std::cerr << "pattern types differ!" << std::endl;
        return false;
    }

    if (expected.fg_color != actual.fg_color)
    {
        std::cerr << "fg_color states differ!" << std::endl;
        return false;
    }

    if (expected.fg_color && *expected.fg_color != *actual.fg_color)
    {
        std::cerr << "foreground colors differ!" << std::endl;
        return false;
    }

    if (expected.bg_color != actual.bg_color)
    {
        std::cerr << "bg_color states differ!" << std::endl;
        return false;
    }

    if (expected.bg_color && *expected.bg_color != *actual.bg_color)
    {
        std::cerr << "background colors differ!" << std::endl;
        return false;
    }

    return true;
}

bool verify_protection_attrs(const ss::protection_t& expected, const ss::protection_t& actual)
{
    if (expected.locked != actual.locked)
    {
        std::cerr << "locked states differ!" << std::endl;
        return false;
    }

    if (expected.hidden != actual.hidden)
    {
        std::cerr << "hidden states differ!" << std::endl;
        return false;
    }

    if (expected.print_content != actual.print_content)
    {
        std::cerr << "'print content' states differ!" << std::endl;
        return false;
    }

    if (expected.formula_hidden != actual.formula_hidden)
    {
        std::cerr << "'formula hidden' states differ!" << std::endl;
        return false;
    }

    return true;
}

bool verify_border_attrs(const ss::border_t& expected, const ss::border_t& actual)
{
    auto verify_single = [](std::string_view name, const ss::border_attrs_t& _expected, const ss::border_attrs_t& _actual)
    {
        if (_expected.style != _actual.style)
        {
            std::cerr << name << " border style states differ!" << std::endl;
            return false;
        }

        if (_expected.style && *_expected.style != *_actual.style)
        {
            std::cerr << name << " border styles differ!" << std::endl;
            return false;
        }

        if (_expected.border_color != _actual.border_color)
        {
            std::cerr << name << " border color states differ!" << std::endl;
            return false;
        }

        if (_expected.border_color && *_expected.border_color != *_actual.border_color)
        {
            std::cerr << name << " border colors differ!" << std::endl;
            return false;
        }

        if (_expected.border_width != _actual.border_width)
        {
            std::cerr << name << " border width states differ!" << std::endl;
            return false;
        }

        if (_expected.border_width && *_expected.border_width != *_actual.border_width)
        {
            std::cerr << name << " border widths differ!" << std::endl;
            return false;
        }

        return true;
    };

    if (!verify_single("top", expected.top, actual.top))
        return false;

    if (!verify_single("bottom", expected.bottom, actual.bottom))
        return false;

    if (!verify_single("left", expected.left, actual.left))
        return false;

    if (!verify_single("right", expected.right, actual.right))
        return false;

    if (!verify_single("diagonal", expected.diagonal, actual.diagonal))
        return false;

    if (!verify_single("diagonal_bl_tr", expected.diagonal_bl_tr, actual.diagonal_bl_tr))
        return false;

    if (!verify_single("diagonal_tl_br", expected.diagonal_tl_br, actual.diagonal_tl_br))
        return false;

    return true;
}

const ss::cell_style_t* find_cell_style_by_name(
    std::string_view name, const ss::styles& styles)
{
    size_t n = styles.get_cell_styles_count();
    for (size_t i = 0; i < n; ++i)
    {
        const ss::cell_style_t* cur_style = styles.get_cell_style(i);
        if (cur_style->name == name)
            return cur_style;
    }

    std::cerr << "No styles named '" << name << "' found!" << std::endl;
    return nullptr;
}

const ss::cell_format_t* find_cell_format(const ss::styles& styles, std::string_view name, std::string_view parent_name)
{
    const ss::cell_style_t* style = find_cell_style_by_name(name, styles);
    if (!style)
        return nullptr;

    if (style->parent_name != parent_name)
    {
        std::cerr << "Parent name is not as expected: expected='" << parent_name << "'; actual='" << style->parent_name << "'" << std::endl;
        return nullptr;
    }

    return styles.get_cell_style_format(style->xf);
}

void test_odf_fill(const ss::styles& styles)
{
    const ss::cell_style_t* style = find_cell_style_by_name("Name1", styles);
    assert(style);
    assert(style->parent_name == "Text");
    size_t xf = style->xf;
    const ss::cell_format_t* cell_format = styles.get_cell_style_format(xf);
    assert(cell_format);

    size_t fill = cell_format->fill;
    const ss::fill_t* cell_fill = styles.get_fill(fill);
    assert(cell_fill);
    assert(cell_fill->fg_color);
    assert(*cell_fill->fg_color == ss::color_t(0xFF, 0xFE, 0xFF, 0xCC));
    assert(cell_fill->pattern_type);
    assert(*cell_fill->pattern_type == ss::fill_pattern_t::solid);
}

void test_odf_border(const ss::styles &styles)
{
    /* Test that border style applies to all the sides when not specified */
    const ss::cell_style_t* style = find_cell_style_by_name("Name1", styles);
    assert(style);
    const ss::cell_format_t* cell_format = styles.get_cell_style_format(style->xf);
    assert(cell_format);

    const ss::border_t* cell_border = styles.get_border(cell_format->border);
    assert(cell_border->top.style);
    assert(*cell_border->top.style == ss::border_style_t::dotted);
    assert(cell_border->bottom.style);
    assert(*cell_border->bottom.style == ss::border_style_t::dotted);
    assert(cell_border->left.style);
    assert(*cell_border->left.style == ss::border_style_t::dotted);
    assert(cell_border->right.style);
    assert(*cell_border->right.style == ss::border_style_t::dotted);

    ss::color_t expected_color{0xFF, 0xFF, 0xCC, 0x12};
    assert(cell_border->top.border_color);
    assert(*cell_border->top.border_color == expected_color);
    assert(cell_border->bottom.border_color);
    assert(*cell_border->bottom.border_color == expected_color);
    assert(cell_border->left.border_color);
    assert(*cell_border->left.border_color == expected_color);
    assert(cell_border->right.border_color);
    assert(*cell_border->right.border_color == expected_color);

    orcus::length_t expected_width{orcus::length_unit_t::point, 0.06};
    assert(cell_border->top.border_width);
    assert(*cell_border->top.border_width == expected_width);
    assert(cell_border->bottom.border_width);
    assert(*cell_border->bottom.border_width == expected_width);
    assert(cell_border->left.border_width);
    assert(*cell_border->left.border_width == expected_width);
    assert(cell_border->right.border_width);
    assert(*cell_border->right.border_width == expected_width);

    style = find_cell_style_by_name("Name2", styles);
    assert(style);
    cell_format = styles.get_cell_style_format(style->xf);
    assert(cell_format);

    cell_border = styles.get_border(cell_format->border);
    assert(cell_border);
    assert(cell_border->top.style);
    assert(*cell_border->top.style == ss::border_style_t::fine_dashed);
    assert(cell_border->bottom.style);
    assert(*cell_border->bottom.style == ss::border_style_t::double_thin);
    assert(cell_border->left.style);
    assert(*cell_border->left.style == ss::border_style_t::none);
    assert(cell_border->right.style);
    assert(*cell_border->right.style == ss::border_style_t::dash_dot_dot);

    assert(cell_border->top.border_color);
    assert(*cell_border->top.border_color == ss::color_t(0xFF, 0xFF, 0xEE, 0x11));
    assert(cell_border->bottom.border_color);
    assert(*cell_border->bottom.border_color == ss::color_t(0xFF, 0xAE, 0xEE, 0x11));
    assert(cell_border->left.border_color);
    assert(*cell_border->left.border_color == ss::color_t(0xFF, 0x11, 0xEE, 0x11));
    assert(cell_border->right.border_color);
    assert(*cell_border->right.border_color == ss::color_t(0xFF, 0x05, 0xEE, 0x11));

    expected_width.value = 0.74; // point
    assert(cell_border->top.border_width);
    assert(*cell_border->top.border_width == expected_width);
    expected_width.value = 1.74;
    assert(cell_border->bottom.border_width);
    assert(*cell_border->bottom.border_width == expected_width);
    expected_width.value = 0.74;
    assert(cell_border->left.border_width);
    assert(*cell_border->left.border_width == expected_width);
    expected_width.value = 0.22;
    assert(cell_border->right.border_width);
    assert(*cell_border->right.border_width == expected_width);

    /*Test that border applies to the diagonal*/
    style = find_cell_style_by_name("Name3", styles);
    assert(style);
    cell_format = styles.get_cell_style_format(style->xf);
    assert(cell_format);

    cell_border = styles.get_border(cell_format->border);
    assert(cell_border);

    // 1.74pt dashed #ffccee
    assert(cell_border->diagonal_bl_tr.style);
    assert(*cell_border->diagonal_bl_tr.style == ss::border_style_t::dashed);
    assert(cell_border->diagonal_bl_tr.border_color);
    assert(*cell_border->diagonal_bl_tr.border_color == ss::color_t(0xFF, 0xFF, 0xCC, 0xEE));
    expected_width.value = 1.74;
    assert(cell_border->diagonal_bl_tr.border_width);
    assert(*cell_border->diagonal_bl_tr.border_width == expected_width);

    // 0.74pt dash-dot #120000
    assert(cell_border->diagonal_tl_br.style);
    assert(*cell_border->diagonal_tl_br.style == ss::border_style_t::dash_dot);
    assert(cell_border->diagonal_tl_br.border_color);
    assert(*cell_border->diagonal_tl_br.border_color == ss::color_t(0xFF, 0x12, 0x00, 0x00));
    expected_width.value = 0.74;
    assert(cell_border->diagonal_tl_br.border_width);
    assert(*cell_border->diagonal_tl_br.border_width == expected_width);
}

void test_odf_cell_protection(const ss::styles& styles)
{
    /* Test that Cell is only protected and not hidden , Print Content is true */
    const ss::cell_style_t* style = find_cell_style_by_name("Name5", styles);
    assert(style);
    size_t  xf = style->xf;
    const ss::cell_format_t* cell_format = styles.get_cell_style_format(xf);
    size_t protection = cell_format->protection;
    assert(cell_format);

    const ss::protection_t* cell_protection = styles.get_protection(protection);
    assert(*cell_protection->locked == true);
    assert(*cell_protection->hidden == true);
    assert(*cell_protection->print_content == true);
    assert(!cell_protection->formula_hidden);

    /* Test that Cell is  protected and formula is hidden , Print Content is false */
    style = find_cell_style_by_name("Name6", styles);
    assert(style);
    xf = style->xf;
    cell_format = styles.get_cell_style_format(xf);
    protection = cell_format->protection;
    assert(cell_format);

    cell_protection = styles.get_protection(protection);
    assert(*cell_protection->locked == true);
    assert(!cell_protection->hidden); // not set
    assert(*cell_protection->print_content == false);
    assert(*cell_protection->formula_hidden == true);

    /* Test that Cell is not protected by any way, Print Content is false */
    style = find_cell_style_by_name("Name7", styles);
    assert(style);
    xf = style->xf;
    cell_format = styles.get_cell_style_format(xf);
    protection = cell_format->protection;
    assert(cell_format);

    cell_protection = styles.get_protection(protection);
    assert(*cell_protection->locked == false);
    assert(*cell_protection->hidden == false);
    assert(*cell_protection->print_content == true);
    assert(*cell_protection->formula_hidden == false);
}

void test_odf_font(const ss::styles& styles)
{
    const ss::cell_style_t* style = find_cell_style_by_name("Name8", styles);
    assert(style);
    size_t xf = style->xf;
    const ss::cell_format_t* cell_format = styles.get_cell_style_format(xf);
    size_t font = cell_format->font;
    assert(cell_format);

    const ss::font_t* cell_font = styles.get_font(font);
    assert(*cell_font->name == "Liberation Sans");
    assert(*cell_font->size == 24);
    assert(*cell_font->bold == true);
    assert(*cell_font->italic == true);
    assert(*cell_font->underline_style == ss::underline_t::single_line);
    assert(*cell_font->underline_width == ss::underline_width_t::thick);
    assert(!cell_font->underline_mode); // not set
    assert(!cell_font->underline_type); // not set
    assert(cell_font->color->red == (int)0x80);
    assert(cell_font->color->green == (int)0x80);
    assert(cell_font->color->blue == (int)0x80);

    style = find_cell_style_by_name("Name9", styles);
    assert(style);
    xf = style->xf;
    cell_format = styles.get_cell_style_format(xf);
    font = cell_format->font;
    assert(cell_format);

    cell_font = styles.get_font(font);
    assert(*cell_font->name == "Tahoma");
    assert(*cell_font->size == 00);
    assert(*cell_font->bold == true);
    assert(!cell_font->italic);
    assert(*cell_font->underline_style == ss::underline_t::dash);
    assert(*cell_font->underline_width == ss::underline_width_t::bold);
    assert(!cell_font->underline_mode); // not set
    assert(!cell_font->underline_type); // not set
    assert(cell_font->underline_color->red == (int)0x18);
    assert(cell_font->underline_color->green == (int)0x56);
    assert(cell_font->underline_color->blue == (int)0xff);
}

void test_odf_text_strikethrough(const ss::styles& styles)
{
    const ss::cell_style_t* style = find_cell_style_by_name("Name20", styles);
    assert(style);
    size_t xf = style->xf;
    const ss::cell_format_t* cell_format = styles.get_cell_style_format(xf);
    size_t font = cell_format->font;
    assert(cell_format);

    const ss::font_t* cell_font = styles.get_font(font);
    assert(*cell_font->strikethrough_style == ss::strikethrough_style_t::solid);
    assert(!cell_font->strikethrough_width); // not set
    assert(*cell_font->strikethrough_type == ss::strikethrough_type_t::single_type);
    assert(!cell_font->strikethrough_text); // not set

    style = find_cell_style_by_name("Name21", styles);
    assert(style);
    xf = style->xf;
    cell_format = styles.get_cell_style_format(xf);
    font = cell_format->font;
    assert(cell_format);

    cell_font = styles.get_font(font);
    assert(*cell_font->strikethrough_style == ss::strikethrough_style_t::solid);
    assert(*cell_font->strikethrough_width == ss::strikethrough_width_t::bold);
    assert(*cell_font->strikethrough_type == ss::strikethrough_type_t::single_type);
    assert(!cell_font->strikethrough_text); // not set

    style = find_cell_style_by_name("Name22", styles);
    assert(style);
    xf = style->xf;
    cell_format = styles.get_cell_style_format(xf);
    font = cell_format->font;
    assert(cell_format);

    cell_font = styles.get_font(font);
    assert(*cell_font->strikethrough_style == ss::strikethrough_style_t::solid);
    assert(!cell_font->strikethrough_width); // not set
    assert(*cell_font->strikethrough_type == ss::strikethrough_type_t::single_type);
    assert(*cell_font->strikethrough_text == ss::strikethrough_text_t::slash);
}

void test_odf_text_alignment(const ss::styles& styles)
{
    const ss::cell_style_t* style = find_cell_style_by_name("Name23", styles);
    assert(style);
    size_t xf = style->xf;
    const ss::cell_format_t* cell_format = styles.get_cell_style_format(xf);
    assert(cell_format);

    assert(cell_format->hor_align == ss::hor_alignment_t::right);
    assert(cell_format->ver_align == ss::ver_alignment_t::middle);
}

void test_cell_styles()
{
    stack_printer __sp__(__func__);

    test_model model;

    model.load(SRCDIR"/test/ods/import-styles/cell-styles.xml");
    test_odf_fill(model.styles);
    test_odf_border(model.styles);
    test_odf_cell_protection(model.styles);
    test_odf_font(model.styles);
    test_odf_text_strikethrough(model.styles);
    test_odf_text_alignment(model.styles);
}

void test_standard_styles()
{
    stack_printer __sp__(__func__);

    test_model model;
    model.load(SRCDIR"/test/ods/import-styles/standard-styles.xml");

    {
        // Heading only specifies color, font size, font style and font weight.
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Heading", "Default");
        assert(cell_format);

        ss::font_t expected;
        expected.size = 24;
        expected.bold = true;
        expected.italic = false;
        expected.color = ss::color_t(0, 0, 0);

        const ss::font_t* actual = model.styles.get_font(cell_format->font);
        assert(actual);
        assert(verify_font_attrs(expected, *actual));
    }

    {
        // Heading 1 only overwrites font size.
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Heading 1", "Heading");
        assert(cell_format);

        ss::font_t expected;
        expected.size = 18;

        const ss::font_t* actual = model.styles.get_font(cell_format->font);
        assert(actual);
        assert(verify_font_attrs(expected, *actual));
    }

    {
        // Heading 2 also only overwrites font size but with a different size.
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Heading 2", "Heading");
        assert(cell_format);

        ss::font_t expected;
        expected.size = 12;

        const ss::font_t* actual = model.styles.get_font(cell_format->font);
        assert(actual);
        assert(verify_font_attrs(expected, *actual));
    }

    {
        // Text simply inherits from Default.
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Text", "Default");
        assert(cell_format);

        ss::font_t expected; // nothing is active

        const ss::font_t* actual = model.styles.get_font(cell_format->font);
        assert(actual);
        assert(verify_font_attrs(expected, *actual));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Note", "Text");
        assert(cell_format);

        ss::font_t expected;
        expected.color = ss::color_t(0x33, 0x33, 0x33);

        const ss::font_t* actual = model.styles.get_font(cell_format->font);
        assert(actual);
        assert(verify_font_attrs(expected, *actual));

        ss::fill_t expected_fill;
        expected_fill.pattern_type = ss::fill_pattern_t::solid;
        expected_fill.fg_color = ss::color_t(0xff, 0xff, 0xcc);

        const ss::fill_t* actual_fill = model.styles.get_fill(cell_format->fill);
        assert(actual_fill);
        assert(verify_fill_attrs(expected_fill, *actual_fill));

        // fo:border="0.75pt solid #808080" -> same border attributes applied to top, bottom, left and right borders.
        ss::border_t expected_border;
        expected_border.top.style = ss::border_style_t::solid;
        expected_border.top.border_width = orcus::length_t{orcus::length_unit_t::point, 0.75};
        expected_border.top.border_color = ss::color_t(0xFF, 0x80, 0x80, 0x80);

        expected_border.bottom = expected_border.top;
        expected_border.left = expected_border.top;
        expected_border.right = expected_border.top;

        const auto* actual_border = model.styles.get_border(cell_format->border);
        assert(actual_border);
        assert(verify_border_attrs(expected_border, *actual_border));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Footnote", "Text");
        assert(cell_format);

        ss::font_t expected;
        expected.color = ss::color_t(0x80, 0x80, 0x80);
        expected.italic = true;

        const ss::font_t* actual = model.styles.get_font(cell_format->font);
        assert(actual);
        assert(verify_font_attrs(expected, *actual));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Hyperlink", "Text");
        assert(cell_format);

        // TODO: Since we currently handle text-underline-width and
        // text-underline-color incorrectly, we cannot perform exhaustive check.
        // We can only check the attributes individually.

        const ss::font_t* actual = model.styles.get_font(cell_format->font);
        assert(actual);

        // fo:color="#0000ee"
        assert(actual->color);
        assert(*actual->color == ss::color_t(0x00, 0x00, 0xee));

        // style:text-underline-style="solid"
        assert(actual->underline_style);
        assert(*actual->underline_style == ss::underline_t::single_line); // solid

        // style:text-underline-width="auto"
        assert(actual->underline_width);
        assert(*actual->underline_width == ss::underline_width_t::automatic);

        // style:text-underline-color="font-color" (use the same color as the font)
        assert(!actual->underline_color); // this implies the same color as the font
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Status", "Default");
        assert(cell_format);
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Good", "Status");
        assert(cell_format);

        ss::font_t expected;
        expected.color = ss::color_t(0x00, 0x66, 0x00);

        const ss::font_t* actual = model.styles.get_font(cell_format->font);
        assert(actual);
        assert(verify_font_attrs(expected, *actual));

        ss::fill_t expected_fill;
        expected_fill.pattern_type = ss::fill_pattern_t::solid;
        expected_fill.fg_color = ss::color_t(0xcc, 0xff, 0xcc);

        const ss::fill_t* actual_fill = model.styles.get_fill(cell_format->fill);
        assert(actual_fill);
        assert(verify_fill_attrs(expected_fill, *actual_fill));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Neutral", "Status");
        assert(cell_format);

        ss::font_t expected;
        expected.color = ss::color_t(0x99, 0x66, 0x00);

        const ss::font_t* actual = model.styles.get_font(cell_format->font);
        assert(actual);
        assert(verify_font_attrs(expected, *actual));

        ss::fill_t expected_fill;
        expected_fill.pattern_type = ss::fill_pattern_t::solid;
        expected_fill.fg_color = ss::color_t(0xff, 0xff, 0xcc);

        const ss::fill_t* actual_fill = model.styles.get_fill(cell_format->fill);
        assert(actual_fill);
        assert(verify_fill_attrs(expected_fill, *actual_fill));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Bad", "Status");
        assert(cell_format);

        ss::font_t expected;
        expected.color = ss::color_t(0xcc, 0x00, 0x00);

        const ss::font_t* actual = model.styles.get_font(cell_format->font);
        assert(actual);
        assert(verify_font_attrs(expected, *actual));

        ss::fill_t expected_fill;
        expected_fill.pattern_type = ss::fill_pattern_t::solid;
        expected_fill.fg_color = ss::color_t(0xff, 0xcc, 0xcc);

        const ss::fill_t* actual_fill = model.styles.get_fill(cell_format->fill);
        assert(actual_fill);
        assert(verify_fill_attrs(expected_fill, *actual_fill));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Warning", "Status");
        assert(cell_format);

        ss::font_t expected;
        expected.color = ss::color_t(0xcc, 0x00, 0x00);

        const ss::font_t* actual = model.styles.get_font(cell_format->font);
        assert(actual);
        assert(verify_font_attrs(expected, *actual));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Error", "Status");
        assert(cell_format);

        ss::font_t expected;
        expected.color = ss::color_t(0xff, 0xff, 0xff);
        expected.bold = true;

        const ss::font_t* actual = model.styles.get_font(cell_format->font);
        assert(actual);
        assert(verify_font_attrs(expected, *actual));

        ss::fill_t expected_fill;
        expected_fill.pattern_type = ss::fill_pattern_t::solid;
        expected_fill.fg_color = ss::color_t(0xcc, 0x00, 0x00);

        const ss::fill_t* actual_fill = model.styles.get_fill(cell_format->fill);
        assert(actual_fill);
        assert(verify_fill_attrs(expected_fill, *actual_fill));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Accent", "Default");
        assert(cell_format);

        ss::font_t expected;
        expected.bold = true;

        const ss::font_t* actual = model.styles.get_font(cell_format->font);
        assert(actual);
        assert(verify_font_attrs(expected, *actual));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Accent 1", "Accent");
        assert(cell_format);

        ss::font_t expected;
        expected.color = ss::color_t(0xff, 0xff, 0xff);

        const ss::font_t* actual = model.styles.get_font(cell_format->font);
        assert(actual);
        assert(verify_font_attrs(expected, *actual));

        ss::fill_t expected_fill;
        expected_fill.pattern_type = ss::fill_pattern_t::solid;
        expected_fill.fg_color = ss::color_t(0x00, 0x00, 0x00);

        const ss::fill_t* actual_fill = model.styles.get_fill(cell_format->fill);
        assert(actual_fill);
        assert(verify_fill_attrs(expected_fill, *actual_fill));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Accent 2", "Accent");
        assert(cell_format);

        ss::font_t expected;
        expected.color = ss::color_t(0xff, 0xff, 0xff);

        const ss::font_t* actual = model.styles.get_font(cell_format->font);
        assert(actual);
        assert(verify_font_attrs(expected, *actual));

        ss::fill_t expected_fill;
        expected_fill.pattern_type = ss::fill_pattern_t::solid;
        expected_fill.fg_color = ss::color_t(0x80, 0x80, 0x80);

        const ss::fill_t* actual_fill = model.styles.get_fill(cell_format->fill);
        assert(actual_fill);
        assert(verify_fill_attrs(expected_fill, *actual_fill));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Accent 3", "Accent");
        assert(cell_format);

        ss::fill_t expected_fill;
        expected_fill.pattern_type = ss::fill_pattern_t::solid;
        expected_fill.fg_color = ss::color_t(0xdd, 0xdd, 0xdd);

        const ss::fill_t* actual_fill = model.styles.get_fill(cell_format->fill);
        assert(actual_fill);
        assert(verify_fill_attrs(expected_fill, *actual_fill));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Result", "Default");
        assert(cell_format);

        ss::font_t expected;
        expected.bold = true;
        expected.italic = true;
        expected.underline_style = ss::underline_t::single_line;

        const ss::font_t* actual = model.styles.get_font(cell_format->font);
        assert(actual);
        assert(verify_font_attrs(expected, *actual));
    }
}

void test_cell_protection_styles()
{
    stack_printer __sp__(__func__);

    test_model model;
    model.load(SRCDIR"/test/ods/import-styles/cell-protection.xml");

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Hide_20_Formula", "Protected");
        assert(cell_format);

        const auto* protection = model.styles.get_protection(cell_format->protection);
        assert(protection);

        ss::protection_t expected;
        expected.locked = true;
        expected.formula_hidden = true;
        expected.print_content = true;

        assert(verify_protection_attrs(expected, *protection));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Hide_20_When_20_Printing", "Protected");
        assert(cell_format);

        const auto* protection = model.styles.get_protection(cell_format->protection);
        assert(protection);

        ss::protection_t expected;
        expected.locked = true;
        expected.print_content = false;

        assert(verify_protection_attrs(expected, *protection));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Hide_20_All", "Protected");
        assert(cell_format);

        const auto* protection = model.styles.get_protection(cell_format->protection);
        assert(protection);

        ss::protection_t expected;
        expected.locked = true;
        expected.hidden = true;
        expected.print_content = true;

        assert(verify_protection_attrs(expected, *protection));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Not_20_Protected", "Default");
        assert(cell_format);

        const auto* protection = model.styles.get_protection(cell_format->protection);
        assert(protection);

        ss::protection_t expected;
        expected.locked = false;
        expected.hidden = false;
        expected.print_content = true;
        expected.formula_hidden = false;

        assert(verify_protection_attrs(expected, *protection));
    }
}

} // anonymous namespace

int main()
{
    test_cell_styles();
    test_standard_styles();
    test_cell_protection_styles();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
