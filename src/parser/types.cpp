/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/types.hpp>
#include <orcus/parser_global.hpp>
#include <orcus/global.hpp>
#include <orcus/xml_namespace.hpp>

#include <limits>
#include <sstream>
#include <string_view>
#include <iomanip>
#include <mdds/sorted_string_map.hpp>

#include "ostream_utils.hpp"

namespace orcus {

const xmlns_id_t XMLNS_UNKNOWN_ID = nullptr;
const xml_token_t XML_UNKNOWN_TOKEN = 0;

size_t xml_token_pair_hash::operator()(const xml_token_pair_t& v) const
{
    return std::hash<const char*>()(v.first) ^ std::hash<size_t>()(v.second);
}

const size_t index_not_found = std::numeric_limits<size_t>::max();

parse_error_value_t::parse_error_value_t() :
    offset(0)
{
}

parse_error_value_t::parse_error_value_t(std::string_view _str, std::ptrdiff_t _offset) :
    str(_str), offset(_offset)
{
}

bool parse_error_value_t::operator==(const parse_error_value_t& other) const
{
    return str == other.str && offset == other.offset;
}

bool parse_error_value_t::operator!=(const parse_error_value_t& other) const
{
    return !operator==(other);
}

xml_name_t::xml_name_t() : ns(XMLNS_UNKNOWN_ID), name() {}
xml_name_t::xml_name_t(xmlns_id_t _ns, std::string_view _name) : ns(_ns), name(_name) {}
xml_name_t::xml_name_t(const xml_name_t& r) : ns(r.ns), name(r.name) {}

xml_name_t& xml_name_t::operator= (const xml_name_t& other)
{
    ns = other.ns;
    name = other.name;
    return *this;
}

bool xml_name_t::operator== (const xml_name_t& other) const
{
    return ns == other.ns && name == other.name;
}

bool xml_name_t::operator!= (const xml_name_t& other) const
{
    return !operator==(other);
}

std::string xml_name_t::to_string(const xmlns_context& cxt, to_string_type type) const
{
    std::ostringstream os;

    if (ns)
    {
        std::string_view ns_str;
        switch (type)
        {
            case use_alias:
                ns_str = cxt.get_alias(ns);
                break;
            case use_short_name:
                ns_str = cxt.get_short_name(ns);
                break;

        }
        if (!ns_str.empty())
            os << ns_str << ':';
    }
    os << name;

    return os.str();
}

std::string xml_name_t::to_string(const xmlns_repository& repo) const
{
    std::ostringstream os;

    if (ns)
    {
        std::string ns_str = repo.get_short_name(ns);
        if (!ns_str.empty())
            os << ns_str << ':';
    }
    os << name;

    return os.str();
}

xml_token_attr_t::xml_token_attr_t() :
    ns(XMLNS_UNKNOWN_ID), name(XML_UNKNOWN_TOKEN), transient(false) {}

xml_token_attr_t::xml_token_attr_t(
    xmlns_id_t _ns, xml_token_t _name, std::string_view _value, bool _transient) :
    ns(_ns), name(_name), value(_value), transient(_transient) {}

xml_token_attr_t::xml_token_attr_t(
    xmlns_id_t _ns, xml_token_t _name, std::string_view _raw_name, std::string_view _value, bool _transient) :
    ns(_ns), name(_name), raw_name(_raw_name), value(_value), transient(_transient) {}

xml_token_element_t::xml_token_element_t() : ns(nullptr), name(XML_UNKNOWN_TOKEN) {}

xml_token_element_t::xml_token_element_t(
    xmlns_id_t _ns, xml_token_t _name, std::string_view _raw_name, std::vector<xml_token_attr_t>&& _attrs)  :
    ns(_ns), name(_name), raw_name(_raw_name), attrs(std::move(_attrs)) {}

xml_token_element_t::xml_token_element_t(const xml_token_element_t& other) :
    ns(other.ns), name(other.name), raw_name(other.raw_name), attrs(other.attrs) {}

xml_token_element_t::xml_token_element_t(xml_token_element_t&& other) :
    ns(other.ns), name(other.name), raw_name(other.raw_name), attrs(std::move(other.attrs)) {}

xml_declaration_t::xml_declaration_t() :
    version_major(1),
    version_minor(0),
    encoding(character_set_t::unspecified),
    standalone(false) {}

xml_declaration_t::xml_declaration_t(uint8_t _version_major, uint8_t _version_minor, character_set_t _encoding, bool _standalone) :
    version_major(_version_major), version_minor(_version_minor), encoding(_encoding), standalone(_standalone) {}

xml_declaration_t::xml_declaration_t(const xml_declaration_t& other) :
    version_major(other.version_major),
    version_minor(other.version_minor),
    encoding(other.encoding),
    standalone(other.standalone) {}

xml_declaration_t::~xml_declaration_t() {}

xml_declaration_t& xml_declaration_t::operator= (const xml_declaration_t& other)
{
    version_major = other.version_major;
    version_minor = other.version_minor;
    encoding = other.encoding;
    standalone = other.standalone;
    return *this;
}

bool xml_declaration_t::operator== (const xml_declaration_t& other) const
{
    return version_major == other.version_major && version_minor == other.version_minor &&
        encoding == other.encoding && standalone == other.standalone;
}

bool xml_declaration_t::operator!= (const xml_declaration_t& other) const
{
    return !operator== (other);
}

length_t::length_t() : unit(length_unit_t::unknown), value(0.0) {}

std::string length_t::to_string() const
{
    std::ostringstream os;
    os << value;

    switch (unit)
    {
        case length_unit_t::centimeter:
            os << " cm";
        break;
        case length_unit_t::inch:
            os << " in";
        break;
        case length_unit_t::point:
            os << " pt";
        break;
        case length_unit_t::twip:
            os << " twip";
        break;
        case length_unit_t::unknown:
        default:
            ;
    }

    return os.str();
}

bool length_t::operator== (const length_t& other) const noexcept
{
    return value == other.value && unit == other.unit;
}

bool length_t::operator!= (const length_t& other) const noexcept
{
    return !operator== (other);
}

date_time_t::date_time_t() :
    year(0), month(0), day(0), hour(0), minute(0), second(0.0) {}

date_time_t::date_time_t(int _year, int _month, int _day) :
    year(_year), month(_month), day(_day), hour(0), minute(0), second(0.0) {}

date_time_t::date_time_t(int _year, int _month, int _day, int _hour, int _minute, double _second) :
    year(_year), month(_month), day(_day), hour(_hour), minute(_minute), second(_second) {}

date_time_t::date_time_t(const date_time_t& other) :
    year(other.year),
    month(other.month),
    day(other.day),
    hour(other.hour),
    minute(other.minute),
    second(other.second) {}

date_time_t::~date_time_t() {}

date_time_t& date_time_t::operator= (date_time_t other)
{
    swap(other);
    return *this;
}

void date_time_t::swap(date_time_t& other)
{
    std::swap(year, other.year);
    std::swap(month, other.month);
    std::swap(day, other.day);
    std::swap(hour, other.hour);
    std::swap(minute, other.minute);
    std::swap(second, other.second);
}

date_time_t date_time_t::from_chars(std::string_view str)
{
    auto flush_int = [](int& store, const char*& digit, size_t& digit_len)
    {
        long v;
        parse_integer(digit, digit + digit_len, v);
        store = v;

        digit = nullptr;
        digit_len = 0;
    };

    auto process_char = [](const char* p, const char*& digit, size_t& digit_len)
    {
        if (!digit)
        {
            digit = p;
            digit_len = 1;
            return;
        }

        ++digit_len;
    };

    date_time_t ret;
    int dash_count = 0, t_count = 0, colon_count = 0;

    const char* p = str.data();
    const char* p_end = p + str.size();
    const char* digit = p;
    size_t digit_len = 0;

    bool valid = true;
    for (; p != p_end && valid; ++p)
    {
        switch (*p)
        {
            case '-':
            {
                if (t_count || colon_count || !digit)
                {
                    // Invalid date-time value.  All dashes must occur before
                    // any of 'T' and ':' occur.
                    valid = false;
                    break;
                }

                switch (dash_count)
                {
                    case 0:
                        // Flush year.
                        flush_int(ret.year, digit, digit_len);
                    break;
                    case 1:
                        // Flush month.
                        flush_int(ret.month, digit, digit_len);
                    break;
                    default:
                        valid = false;
                }
                ++dash_count;
            }
            break;
            case 'T':
            {
                if (t_count || dash_count != 2 || !digit)
                {
                    // Invalid date-time value.
                    valid = false;
                    break;
                }

                // Flush day.
                flush_int(ret.day, digit, digit_len);
                ++t_count;
            }
            break;
            case ':':
            {
                if (!t_count || !digit)
                {
                    // Invalid date-time value.
                    valid = false;
                    break;
                }

                switch (colon_count)
                {
                    case 0:
                        // Flush hour.
                        flush_int(ret.hour, digit, digit_len);
                    break;
                    case 1:
                        // Flush minute.
                        flush_int(ret.minute, digit, digit_len);
                    break;
                    default:
                        valid = false;
                }

                ++colon_count;
            }
            break;
            default:
            {
                if (t_count)
                {
                    // Time element.
                    switch (colon_count)
                    {
                        case 0:
                            // Hour
                            process_char(p, digit, digit_len);
                        break;
                        case 1:
                            // Minute
                            process_char(p, digit, digit_len);
                        break;
                        case 2:
                            // Second
                            process_char(p, digit, digit_len);
                        break;
                        default:
                            valid = false;
                    }
                }
                else
                {
                    // Date element.
                    switch (dash_count)
                    {
                        case 0:
                            // Year
                            process_char(p, digit, digit_len);
                        break;
                        case 1:
                            // Month
                            process_char(p, digit, digit_len);
                        break;
                        case 2:
                            // Day
                            process_char(p, digit, digit_len);
                        break;
                        default:
                            valid = false;
                    }
                }
            }
        }

    }

    if (!valid || !digit)
        return ret;

    if (t_count)
    {
        // Flush second.
        ret.second = strtod(digit, nullptr);
    }
    else
    {
        // Flush day.
        flush_int(ret.day, digit, digit_len);
    }

    return ret;
}

bool date_time_t::operator== (const date_time_t& other) const
{
    return year == other.year && month == other.month && day == other.day &&
        hour == other.hour && minute == other.minute && second == other.second;
}

bool date_time_t::operator!= (const date_time_t& other) const
{
    return !operator== (other);
}

bool date_time_t::operator< (const date_time_t& other) const
{
    if (year != other.year)
        return year < other.year;

    if (month != other.month)
        return month < other.month;

    if (day != other.day)
        return day < other.day;

    if (hour != other.hour)
        return hour < other.hour;

    if (minute != other.minute)
        return minute < other.minute;

    return second < other.second;
}

std::string date_time_t::to_string() const
{
    std::ostringstream os;

    // NB: setfill is sticky for the entire run whereas setw gets reset for each
    // value.
    os << std::setfill('0');

    os << std::setw(4) << year
       << "-" << std::setw(2) << month
       << "-" << std::setw(2) << day
       << "T" << std::setw(2) << hour
       << ":" << std::setw(2) << minute
       << ":" << std::setw(2) << second;

    return os.str();
}

namespace {

namespace dump_format {

typedef mdds::sorted_string_map<dump_format_t> map_type;

// Keys must be sorted.
constexpr map_type::entry entries[] =
{
    { ORCUS_ASCII("check"),       dump_format_t::check       },
    { ORCUS_ASCII("csv"),         dump_format_t::csv         },
    { ORCUS_ASCII("debug-state"), dump_format_t::debug_state },
    { ORCUS_ASCII("flat"),        dump_format_t::flat        },
    { ORCUS_ASCII("html"),        dump_format_t::html        },
    { ORCUS_ASCII("json"),        dump_format_t::json        },
    { ORCUS_ASCII("none"),        dump_format_t::none        },
    { ORCUS_ASCII("xml"),         dump_format_t::xml         },
    { ORCUS_ASCII("yaml"),        dump_format_t::yaml        },
};

const map_type& get()
{
    static map_type mt(entries, std::size(entries), dump_format_t::unknown);
    return mt;
}

} // namespace dump_format

} // anonymous namespace

dump_format_t to_dump_format_enum(std::string_view s)
{
    return dump_format::get().find(s.data(), s.size());
}

std::vector<std::pair<std::string_view, dump_format_t>> get_dump_format_entries()
{
    std::vector<std::pair<std::string_view, dump_format_t>> ret;
    for (const auto& e : dump_format::entries)
        ret.emplace_back(std::string_view{e.key, e.keylen}, e.value);

    return ret;
}

std::ostream& operator<< (std::ostream& os, const length_t& v)
{
    os << v.to_string();
    return os;
}

std::ostream& operator<< (std::ostream& os, const date_time_t& v)
{
    os << v.to_string();
    return os;
}

std::ostream& operator<< (std::ostream& os, format_t v)
{
    static const char* values[] = {
        "unknown",
        "ods",
        "xlsx",
        "gnumeric",
        "xls-xml",
        "csv"
    };

    size_t vi = static_cast<std::underlying_type_t<format_t>>(v);
    size_t n = std::size(values);

    if (vi >= n)
        os << "???";
    else
        os << values[vi];

    return os;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
