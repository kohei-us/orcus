/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_test_global.hpp"

#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/sheet.hpp>
#include <orcus/spreadsheet/styles.hpp>
#include <orcus/spreadsheet/shared_strings.hpp>
#include <orcus/stream.hpp>
#include <orcus/parser_global.hpp>

#include <sstream>
#include <cmath>
#include <iostream>
#include <chrono>

namespace ss = orcus::spreadsheet;

namespace orcus { namespace test {

std::string get_content_check(const spreadsheet::document& doc)
{
    std::ostringstream os;
    doc.dump_check(os);
    return os.str();
}

std::string get_content_as_csv(const spreadsheet::document& doc, spreadsheet::sheet_t sheet_index)
{
    const spreadsheet::sheet* sh = doc.get_sheet(sheet_index);
    if (!sh)
        return std::string();

    std::ostringstream os;
    sh->dump_csv(os);
    return os.str();
}

void verify_content(
    const char* filename, size_t line_no, const spreadsheet::document& doc, std::string_view expected)
{
    std::string actual = get_content_check(doc);
    verify_content(filename, line_no, expected, actual);
}

void verify_value_to_decimals(
    const char* filename, size_t line_no, double expected, double actual, int decimals)
{
    double expected_f = expected;
    double actual_f = actual;

    for (int i = 0; i < decimals; ++i)
    {
        expected_f *= 10.0;
        actual_f *= 10.0;
    }

    long expected_i = std::lround(expected_f);
    long actual_i = std::lround(actual_f);

    if (expected_i == actual_i)
        return;

    std::ostringstream os;
    os << "value is not as expected: (expected: " << expected << "; actual: " << actual << ")";
    throw assert_error(filename, line_no, os.str().data());
}

std::string prefix_multiline_string(std::string_view str, std::string_view prefix)
{
    std::ostringstream os;

    const char* p = str.data();
    const char* p_end = p + str.size();

    const char* p0 = nullptr;
    for (; p != p_end; ++p)
    {
        if (!p0)
            p0 = p;

        if (*p == '\n')
        {
            os << prefix << std::string_view(p0, std::distance(p0, p)) << '\n';
            p0 = nullptr;
        }
    }

    if (p0)
        os << prefix << std::string_view(p0, std::distance(p0, p));

    return os.str();
}

bool set(const std::optional<bool>& v)
{
    return v && v.value();
}

bool strikethrough_set(const ss::strikethrough_t& st)
{
    if (!st.style)
        return false;

    if (*st.style != ss::strikethrough_style_t::solid)
        return false;

    if (!st.type)
        return false;

    if (*st.type != ss::strikethrough_type_t::single_type)
        return false;

    if (!st.width)
        return false;

    if (*st.width != ss::strikethrough_width_t::width_auto)
        return false;

    return true;
}

const ss::font_t* get_font(const ss::document& doc, ss::sheet_t sheet, ss::row_t row, ss::col_t col)
{
    const ss::sheet* sh = doc.get_sheet(sheet);
    if (!sh)
    {
        std::cerr << "Failed to get a sheet at the sheet index of " << sheet << std::endl;
        return nullptr;
    }

    std::size_t xf = sh->get_cell_format(row, col);

    const auto& styles = doc.get_styles();

    const ss::cell_format_t* cell_format = styles.get_cell_format(xf);
    if (!cell_format)
    {
        std::cerr << "Failed to get a cell format buffer at row " << row << " and column " << col << std::endl;
        return nullptr;
    }

    return styles.get_font(cell_format->font);
};

bool check_cell_text(const ss::document& doc, ss::sheet_t sheet, ss::row_t row, ss::col_t col, std::string_view expected)
{
    const auto& sstrings = doc.get_shared_strings();
    const ss::sheet* sh = doc.get_sheet(sheet);
    if (!sh)
    {
        std::cerr << "Failed to get a sheet at the sheet index of " << sheet << std::endl;
        return false;
    }

    std::size_t si = sh->get_string_identifier(row, col);
    const std::string* s = sstrings.get_string(si);
    if (!s)
    {
        std::cerr << "expected='" << expected << "'; actual=<none> "
            << "(sheet=" << sh->get_index() << "; row=" << row << "; column=" << col << ")"
            << std::endl;

        return false;
    }

    if (*s == expected)
        return true;

    std::cerr << "expected='" << expected << "'; actual='" << *s << "' "
        << "(sheet=" << sh->get_index() << "; row=" << row << "; column=" << col << ")"
        << std::endl;

    return false;
};

const ss::format_runs_t* get_format_runs(
    const ss::document& doc, ss::sheet_t sheet, ss::row_t row, ss::col_t col)
{
    const ss::sheet* sh = doc.get_sheet(sheet);
    if (!sh)
    {
        std::cerr << "Failed to get a sheet at the sheet index of " << sheet << std::endl;
        return nullptr;
    }

    auto si = sh->get_string_identifier(row, col);
    return doc.get_shared_strings().get_format_runs(si);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
