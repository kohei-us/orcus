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

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;
namespace ss = orcus::spreadsheet;

using orcus::test::stack_printer;

namespace {

struct test_model
{
    orcus::string_pool pool;
    orcus::file_content content;
    orcus::spreadsheet::styles styles;
    orcus::spreadsheet::import_styles istyles;

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

bool verify_active_font_attrs(
    const std::pair<ss::font_t, ss::font_active_t>& expected,
    const std::pair<ss::font_t, ss::font_active_t>& actual)
{
    if (expected.second != actual.second)
    {
        std::cerr << "active masks differ!" << std::endl;
        return false;
    }

    const ss::font_active_t& mask = expected.second;

    if (mask.name && expected.first.name != actual.first.name)
    {
        std::cerr << "font names differ!" << std::endl;
        return false;
    }

    if (mask.size && expected.first.size != actual.first.size)
    {
        std::cerr << "font sizes differ!" << std::endl;
        return false;
    }

    if (mask.bold && expected.first.bold != actual.first.bold)
    {
        std::cerr << "font boldnesses differ!" << std::endl;
        return false;
    }

    if (mask.italic && expected.first.italic != actual.first.italic)
    {
        std::cerr << "font italic flags differ!" << std::endl;
        return false;
    }

    if (mask.underline_style && expected.first.underline_style != actual.first.underline_style)
    {
        std::cerr << "underline styles differ!" << std::endl;
        return false;
    }

    if (mask.underline_width && expected.first.underline_width != actual.first.underline_width)
    {
        std::cerr << "underline widths differ!" << std::endl;
        return false;
    }

    if (mask.underline_mode && expected.first.underline_mode != actual.first.underline_mode)
    {
        std::cerr << "underline modes differ!" << std::endl;
        return false;
    }

    if (mask.underline_type && expected.first.underline_type != actual.first.underline_type)
    {
        std::cerr << "underline types differ!" << std::endl;
        return false;
    }

    if (mask.underline_color && expected.first.underline_color != actual.first.underline_color)
    {
        std::cerr << "underline colors differ!" << std::endl;
        return false;
    }

    if (mask.color && expected.first.color != actual.first.color)
    {
        std::cerr << "font colors differ!" << std::endl;
        return false;
    }

    if (mask.strikethrough_style && expected.first.strikethrough_style != actual.first.strikethrough_style)
    {
        std::cerr << "strikethrough styles differ!" << std::endl;
        return false;
    }

    if (mask.strikethrough_width && expected.first.strikethrough_width != actual.first.strikethrough_width)
    {
        std::cerr << "strikethrough widths differ!" << std::endl;
        return false;
    }

    if (mask.strikethrough_type && expected.first.strikethrough_type != actual.first.strikethrough_type)
    {
        std::cerr << "strikethrough types differ!" << std::endl;
        return false;
    }

    if (mask.strikethrough_text && expected.first.strikethrough_text != actual.first.strikethrough_text)
    {
        std::cerr << "strikethrough texts differ!" << std::endl;
        return false;
    }

    return true;
}

bool verify_active_fill_attrs(
    const std::pair<ss::fill_t, ss::fill_active_t>& expected,
    const std::pair<ss::fill_t, ss::fill_active_t>& actual)
{
    if (expected.second != actual.second)
    {
        std::cerr << "active masks differ!" << std::endl;
        return false;
    }

    const ss::fill_active_t& mask = expected.second;

    if (mask.pattern_type && expected.first.pattern_type != actual.first.pattern_type)
    {
        std::cerr << "pattern types differ!" << std::endl;
        return false;
    }

    if (mask.fg_color && expected.first.fg_color != actual.first.fg_color)
    {
        std::cerr << "foreground colors differ!" << std::endl;
        return false;
    }

    if (mask.bg_color && expected.first.bg_color != actual.first.bg_color)
    {
        std::cerr << "background colors differ!" << std::endl;
        return false;
    }

    return true;
}

bool verify_active_protection_attrs(
    const std::pair<ss::protection_t, ss::protection_active_t>& expected,
    const std::pair<ss::protection_t, ss::protection_active_t>& actual)
{
    if (expected.second != actual.second)
    {
        std::cerr << "active masks differ!" << std::endl;
        return false;
    }

    const ss::protection_active_t& mask = expected.second;

    if (mask.locked && expected.first.locked != actual.first.locked)
    {
        std::cerr << "locked states differ!" << std::endl;
        return false;
    }

    if (mask.hidden && expected.first.hidden != actual.first.hidden)
    {
        std::cerr << "hidden states differ!" << std::endl;
        return false;
    }

    if (mask.print_content && expected.first.print_content != actual.first.print_content)
    {
        std::cerr << "'print content' states differ!" << std::endl;
        return false;
    }

    if (mask.formula_hidden && expected.first.formula_hidden != actual.first.formula_hidden)
    {
        std::cerr << "'formula hidden' states differ!" << std::endl;
        return false;
    }

    return true;
}

bool verify_active_border_attrs(
    const std::pair<ss::border_t, ss::border_active_t>& expected,
    const std::pair<ss::border_t, ss::border_active_t>& actual)
{
    if (expected.second != actual.second)
    {
        std::cerr << "active masks differ!" << std::endl;
        return false;
    }

    const ss::border_active_t& mask = expected.second;

    auto verify_single = [](std::string_view name, const ss::border_attrs_active_t& _mask,
        const ss::border_attrs_t& _expected, const ss::border_attrs_t& _actual)
    {
        if (_mask.style && _expected.style != _actual.style)
        {
            std::cerr << name << " border styles differ!" << std::endl;
            return false;
        }

        if (_mask.border_color && _expected.border_color != _actual.border_color)
        {
            std::cerr << name << " border colors differ!" << std::endl;
            return false;
        }

        if (_mask.border_width && _expected.border_width != _actual.border_width)
        {
            std::cerr << name << " border widths differ!" << std::endl;
            return false;
        }

        return true;
    };

    if (!verify_single("top", mask.top, expected.first.top, actual.first.top))
        return false;

    if (!verify_single("bottom", mask.bottom, expected.first.bottom, actual.first.bottom))
        return false;

    if (!verify_single("left", mask.left, expected.first.left, actual.first.left))
        return false;

    if (!verify_single("right", mask.right, expected.first.right, actual.first.right))
        return false;

    if (!verify_single("diagonal", mask.diagonal, expected.first.diagonal, actual.first.diagonal))
        return false;

    if (!verify_single("diagonal_bl_tr", mask.diagonal_bl_tr, expected.first.diagonal_bl_tr, actual.first.diagonal_bl_tr))
        return false;

    if (!verify_single("diagonal_tl_br", mask.diagonal_tl_br, expected.first.diagonal_tl_br, actual.first.diagonal_tl_br))
        return false;

    return true;
}

const orcus::spreadsheet::cell_style_t* find_cell_style_by_name(
    std::string_view name, const orcus::spreadsheet::styles& styles)
{
    size_t n = styles.get_cell_styles_count();
    for (size_t i = 0; i < n; ++i)
    {
        const orcus::spreadsheet::cell_style_t* cur_style = styles.get_cell_style(i);
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

void test_odf_fill(const orcus::spreadsheet::styles& styles)
{
    const orcus::spreadsheet::cell_style_t* style = find_cell_style_by_name("Name1", styles);
    assert(style);
    assert(style->parent_name == "Text");
    size_t xf = style->xf;
    const orcus::spreadsheet::cell_format_t* cell_format = styles.get_cell_style_format(xf);
    assert(cell_format);

    size_t fill = cell_format->fill;
    const orcus::spreadsheet::fill_t* cell_fill = styles.get_fill(fill);
    assert(cell_fill);
    assert(cell_fill->fg_color.red == 0xfe);
    assert(cell_fill->fg_color.green == 0xff);
    assert(cell_fill->fg_color.blue == 0xcc);
    assert(cell_fill->pattern_type == orcus::spreadsheet::fill_pattern_t::solid);
}

void test_odf_border(const orcus::spreadsheet::styles &styles)
{
    /* Test that border style applies to all the sides when not specified */
    const orcus::spreadsheet::cell_style_t* style = find_cell_style_by_name("Name1", styles);
    assert(style);
    size_t xf = style->xf;
    const orcus::spreadsheet::cell_format_t* cell_format = styles.get_cell_style_format(xf);
    size_t border = cell_format->border;
    assert(cell_format);

    const orcus::spreadsheet::border_t* cell_border = styles.get_border(border);
    assert(cell_border->top.style == orcus::spreadsheet::border_style_t::dotted);
    assert(cell_border->bottom.style == orcus::spreadsheet::border_style_t::dotted);
    assert(cell_border->left.style == orcus::spreadsheet::border_style_t::dotted);
    assert(cell_border->right.style == orcus::spreadsheet::border_style_t::dotted);
    assert(cell_border->top.border_color.red == 0xff);
    assert(cell_border->bottom.border_color.green == 0xcc);
    assert(cell_border->left.border_color.blue == 0x12);
    assert(cell_border->right.border_width.value == 0.06);
    assert(cell_border->top.border_width.value == 0.06);

    /*Test that border applies to only specified sides*/
    style = find_cell_style_by_name("Name2", styles);
    assert(style);
    xf = style->xf;
    cell_format = styles.get_cell_style_format(xf);
    border = cell_format->border;
    assert(cell_format);

    cell_border = styles.get_border(border);
    assert(cell_border->top.style == orcus::spreadsheet::border_style_t::fine_dashed);
    assert(cell_border->bottom.style == orcus::spreadsheet::border_style_t::double_thin);
    assert(cell_border->left.style == orcus::spreadsheet::border_style_t::none);
    assert(cell_border->right.style == orcus::spreadsheet::border_style_t::dash_dot_dot);
    assert(cell_border->top.border_color.red == 0xff);
    assert(cell_border->bottom.border_color.green == 0xee);
    assert(cell_border->left.border_color.blue == 0x11);
    assert(cell_border->right.border_width.value == 0.22);
    assert(cell_border->bottom.border_width.value == 1.74);

    /*Test that border applies to the diagonal*/
    style = find_cell_style_by_name("Name3", styles);
    assert(style);
    xf = style->xf;
    cell_format = styles.get_cell_style_format(xf);
    border = cell_format->border;
    assert(cell_format);

    cell_border = styles.get_border(border);
    assert(cell_border->diagonal_bl_tr.style == orcus::spreadsheet::border_style_t::dashed);
    assert(cell_border->diagonal_tl_br.style == orcus::spreadsheet::border_style_t::dash_dot);
    assert(cell_border->diagonal_bl_tr.border_color.red == 0xff);
    assert(cell_border->diagonal_tl_br.border_color.green == 0x00);
    assert(cell_border->diagonal_tl_br.border_width.value == 0.74);
}

void test_odf_cell_protection(const orcus::spreadsheet::styles& styles)
{
    /* Test that Cell is only protected and not hidden , Print Content is true */
    const orcus::spreadsheet::cell_style_t* style = find_cell_style_by_name("Name5", styles);
    assert(style);
    size_t  xf = style->xf;
    const orcus::spreadsheet::cell_format_t* cell_format = styles.get_cell_style_format(xf);
    size_t protection = cell_format->protection;
    assert(cell_format);

    const orcus::spreadsheet::protection_t* cell_protection = styles.get_protection(protection);
    assert(cell_protection->locked == true);
    assert(cell_protection->hidden == true);
    assert(cell_protection->print_content == true);
    assert(cell_protection->formula_hidden == false);

    /* Test that Cell is  protected and formula is hidden , Print Content is false */
    style = find_cell_style_by_name("Name6", styles);
    assert(style);
    xf = style->xf;
    cell_format = styles.get_cell_style_format(xf);
    protection = cell_format->protection;
    assert(cell_format);

    cell_protection = styles.get_protection(protection);
    assert(cell_protection->locked == true);
    assert(cell_protection->hidden == false);
    assert(cell_protection->print_content == false);
    assert(cell_protection->formula_hidden == true);

    /* Test that Cell is not protected by any way, Print Content is false */
    style = find_cell_style_by_name("Name7", styles);
    assert(style);
    xf = style->xf;
    cell_format = styles.get_cell_style_format(xf);
    protection = cell_format->protection;
    assert(cell_format);

    cell_protection = styles.get_protection(protection);
    assert(cell_protection->locked == false);
    assert(cell_protection->hidden == false);
    assert(cell_protection->print_content == true);
    assert(cell_protection->formula_hidden == false);
}

void test_odf_font(const orcus::spreadsheet::styles& styles)
{
    const orcus::spreadsheet::cell_style_t* style = find_cell_style_by_name("Name8", styles);
    assert(style);
    size_t xf = style->xf;
    const orcus::spreadsheet::cell_format_t* cell_format = styles.get_cell_style_format(xf);
    size_t font = cell_format->font;
    assert(cell_format);

    const orcus::spreadsheet::font_t* cell_font = styles.get_font(font);
    assert(cell_font->name == "Liberation Sans");
    assert(cell_font->size == 24);
    assert(cell_font->bold == true);
    assert(cell_font->italic == true);
    assert(cell_font->underline_style == orcus::spreadsheet::underline_t::single_line);
    assert(cell_font->underline_width == orcus::spreadsheet::underline_width_t::thick);
    assert(cell_font->underline_mode == orcus::spreadsheet::underline_mode_t::continuous);
    assert(cell_font->underline_type == orcus::spreadsheet::underline_type_t::none);
    assert(cell_font->color.red == (int)0x80);
    assert(cell_font->color.green == (int)0x80);
    assert(cell_font->color.blue == (int)0x80);

    style = find_cell_style_by_name("Name9", styles);
    assert(style);
    xf = style->xf;
    cell_format = styles.get_cell_style_format(xf);
    font = cell_format->font;
    assert(cell_format);

    cell_font = styles.get_font(font);
    assert(cell_font->name == "Tahoma");
    assert(cell_font->size == 00);
    assert(cell_font->bold == true);
    assert(cell_font->italic == false);
    assert(cell_font->underline_style == orcus::spreadsheet::underline_t::dash);
    assert(cell_font->underline_width == orcus::spreadsheet::underline_width_t::bold);
    assert(cell_font->underline_mode == orcus::spreadsheet::underline_mode_t::continuous);
    assert(cell_font->underline_type == orcus::spreadsheet::underline_type_t::none);
    assert(cell_font->underline_color.red == (int)0x18);
    assert(cell_font->underline_color.green == (int)0x56);
    assert(cell_font->underline_color.blue == (int)0xff);
}

void test_number_format_styles()
{
#if 0 // TODO: temporarily disable this test
    stack_printer __sp__(__func__);

    test_model model;
    model.load(SRCDIR"/test/ods/styles/number-format.xml");

    auto verify_number_format_code = [](const ss::styles& styles, std::string_view style_name, std::string_view expected_code) -> bool {
            const ss::cell_format_t* xf = find_cell_format(styles, style_name, "");
            if (!xf)
            {
                std::cerr << "No cell format for style named '" << style_name << "' found." << std::endl;
                return false;
            }

            const auto* xnf = styles.get_number_format_state(xf->number_format);
            if (!xnf)
            {
                std::cerr << "No number format style found for style named '" << style_name << "'." << std::endl;
                return false;
            }

            // Make sure the format_string is the only active attribute.  In ODF,
            // format identifier is not used, so it should never be active.
            ss::number_format_active_t active_expected;
            active_expected.format_string = true;

            if (active_expected != xnf->second)
            {
                std::cerr << "Active number format attributes were not as expected." << std::endl;
                return false;
            }

            if (xnf->first.format_string != expected_code)
            {
                std::cerr << "Expected format code was '" << expected_code << "' but the actual code was '" << xnf->first.format_string << "'." << std::endl;
                return false;
            }

            return true;
    };

    assert(verify_number_format_code(model.styles, "Name10", "#.000000"));
    assert(verify_number_format_code(model.styles, "Name11", "[$₹]#,##0.00;[RED]-[$₹]#,##0.00"));
    assert(verify_number_format_code(model.styles, "Name12", "0.00%"));
    assert(verify_number_format_code(model.styles, "Name13", "#.00E+00"));
    assert(verify_number_format_code(model.styles, "Name15", "BOOLEAN"));
    assert(verify_number_format_code(model.styles, "Name16", "#### ?/11"));
    assert(verify_number_format_code(model.styles, "Name17", "MM/DD/YY"));
    assert(verify_number_format_code(model.styles, "Name18", "HH:MM:SS AM/PM"));
    assert(verify_number_format_code(model.styles, "Name19", "[>=0]0.00;[RED]-0.00"));
#endif
}

void test_odf_text_strikethrough(const orcus::spreadsheet::styles& styles)
{
    const orcus::spreadsheet::cell_style_t* style = find_cell_style_by_name("Name20", styles);
    assert(style);
    size_t xf = style->xf;
    const orcus::spreadsheet::cell_format_t* cell_format = styles.get_cell_style_format(xf);
    size_t font = cell_format->font;
    assert(cell_format);

    const orcus::spreadsheet::font_t* cell_font = styles.get_font(font);
    assert(cell_font->strikethrough_style == orcus::spreadsheet::strikethrough_style_t::solid);
    assert(cell_font->strikethrough_width == orcus::spreadsheet::strikethrough_width_t::unknown);
    assert(cell_font->strikethrough_type == orcus::spreadsheet::strikethrough_type_t::single_type);
    assert(cell_font->strikethrough_text == orcus::spreadsheet::strikethrough_text_t::unknown);

    style = find_cell_style_by_name("Name21", styles);
    assert(style);
    xf = style->xf;
    cell_format = styles.get_cell_style_format(xf);
    font = cell_format->font;
    assert(cell_format);

    cell_font = styles.get_font(font);
    assert(cell_font->strikethrough_style == orcus::spreadsheet::strikethrough_style_t::solid);
    assert(cell_font->strikethrough_width == orcus::spreadsheet::strikethrough_width_t::bold);
    assert(cell_font->strikethrough_type == orcus::spreadsheet::strikethrough_type_t::single_type);
    assert(cell_font->strikethrough_text == orcus::spreadsheet::strikethrough_text_t::unknown);

    style = find_cell_style_by_name("Name22", styles);
    assert(style);
    xf = style->xf;
    cell_format = styles.get_cell_style_format(xf);
    font = cell_format->font;
    assert(cell_format);

    cell_font = styles.get_font(font);
    assert(cell_font->strikethrough_style == orcus::spreadsheet::strikethrough_style_t::solid);
    assert(cell_font->strikethrough_width == orcus::spreadsheet::strikethrough_width_t::unknown);
    assert(cell_font->strikethrough_type == orcus::spreadsheet::strikethrough_type_t::single_type);
    assert(cell_font->strikethrough_text == orcus::spreadsheet::strikethrough_text_t::slash);
}

void test_odf_text_alignment(const orcus::spreadsheet::styles& styles)
{
    const orcus::spreadsheet::cell_style_t* style = find_cell_style_by_name("Name23", styles);
    assert(style);
    size_t xf = style->xf;
    const orcus::spreadsheet::cell_format_t* cell_format = styles.get_cell_style_format(xf);
    assert(cell_format);

    assert(cell_format->hor_align == orcus::spreadsheet::hor_alignment_t::right);
    assert(cell_format->ver_align == orcus::spreadsheet::ver_alignment_t::middle);
}

void test_cell_styles()
{
    stack_printer __sp__(__func__);

    test_model model;

    model.load(SRCDIR"/test/ods/styles/cell-styles.xml");
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
    model.load(SRCDIR"/test/ods/styles/standard-styles.xml");

    {
        // Heading only specifies color, font size, font style and font weight.
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Heading", "Default");
        assert(cell_format);

        std::pair<ss::font_t, ss::font_active_t> expected;
        expected.first.size = 24;
        expected.first.bold = true;
        expected.first.italic = false;
        expected.first.color = ss::color_t(0, 0, 0);
        expected.second.size = true;
        expected.second.bold = true;
        expected.second.italic = true;
        expected.second.color = true;

        const auto* font_state = model.styles.get_font_state(cell_format->font);
        assert(font_state);
        assert(verify_active_font_attrs(expected, *font_state));
    }

    {
        // Heading 1 only overwrites font size.
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Heading 1", "Heading");
        assert(cell_format);

        std::pair<ss::font_t, ss::font_active_t> expected;
        expected.first.size = 18;
        expected.second.size = true;

        const auto* font_state = model.styles.get_font_state(cell_format->font);
        assert(font_state);
        assert(verify_active_font_attrs(expected, *font_state));
    }

    {
        // Heading 2 also only overwrites font size but with a different size.
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Heading 2", "Heading");
        assert(cell_format);

        std::pair<ss::font_t, ss::font_active_t> expected;
        expected.first.size = 12;
        expected.second.size = true;

        const auto* font_state = model.styles.get_font_state(cell_format->font);
        assert(font_state);
        assert(verify_active_font_attrs(expected, *font_state));
    }

    {
        // Text simply inherits from Default.
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Text", "Default");
        assert(cell_format);

        std::pair<ss::font_t, ss::font_active_t> expected; // nothing is active

        const auto* font_state = model.styles.get_font_state(cell_format->font);
        assert(font_state);
        assert(verify_active_font_attrs(expected, *font_state));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Note", "Text");
        assert(cell_format);

        std::pair<ss::font_t, ss::font_active_t> expected;
        expected.first.color = ss::color_t(0x33, 0x33, 0x33);
        expected.second.color = true;

        const auto* font_state = model.styles.get_font_state(cell_format->font);
        assert(font_state);
        assert(verify_active_font_attrs(expected, *font_state));

        std::pair<ss::fill_t, ss::fill_active_t> expected_fill;
        expected_fill.first.pattern_type = ss::fill_pattern_t::solid;
        expected_fill.first.fg_color = ss::color_t(0xff, 0xff, 0xcc);
        expected_fill.second.pattern_type = true;
        expected_fill.second.fg_color = true;

        const auto* fill_state = model.styles.get_fill_state(cell_format->fill);
        assert(fill_state);
        assert(verify_active_fill_attrs(expected_fill, *fill_state));

        // fo:border="0.75pt solid #808080" -> same border attributes applied to top, bottom, left and right borders.
        std::pair<ss::border_t, ss::border_active_t> expected_border;
        expected_border.first.top.style = ss::border_style_t::solid;
        expected_border.first.top.border_width.value = 0.75;
        expected_border.first.top.border_width.unit = orcus::length_unit_t::point;
        expected_border.first.top.border_color = ss::color_t(0x80, 0x80, 0x80);

        expected_border.first.bottom = expected_border.first.top;
        expected_border.first.left = expected_border.first.top;
        expected_border.first.right = expected_border.first.top;

        expected_border.second.top.style = true;
        expected_border.second.top.border_width = true;
        expected_border.second.top.border_color = true;

        expected_border.second.bottom = expected_border.second.top;
        expected_border.second.left = expected_border.second.top;
        expected_border.second.right = expected_border.second.top;

        const auto* border_state = model.styles.get_border_state(cell_format->border);
        assert(border_state);
        assert(verify_active_border_attrs(expected_border, *border_state));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Footnote", "Text");
        assert(cell_format);

        std::pair<ss::font_t, ss::font_active_t> expected;
        expected.first.color = ss::color_t(0x80, 0x80, 0x80);
        expected.first.italic = true;
        expected.second.color = true;
        expected.second.italic = true;

        const auto* font_state = model.styles.get_font_state(cell_format->font);
        assert(font_state);
        assert(verify_active_font_attrs(expected, *font_state));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Hyperlink", "Text");
        assert(cell_format);

        // TODO: Since we currently handle text-underline-width and
        // text-underline-color incorrectly, we cannot perform exhaustive check.
        // We can only check the attributes individually.

        const auto* font_state = model.styles.get_font_state(cell_format->font);
        assert(font_state);

        const ss::font_t& values = font_state->first;
        const ss::font_active_t& active = font_state->second;

        // fo:color="#0000ee"
        assert(values.color == ss::color_t(0x00, 0x00, 0xee));
        assert(active.color);

        // style:text-underline-style="solid"
        assert(values.underline_style == ss::underline_t::single_line); // solid
        assert(active.underline_style);

        // style:text-underline-width="auto"
        // TODO: we cannot handle this until 0.18.

        // style:text-underline-color="font-color" (use the same color as the font)
        assert(values.underline_color == ss::color_t(0x00, 0x00, 0xee));
        assert(active.underline_color);
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Status", "Default");
        assert(cell_format);
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Good", "Status");
        assert(cell_format);

        std::pair<ss::font_t, ss::font_active_t> expected;
        expected.first.color = ss::color_t(0x00, 0x66, 0x00);
        expected.second.color = true;

        const auto* font_state = model.styles.get_font_state(cell_format->font);
        assert(font_state);
        assert(verify_active_font_attrs(expected, *font_state));

        std::pair<ss::fill_t, ss::fill_active_t> expected_fill;
        expected_fill.first.pattern_type = ss::fill_pattern_t::solid;
        expected_fill.first.fg_color = ss::color_t(0xcc, 0xff, 0xcc);
        expected_fill.second.pattern_type = true;
        expected_fill.second.fg_color = true;

        const auto* fill_state = model.styles.get_fill_state(cell_format->fill);
        assert(fill_state);
        assert(verify_active_fill_attrs(expected_fill, *fill_state));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Neutral", "Status");
        assert(cell_format);

        std::pair<ss::font_t, ss::font_active_t> expected;
        expected.first.color = ss::color_t(0x99, 0x66, 0x00);
        expected.second.color = true;

        const auto* font_state = model.styles.get_font_state(cell_format->font);
        assert(font_state);
        assert(verify_active_font_attrs(expected, *font_state));

        std::pair<ss::fill_t, ss::fill_active_t> expected_fill;
        expected_fill.first.pattern_type = ss::fill_pattern_t::solid;
        expected_fill.first.fg_color = ss::color_t(0xff, 0xff, 0xcc);
        expected_fill.second.pattern_type = true;
        expected_fill.second.fg_color = true;

        const auto* fill_state = model.styles.get_fill_state(cell_format->fill);
        assert(fill_state);
        assert(verify_active_fill_attrs(expected_fill, *fill_state));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Bad", "Status");
        assert(cell_format);

        std::pair<ss::font_t, ss::font_active_t> expected;
        expected.first.color = ss::color_t(0xcc, 0x00, 0x00);
        expected.second.color = true;

        const auto* font_state = model.styles.get_font_state(cell_format->font);
        assert(font_state);
        assert(verify_active_font_attrs(expected, *font_state));

        std::pair<ss::fill_t, ss::fill_active_t> expected_fill;
        expected_fill.first.pattern_type = ss::fill_pattern_t::solid;
        expected_fill.first.fg_color = ss::color_t(0xff, 0xcc, 0xcc);
        expected_fill.second.pattern_type = true;
        expected_fill.second.fg_color = true;

        const auto* fill_state = model.styles.get_fill_state(cell_format->fill);
        assert(fill_state);
        assert(verify_active_fill_attrs(expected_fill, *fill_state));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Warning", "Status");
        assert(cell_format);

        std::pair<ss::font_t, ss::font_active_t> expected;
        expected.first.color = ss::color_t(0xcc, 0x00, 0x00);
        expected.second.color = true;

        const auto* font_state = model.styles.get_font_state(cell_format->font);
        assert(font_state);
        assert(verify_active_font_attrs(expected, *font_state));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Error", "Status");
        assert(cell_format);

        std::pair<ss::font_t, ss::font_active_t> expected;
        expected.first.color = ss::color_t(0xff, 0xff, 0xff);
        expected.first.bold = true;
        expected.second.color = true;
        expected.second.bold = true;

        const auto* font_state = model.styles.get_font_state(cell_format->font);
        assert(font_state);
        assert(verify_active_font_attrs(expected, *font_state));

        std::pair<ss::fill_t, ss::fill_active_t> expected_fill;
        expected_fill.first.pattern_type = ss::fill_pattern_t::solid;
        expected_fill.first.fg_color = ss::color_t(0xcc, 0x00, 0x00);
        expected_fill.second.pattern_type = true;
        expected_fill.second.fg_color = true;

        const auto* fill_state = model.styles.get_fill_state(cell_format->fill);
        assert(fill_state);
        assert(verify_active_fill_attrs(expected_fill, *fill_state));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Accent", "Default");
        assert(cell_format);

        std::pair<ss::font_t, ss::font_active_t> expected;
        expected.first.bold = true;
        expected.second.bold = true;

        const auto* font_state = model.styles.get_font_state(cell_format->font);
        assert(font_state);
        assert(verify_active_font_attrs(expected, *font_state));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Accent 1", "Accent");
        assert(cell_format);

        std::pair<ss::font_t, ss::font_active_t> expected;
        expected.first.color = ss::color_t(0xff, 0xff, 0xff);
        expected.second.color = true;

        const auto* font_state = model.styles.get_font_state(cell_format->font);
        assert(font_state);
        assert(verify_active_font_attrs(expected, *font_state));

        std::pair<ss::fill_t, ss::fill_active_t> expected_fill;
        expected_fill.first.pattern_type = ss::fill_pattern_t::solid;
        expected_fill.first.fg_color = ss::color_t(0x00, 0x00, 0x00);
        expected_fill.second.pattern_type = true;
        expected_fill.second.fg_color = true;

        const auto* fill_state = model.styles.get_fill_state(cell_format->fill);
        assert(fill_state);
        assert(verify_active_fill_attrs(expected_fill, *fill_state));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Accent 2", "Accent");
        assert(cell_format);

        std::pair<ss::font_t, ss::font_active_t> expected;
        expected.first.color = ss::color_t(0xff, 0xff, 0xff);
        expected.second.color = true;

        std::pair<ss::fill_t, ss::fill_active_t> expected_fill;
        expected_fill.first.pattern_type = ss::fill_pattern_t::solid;
        expected_fill.first.fg_color = ss::color_t(0x80, 0x80, 0x80);
        expected_fill.second.pattern_type = true;
        expected_fill.second.fg_color = true;

        const auto* fill_state = model.styles.get_fill_state(cell_format->fill);
        assert(fill_state);
        assert(verify_active_fill_attrs(expected_fill, *fill_state));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Accent 3", "Accent");
        assert(cell_format);

        std::pair<ss::fill_t, ss::fill_active_t> expected_fill;
        expected_fill.first.pattern_type = ss::fill_pattern_t::solid;
        expected_fill.first.fg_color = ss::color_t(0xdd, 0xdd, 0xdd);
        expected_fill.second.pattern_type = true;
        expected_fill.second.fg_color = true;

        const auto* fill_state = model.styles.get_fill_state(cell_format->fill);
        assert(fill_state);
        assert(verify_active_fill_attrs(expected_fill, *fill_state));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Result", "Default");
        assert(cell_format);

        std::pair<ss::font_t, ss::font_active_t> expected;
        expected.first.bold = true;
        expected.first.italic = true;
        expected.first.underline_style = ss::underline_t::single_line;
        expected.second.bold = true;
        expected.second.italic = true;
        expected.second.underline_style = true;
    }
}

void test_cell_protection_styles()
{
    stack_printer __sp__(__func__);

    test_model model;
    model.load(SRCDIR"/test/ods/styles/cell-protection.xml");

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Hide_20_Formula", "Protected");
        assert(cell_format);

        const auto* protection_state = model.styles.get_protection_state(cell_format->protection);
        assert(protection_state);

        std::pair<ss::protection_t, ss::protection_active_t> expected;
        expected.first.locked = true;
        expected.first.formula_hidden = true;
        expected.first.print_content = true;
        expected.second.locked = true;
        expected.second.formula_hidden = true;
        expected.second.print_content = true;

        assert(verify_active_protection_attrs(expected, *protection_state));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Hide_20_When_20_Printing", "Protected");
        assert(cell_format);

        const auto* protection_state = model.styles.get_protection_state(cell_format->protection);
        assert(protection_state);

        std::pair<ss::protection_t, ss::protection_active_t> expected;
        expected.first.locked = true;
        expected.first.print_content = false;
        expected.second.locked = true;
        expected.second.print_content = true;

        assert(verify_active_protection_attrs(expected, *protection_state));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Hide_20_All", "Protected");
        assert(cell_format);

        const auto* protection_state = model.styles.get_protection_state(cell_format->protection);
        assert(protection_state);

        std::pair<ss::protection_t, ss::protection_active_t> expected;
        expected.first.locked = true;
        expected.first.hidden = true;
        expected.first.print_content = true;
        expected.second.locked = true;
        expected.second.hidden = true;
        expected.second.print_content = true;

        assert(verify_active_protection_attrs(expected, *protection_state));
    }

    {
        const ss::cell_format_t* cell_format = find_cell_format(model.styles, "Not_20_Protected", "Default");
        assert(cell_format);

        const auto* protection_state = model.styles.get_protection_state(cell_format->protection);
        assert(protection_state);

        std::pair<ss::protection_t, ss::protection_active_t> expected;
        expected.first.locked = false;
        expected.first.hidden = false;
        expected.first.print_content = true;
        expected.first.formula_hidden = false;
        expected.second.locked = true;
        expected.second.hidden = true;
        expected.second.print_content = true;
        expected.second.formula_hidden = true;

        assert(verify_active_protection_attrs(expected, *protection_state));
    }
}

} // anonymous namespace

int main()
{
    test_cell_styles();
    test_number_format_styles();
    test_standard_styles();
    test_cell_protection_styles();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
