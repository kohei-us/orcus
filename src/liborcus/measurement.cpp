/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/measurement.hpp>
#include <orcus/exception.hpp>
#include <orcus/parser_global.hpp>

#include <mdds/sorted_string_map.hpp>
#include <mdds/global.hpp>

#include <sstream>

namespace orcus {

double to_double(std::string_view s, const char** p_parse_ended)
{
    const char* p = s.data();
    double value;
    const char* p_last = parse_numeric(p, p + s.size(), value);
    if (p_parse_ended)
        *p_parse_ended = p_last;

    return value;
}

long to_long(std::string_view s, const char** p_parse_ended)
{
    long value;
    const char* p_last = parse_integer(s.data(), s.data() + s.size(), value);
    if (p_parse_ended)
        *p_parse_ended = p_last;

    return value;
}

std::optional<long> to_long_checked(std::string_view s)
{
    const char* p_end = nullptr;
    long v = to_long(s, &p_end);
    if (p_end == s.data())
        return {};

    return v;
}

bool to_bool(std::string_view s)
{
    size_t n = s.size();
    if (n == 1)
        // Any single char other than '0' is true.
        return s[0] != '0';

    return s == "true" || s == "TRUE";
}

namespace {

namespace length {

using map_type = mdds::sorted_string_map<length_unit_t>;

// Keys must be sorted.
constexpr map_type::entry_type entries[] =
{
    { "cm", length_unit_t::centimeter },
    { "in", length_unit_t::inch },
    { "mm", length_unit_t::millimeter },
    { "pt", length_unit_t::point },
    { "px", length_unit_t::pixel }
};

const map_type& get()
{
    static map_type mt(entries, std::size(entries), length_unit_t::unknown);
    return mt;
}

} // namespace length

} // anonymous namespace

length_t to_length(std::string_view str)
{
    length_t ret;
    if (str.empty())
        return ret;

    const char* p = str.data();
    const char* p_start = p;
    const char* p_end = p_start + str.size();
    p = parse_numeric(p, p_end, ret.value);

    std::string_view tail(p, p_end-p);
    ret.unit = length::get().find(tail);

    return ret;
}

namespace {

double convert_inch(double value, length_unit_t unit_to)
{
    switch (unit_to)
    {
        case length_unit_t::twip:
            // inches to twips : 1 twip = 1/1440 inches
            return value * 1440.0;
        default:
            ;
    }

    throw general_error("convert_inch: unsupported unit of measurement.");
}

double convert_point(double value, length_unit_t unit_to)
{
    switch (unit_to)
    {
        case length_unit_t::twip:
            // 20 twips = 1 point
            return value * 20.0;
        default:
            ;
    }

    throw general_error("convert_point: unsupported unit of measurement.");
}

double convert_centimeter(double value, length_unit_t unit_to)
{
    switch (unit_to)
    {
        case length_unit_t::twip:
            // centimeters to twips : 2.54 cm = 1 inch = 1440 twips
            return value / 2.54 * 1440.0;
        default:
            ;
    }

    throw general_error("convert_centimeter: unsupported unit of measurement.");
}

double convert_millimeter(double value, length_unit_t unit_to)
{
    switch (unit_to)
    {
        case length_unit_t::twip:
            // millimeters to twips : 25.4 mm = 1 inch = 1440 twips
            return value / 25.4 * 1440.0;
        default:
            ;
    }

    throw general_error("convert_millimeter: unsupported unit of measurement.");
}

double convert_twip(double value, length_unit_t unit_to)
{
    switch (unit_to)
    {
        case length_unit_t::inch:
            // twips to inches : 1 twip = 1/1440 inches
            return value / 1440.0;
        case length_unit_t::point:
            // 1 twip = 1/1440 inches = 72/1440 points = 1/20 points
            return value / 20.0;
        default:
            ;
    }
    throw general_error("convert_twip: unsupported unit of measurement.");
}

/**
 * Since Excel's column width is based on the maximum digit width of font
 * used as the "Normal" style font, it's impossible to convert it accurately
 * without the font information.
 */
double convert_xlsx_column_digit(double value, length_unit_t unit_to)
{
    // Convert to centimeters first. Here, we'll just assume that a single
    // digit always equals 1.9 millimeters. TODO: find a better way to convert
    // this.
    value *= 0.19;
    return convert_centimeter(value, unit_to);
}

}

double convert(double value, length_unit_t unit_from, length_unit_t unit_to)
{
    if (value == 0.0)
        return value;

    switch (unit_from)
    {
        case length_unit_t::point:
            return convert_point(value, unit_to);
        case length_unit_t::inch:
            return convert_inch(value, unit_to);
        case length_unit_t::centimeter:
            return convert_centimeter(value, unit_to);
        case length_unit_t::millimeter:
            return convert_millimeter(value, unit_to);
        case length_unit_t::twip:
            return convert_twip(value, unit_to);
        case length_unit_t::xlsx_column_digit:
            return convert_xlsx_column_digit(value, unit_to);
        default:
            ;
    }

    std::ostringstream os;
    os << "convert: unsupported unit of measurement (from "
        << static_cast<int>(unit_from) << " to "
        << static_cast<int>(unit_to) << ") (value=" << value << ")";
    throw general_error(os.str());
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
