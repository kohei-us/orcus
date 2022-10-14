/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/types.hpp>
#include <orcus/parser_global.hpp>
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

length_t::length_t(length_unit_t _unit, double _value) : unit(_unit), value(_value) {}

length_t::length_t(const length_t& other) = default;

length_t& length_t::operator= (const length_t& other) = default;

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

using map_type = mdds::sorted_string_map<dump_format_t, mdds::string_view_map_entry>;

// Keys must be sorted.
constexpr map_type::entry entries[] = {
    { "check",       dump_format_t::check       },
    { "csv",         dump_format_t::csv         },
    { "debug-state", dump_format_t::debug_state },
    { "flat",        dump_format_t::flat        },
    { "html",        dump_format_t::html        },
    { "json",        dump_format_t::json        },
    { "none",        dump_format_t::none        },
    { "xml",         dump_format_t::xml         },
    { "yaml",        dump_format_t::yaml        },
};

const map_type& get()
{
    static map_type mt(entries, std::size(entries), dump_format_t::unknown);
    return mt;
}

} // namespace dump_format

namespace charset {

using map_type = mdds::sorted_string_map<character_set_t, mdds::string_view_map_entry>;

// Keys must be sorted.
constexpr map_type::entry entries[] = {
    { "437", character_set_t::ibm437 },
    { "850", character_set_t::ibm850 },
    { "851", character_set_t::ibm851 },
    { "852", character_set_t::ibm852 },
    { "855", character_set_t::ibm855 },
    { "857", character_set_t::ibm857 },
    { "860", character_set_t::ibm860 },
    { "861", character_set_t::ibm861 },
    { "862", character_set_t::ibm862 },
    { "863", character_set_t::ibm863 },
    { "865", character_set_t::ibm865 },
    { "866", character_set_t::ibm866 },
    { "869", character_set_t::ibm869 },
    { "904", character_set_t::ibm904 },
    { "adobe-standard-encoding", character_set_t::adobe_standard_encoding },
    { "adobe-symbol-encoding", character_set_t::adobe_symbol_encoding },
    { "ami-1251", character_set_t::amiga_1251 },
    { "ami1251", character_set_t::amiga_1251 },
    { "amiga-1251", character_set_t::amiga_1251 },
    { "amiga1251", character_set_t::amiga_1251 },
    { "ansi_x3.110-1983", character_set_t::ansi_x3_110_1983 },
    { "ansi_x3.4-1968", character_set_t::us_ascii },
    { "ansi_x3.4-1986", character_set_t::us_ascii },
    { "arabic", character_set_t::iso_8859_6 },
    { "arabic7", character_set_t::asmo_449 },
    { "asmo-708", character_set_t::iso_8859_6 },
    { "asmo_449", character_set_t::asmo_449 },
    { "big5", character_set_t::big5 },
    { "big5-hkscs", character_set_t::big5_hkscs },
    { "bocu-1", character_set_t::bocu_1 },
    { "brf", character_set_t::brf },
    { "bs_4730", character_set_t::bs_4730 },
    { "bs_viewdata", character_set_t::bs_viewdata },
    { "ca", character_set_t::csa_z243_4_1985_1 },
    { "ccsid00858", character_set_t::ibm00858 },
    { "ccsid00924", character_set_t::ibm00924 },
    { "ccsid01140", character_set_t::ibm01140 },
    { "ccsid01141", character_set_t::ibm01141 },
    { "ccsid01142", character_set_t::ibm01142 },
    { "ccsid01143", character_set_t::ibm01143 },
    { "ccsid01144", character_set_t::ibm01144 },
    { "ccsid01145", character_set_t::ibm01145 },
    { "ccsid01146", character_set_t::ibm01146 },
    { "ccsid01147", character_set_t::ibm01147 },
    { "ccsid01148", character_set_t::ibm01148 },
    { "ccsid01149", character_set_t::ibm01149 },
    { "cesu-8", character_set_t::cesu_8 },
    { "chinese", character_set_t::gb_2312_80 },
    { "cn", character_set_t::gb_1988_80 },
    { "cp-ar", character_set_t::ibm868 },
    { "cp-gr", character_set_t::ibm869 },
    { "cp-is", character_set_t::ibm861 },
    { "cp00858", character_set_t::ibm00858 },
    { "cp00924", character_set_t::ibm00924 },
    { "cp01140", character_set_t::ibm01140 },
    { "cp01141", character_set_t::ibm01141 },
    { "cp01142", character_set_t::ibm01142 },
    { "cp01143", character_set_t::ibm01143 },
    { "cp01144", character_set_t::ibm01144 },
    { "cp01145", character_set_t::ibm01145 },
    { "cp01146", character_set_t::ibm01146 },
    { "cp01147", character_set_t::ibm01147 },
    { "cp01148", character_set_t::ibm01148 },
    { "cp01149", character_set_t::ibm01149 },
    { "cp037", character_set_t::ibm037 },
    { "cp038", character_set_t::ibm038 },
    { "cp1026", character_set_t::ibm1026 },
    { "cp154", character_set_t::ptcp154 },
    { "cp273", character_set_t::ibm273 },
    { "cp274", character_set_t::ibm274 },
    { "cp275", character_set_t::ibm275 },
    { "cp278", character_set_t::ibm278 },
    { "cp280", character_set_t::ibm280 },
    { "cp281", character_set_t::ibm281 },
    { "cp284", character_set_t::ibm284 },
    { "cp285", character_set_t::ibm285 },
    { "cp290", character_set_t::ibm290 },
    { "cp297", character_set_t::ibm297 },
    { "cp367", character_set_t::us_ascii },
    { "cp420", character_set_t::ibm420 },
    { "cp423", character_set_t::ibm423 },
    { "cp424", character_set_t::ibm424 },
    { "cp437", character_set_t::ibm437 },
    { "cp500", character_set_t::ibm500 },
    { "cp50220", character_set_t::cp50220 },
    { "cp51932", character_set_t::cp51932 },
    { "cp775", character_set_t::ibm775 },
    { "cp819", character_set_t::iso_8859_1 },
    { "cp850", character_set_t::ibm850 },
    { "cp851", character_set_t::ibm851 },
    { "cp852", character_set_t::ibm852 },
    { "cp855", character_set_t::ibm855 },
    { "cp857", character_set_t::ibm857 },
    { "cp860", character_set_t::ibm860 },
    { "cp861", character_set_t::ibm861 },
    { "cp862", character_set_t::ibm862 },
    { "cp863", character_set_t::ibm863 },
    { "cp864", character_set_t::ibm864 },
    { "cp865", character_set_t::ibm865 },
    { "cp866", character_set_t::ibm866 },
    { "cp868", character_set_t::ibm868 },
    { "cp869", character_set_t::ibm869 },
    { "cp870", character_set_t::ibm870 },
    { "cp871", character_set_t::ibm871 },
    { "cp880", character_set_t::ibm880 },
    { "cp891", character_set_t::ibm891 },
    { "cp903", character_set_t::ibm903 },
    { "cp904", character_set_t::ibm904 },
    { "cp905", character_set_t::ibm905 },
    { "cp918", character_set_t::ibm918 },
    { "cp936", character_set_t::gbk },
    { "csa7-1", character_set_t::csa_z243_4_1985_1 },
    { "csa7-2", character_set_t::csa_z243_4_1985_2 },
    { "csa71", character_set_t::csa_z243_4_1985_1 },
    { "csa72", character_set_t::csa_z243_4_1985_2 },
    { "csa_t500-1983", character_set_t::ansi_x3_110_1983 },
    { "csa_z243.4-1985-1", character_set_t::csa_z243_4_1985_1 },
    { "csa_z243.4-1985-2", character_set_t::csa_z243_4_1985_2 },
    { "csa_z243.4-1985-gr", character_set_t::csa_z243_4_1985_gr },
    { "csadobestandardencoding", character_set_t::adobe_standard_encoding },
    { "csascii", character_set_t::us_ascii },
    { "csbig5", character_set_t::big5 },
    { "csbig5hkscs", character_set_t::big5_hkscs },
    { "csbocu-1", character_set_t::bocu_1 },
    { "csbocu1", character_set_t::bocu_1 },
    { "csbrf", character_set_t::brf },
    { "cscesu-8", character_set_t::cesu_8 },
    { "cscesu8", character_set_t::cesu_8 },
    { "cscp50220", character_set_t::cp50220 },
    { "cscp51932", character_set_t::cp51932 },
    { "csdecmcs", character_set_t::dec_mcs },
    { "csdkus", character_set_t::dk_us },
    { "csebcdicatdea", character_set_t::ebcdic_at_de_a },
    { "csebcdiccafr", character_set_t::ebcdic_ca_fr },
    { "csebcdicdkno", character_set_t::ebcdic_dk_no },
    { "csebcdicdknoa", character_set_t::ebcdic_dk_no_a },
    { "csebcdices", character_set_t::ebcdic_es },
    { "csebcdicesa", character_set_t::ebcdic_es_a },
    { "csebcdicess", character_set_t::ebcdic_es_s },
    { "csebcdicfise", character_set_t::ebcdic_fi_se },
    { "csebcdicfisea", character_set_t::ebcdic_fi_se_a },
    { "csebcdicfr", character_set_t::ebcdic_fr },
    { "csebcdicit", character_set_t::ebcdic_it },
    { "csebcdicpt", character_set_t::ebcdic_pt },
    { "csebcdicuk", character_set_t::ebcdic_uk },
    { "csebcdicus", character_set_t::ebcdic_us },
    { "cseucfixwidjapanese", character_set_t::extended_unix_code_fixed_width_for_japanese },
    { "cseuckr", character_set_t::euc_kr },
    { "cseucpkdfmtjapanese", character_set_t::euc_jp },
    { "csgb18030", character_set_t::gb18030 },
    { "csgb2312", character_set_t::gb2312 },
    { "csgbk", character_set_t::gbk },
    { "cshalfwidthkatakana", character_set_t::jis_x0201 },
    { "cshpdesktop", character_set_t::hp_desktop },
    { "cshplegal", character_set_t::hp_legal },
    { "cshpmath8", character_set_t::hp_math8 },
    { "cshppifont", character_set_t::hp_pi_font },
    { "cshppsmath", character_set_t::adobe_symbol_encoding },
    { "cshproman8", character_set_t::hp_roman8 },
    { "csibbm904", character_set_t::ibm904 },
    { "csibm00858", character_set_t::ibm00858 },
    { "csibm00924", character_set_t::ibm00924 },
    { "csibm01140", character_set_t::ibm01140 },
    { "csibm01141", character_set_t::ibm01141 },
    { "csibm01142", character_set_t::ibm01142 },
    { "csibm01143", character_set_t::ibm01143 },
    { "csibm01144", character_set_t::ibm01144 },
    { "csibm01145", character_set_t::ibm01145 },
    { "csibm01146", character_set_t::ibm01146 },
    { "csibm01147", character_set_t::ibm01147 },
    { "csibm01148", character_set_t::ibm01148 },
    { "csibm01149", character_set_t::ibm01149 },
    { "csibm037", character_set_t::ibm037 },
    { "csibm038", character_set_t::ibm038 },
    { "csibm1026", character_set_t::ibm1026 },
    { "csibm1047", character_set_t::ibm1047 },
    { "csibm273", character_set_t::ibm273 },
    { "csibm274", character_set_t::ibm274 },
    { "csibm275", character_set_t::ibm275 },
    { "csibm277", character_set_t::ibm277 },
    { "csibm278", character_set_t::ibm278 },
    { "csibm280", character_set_t::ibm280 },
    { "csibm281", character_set_t::ibm281 },
    { "csibm284", character_set_t::ibm284 },
    { "csibm285", character_set_t::ibm285 },
    { "csibm290", character_set_t::ibm290 },
    { "csibm297", character_set_t::ibm297 },
    { "csibm420", character_set_t::ibm420 },
    { "csibm423", character_set_t::ibm423 },
    { "csibm424", character_set_t::ibm424 },
    { "csibm500", character_set_t::ibm500 },
    { "csibm851", character_set_t::ibm851 },
    { "csibm855", character_set_t::ibm855 },
    { "csibm857", character_set_t::ibm857 },
    { "csibm860", character_set_t::ibm860 },
    { "csibm861", character_set_t::ibm861 },
    { "csibm863", character_set_t::ibm863 },
    { "csibm864", character_set_t::ibm864 },
    { "csibm865", character_set_t::ibm865 },
    { "csibm866", character_set_t::ibm866 },
    { "csibm868", character_set_t::ibm868 },
    { "csibm869", character_set_t::ibm869 },
    { "csibm870", character_set_t::ibm870 },
    { "csibm871", character_set_t::ibm871 },
    { "csibm880", character_set_t::ibm880 },
    { "csibm891", character_set_t::ibm891 },
    { "csibm903", character_set_t::ibm903 },
    { "csibm905", character_set_t::ibm905 },
    { "csibm918", character_set_t::ibm918 },
    { "csibmebcdicatde", character_set_t::ebcdic_at_de },
    { "csibmsymbols", character_set_t::ibm_symbols },
    { "csibmthai", character_set_t::ibm_thai },
    { "csinvariant", character_set_t::invariant },
    { "csiso102t617bit", character_set_t::t_61_7bit },
    { "csiso10367box", character_set_t::iso_10367_box },
    { "csiso103t618bit", character_set_t::t_61_8bit },
    { "csiso10646utf1", character_set_t::iso_10646_utf_1 },
    { "csiso10swedish", character_set_t::sen_850200_b },
    { "csiso111ecmacyrillic", character_set_t::ecma_cyrillic },
    { "csiso115481", character_set_t::iso_11548_1 },
    { "csiso11swedishfornames", character_set_t::sen_850200_c },
    { "csiso121canadian1", character_set_t::csa_z243_4_1985_1 },
    { "csiso122canadian2", character_set_t::csa_z243_4_1985_2 },
    { "csiso123csaz24341985gr", character_set_t::csa_z243_4_1985_gr },
    { "csiso128t101g2", character_set_t::t_101_g2 },
    { "csiso139csn369103", character_set_t::csn_369103 },
    { "csiso13jisc6220jp", character_set_t::jis_c6220_1969_jp },
    { "csiso141jusib1002", character_set_t::jus_i_b1_002 },
    { "csiso143iecp271", character_set_t::iec_p27_1 },
    { "csiso146serbian", character_set_t::jus_i_b1_003_serb },
    { "csiso147macedonian", character_set_t::jus_i_b1_003_mac },
    { "csiso14jisc6220ro", character_set_t::jis_c6220_1969_ro },
    { "csiso150", character_set_t::greek_ccitt },
    { "csiso150greekccitt", character_set_t::greek_ccitt },
    { "csiso151cuba", character_set_t::nc_nc00_10_81 },
    { "csiso153gost1976874", character_set_t::gost_19768_74 },
    { "csiso158lap", character_set_t::latin_lap },
    { "csiso159jisx02121990", character_set_t::jis_x0212_1990 },
    { "csiso15italian", character_set_t::it },
    { "csiso16portuguese", character_set_t::pt },
    { "csiso17spanish", character_set_t::es },
    { "csiso18greek7old", character_set_t::greek7_old },
    { "csiso19latingreek", character_set_t::latin_greek },
    { "csiso2022cn", character_set_t::iso_2022_cn },
    { "csiso2022cnext", character_set_t::iso_2022_cn_ext },
    { "csiso2022jp", character_set_t::iso_2022_jp },
    { "csiso2022jp2", character_set_t::iso_2022_jp_2 },
    { "csiso2022kr", character_set_t::iso_2022_kr },
    { "csiso2033", character_set_t::iso_2033_1983 },
    { "csiso21german", character_set_t::din_66003 },
    { "csiso25french", character_set_t::nf_z_62_010_1973 },
    { "csiso27latingreek1", character_set_t::latin_greek_1 },
    { "csiso2intlrefversion", character_set_t::iso_646_irv_1983 },
    { "csiso42jisc62261978", character_set_t::jis_c6226_1978 },
    { "csiso47bsviewdata", character_set_t::bs_viewdata },
    { "csiso49inis", character_set_t::inis },
    { "csiso4unitedkingdom", character_set_t::bs_4730 },
    { "csiso50inis8", character_set_t::inis_8 },
    { "csiso51iniscyrillic", character_set_t::inis_cyrillic },
    { "csiso54271981", character_set_t::iso_5427_1981 },
    { "csiso5427cyrillic", character_set_t::iso_5427 },
    { "csiso5428greek", character_set_t::iso_5428_1980 },
    { "csiso57gb1988", character_set_t::gb_1988_80 },
    { "csiso58gb231280", character_set_t::gb_2312_80 },
    { "csiso60danishnorwegian", character_set_t::ns_4551_1 },
    { "csiso60norwegian1", character_set_t::ns_4551_1 },
    { "csiso61norwegian2", character_set_t::ns_4551_2 },
    { "csiso646basic1983", character_set_t::iso_646_basic_1983 },
    { "csiso646danish", character_set_t::ds_2089 },
    { "csiso6937add", character_set_t::iso_6937_2_25 },
    { "csiso69french", character_set_t::nf_z_62_010 },
    { "csiso70videotexsupp1", character_set_t::videotex_suppl },
    { "csiso84portuguese2", character_set_t::pt2 },
    { "csiso85spanish2", character_set_t::es2 },
    { "csiso86hungarian", character_set_t::msz_7795_3 },
    { "csiso87jisx0208", character_set_t::jis_c6226_1983 },
    { "csiso885913", character_set_t::iso_8859_13 },
    { "csiso885914", character_set_t::iso_8859_14 },
    { "csiso885915", character_set_t::iso_8859_15 },
    { "csiso885916", character_set_t::iso_8859_16 },
    { "csiso88596e", character_set_t::iso_8859_6_e },
    { "csiso88596i", character_set_t::iso_8859_6_i },
    { "csiso88598e", character_set_t::iso_8859_8_e },
    { "csiso88598i", character_set_t::iso_8859_8_i },
    { "csiso8859supp", character_set_t::iso_8859_supp },
    { "csiso88greek7", character_set_t::greek7 },
    { "csiso89asmo449", character_set_t::asmo_449 },
    { "csiso90", character_set_t::iso_ir_90 },
    { "csiso91jisc62291984a", character_set_t::jis_c6229_1984_a },
    { "csiso92jisc62991984b", character_set_t::jis_c6229_1984_b },
    { "csiso93jis62291984badd", character_set_t::jis_c6229_1984_b_add },
    { "csiso94jis62291984hand", character_set_t::jis_c6229_1984_hand },
    { "csiso95jis62291984handadd", character_set_t::jis_c6229_1984_hand_add },
    { "csiso96jisc62291984kana", character_set_t::jis_c6229_1984_kana },
    { "csiso99naplps", character_set_t::ansi_x3_110_1983 },
    { "csisolatin1", character_set_t::iso_8859_1 },
    { "csisolatin2", character_set_t::iso_8859_2 },
    { "csisolatin3", character_set_t::iso_8859_3 },
    { "csisolatin4", character_set_t::iso_8859_4 },
    { "csisolatin5", character_set_t::iso_8859_9 },
    { "csisolatin6", character_set_t::iso_8859_10 },
    { "csisolatinarabic", character_set_t::iso_8859_6 },
    { "csisolatincyrillic", character_set_t::iso_8859_5 },
    { "csisolatingreek", character_set_t::iso_8859_7 },
    { "csisolatinhebrew", character_set_t::iso_8859_8 },
    { "csisotextcomm", character_set_t::iso_6937_2_add },
    { "csjisencoding", character_set_t::jis_encoding },
    { "cskoi7switched", character_set_t::koi7_switched },
    { "cskoi8r", character_set_t::koi8_r },
    { "cskoi8u", character_set_t::koi8_u },
    { "csksc56011987", character_set_t::ks_c_5601_1987 },
    { "csksc5636", character_set_t::ksc5636 },
    { "cskz1048", character_set_t::kz_1048 },
    { "csmacintosh", character_set_t::macintosh },
    { "csmicrosoftpublishing", character_set_t::microsoft_publishing },
    { "csmnem", character_set_t::mnem },
    { "csmnemonic", character_set_t::mnemonic },
    { "csn_369103", character_set_t::csn_369103 },
    { "csnatsdano", character_set_t::nats_dano },
    { "csnatsdanoadd", character_set_t::nats_dano_add },
    { "csnatssefi", character_set_t::nats_sefi },
    { "csnatssefiadd", character_set_t::nats_sefi_add },
    { "csosdebcdicdf03irv", character_set_t::osd_ebcdic_df03_irv },
    { "csosdebcdicdf041", character_set_t::osd_ebcdic_df04_1 },
    { "csosdebcdicdf0415", character_set_t::osd_ebcdic_df04_15 },
    { "cspc775baltic", character_set_t::ibm775 },
    { "cspc850multilingual", character_set_t::ibm850 },
    { "cspc862latinhebrew", character_set_t::ibm862 },
    { "cspc8codepage437", character_set_t::ibm437 },
    { "cspc8danishnorwegian", character_set_t::pc8_danish_norwegian },
    { "cspc8turkish", character_set_t::pc8_turkish },
    { "cspcp852", character_set_t::ibm852 },
    { "csptcp154", character_set_t::ptcp154 },
    { "csscsu", character_set_t::scsu },
    { "csshiftjis", character_set_t::shift_jis },
    { "cstis620", character_set_t::tis_620 },
    { "cstscii", character_set_t::tscii },
    { "csucs4", character_set_t::iso_10646_ucs_4 },
    { "csunicode", character_set_t::iso_10646_ucs_2 },
    { "csunicode11", character_set_t::unicode_1_1 },
    { "csunicode11utf7", character_set_t::unicode_1_1_utf_7 },
    { "csunicodeascii", character_set_t::iso_10646_ucs_basic },
    { "csunicodeibm1261", character_set_t::iso_unicode_ibm_1261 },
    { "csunicodeibm1264", character_set_t::iso_unicode_ibm_1264 },
    { "csunicodeibm1265", character_set_t::iso_unicode_ibm_1265 },
    { "csunicodeibm1268", character_set_t::iso_unicode_ibm_1268 },
    { "csunicodeibm1276", character_set_t::iso_unicode_ibm_1276 },
    { "csunicodejapanese", character_set_t::iso_10646_j_1 },
    { "csunicodelatin1", character_set_t::iso_10646_unicode_latin1 },
    { "csunknown8bit", character_set_t::unknown_8bit },
    { "csusdk", character_set_t::us_dk },
    { "csutf16", character_set_t::utf_16 },
    { "csutf16be", character_set_t::utf_16be },
    { "csutf16le", character_set_t::utf_16le },
    { "csutf32", character_set_t::utf_32 },
    { "csutf32be", character_set_t::utf_32be },
    { "csutf32le", character_set_t::utf_32le },
    { "csutf7", character_set_t::utf_7 },
    { "csutf7imap", character_set_t::utf_7_imap },
    { "csutf8", character_set_t::utf_8 },
    { "csventurainternational", character_set_t::ventura_international },
    { "csventuramath", character_set_t::ventura_math },
    { "csventuraus", character_set_t::ventura_us },
    { "csviqr", character_set_t::viqr },
    { "csviscii", character_set_t::viscii },
    { "cswindows1250", character_set_t::windows_1250 },
    { "cswindows1251", character_set_t::windows_1251 },
    { "cswindows1252", character_set_t::windows_1252 },
    { "cswindows1253", character_set_t::windows_1253 },
    { "cswindows1254", character_set_t::windows_1254 },
    { "cswindows1255", character_set_t::windows_1255 },
    { "cswindows1256", character_set_t::windows_1256 },
    { "cswindows1257", character_set_t::windows_1257 },
    { "cswindows1258", character_set_t::windows_1258 },
    { "cswindows30latin1", character_set_t::iso_8859_1_windows_3_0_latin_1 },
    { "cswindows31j", character_set_t::windows_31j },
    { "cswindows31latin1", character_set_t::iso_8859_1_windows_3_1_latin_1 },
    { "cswindows31latin2", character_set_t::iso_8859_2_windows_latin_2 },
    { "cswindows31latin5", character_set_t::iso_8859_9_windows_latin_5 },
    { "cswindows874", character_set_t::windows_874 },
    { "cuba", character_set_t::nc_nc00_10_81 },
    { "cyrillic", character_set_t::iso_8859_5 },
    { "cyrillic-asian", character_set_t::ptcp154 },
    { "de", character_set_t::din_66003 },
    { "dec", character_set_t::dec_mcs },
    { "dec-mcs", character_set_t::dec_mcs },
    { "din_66003", character_set_t::din_66003 },
    { "dk", character_set_t::ds_2089 },
    { "dk-us", character_set_t::dk_us },
    { "ds2089", character_set_t::ds_2089 },
    { "ds_2089", character_set_t::ds_2089 },
    { "e13b", character_set_t::iso_2033_1983 },
    { "ebcdic-at-de", character_set_t::ebcdic_at_de },
    { "ebcdic-at-de-a", character_set_t::ebcdic_at_de_a },
    { "ebcdic-be", character_set_t::ibm274 },
    { "ebcdic-br", character_set_t::ibm275 },
    { "ebcdic-ca-fr", character_set_t::ebcdic_ca_fr },
    { "ebcdic-cp-ar1", character_set_t::ibm420 },
    { "ebcdic-cp-ar2", character_set_t::ibm918 },
    { "ebcdic-cp-be", character_set_t::ibm500 },
    { "ebcdic-cp-ca", character_set_t::ibm037 },
    { "ebcdic-cp-ch", character_set_t::ibm500 },
    { "ebcdic-cp-dk", character_set_t::ibm277 },
    { "ebcdic-cp-es", character_set_t::ibm284 },
    { "ebcdic-cp-fi", character_set_t::ibm278 },
    { "ebcdic-cp-fr", character_set_t::ibm297 },
    { "ebcdic-cp-gb", character_set_t::ibm285 },
    { "ebcdic-cp-gr", character_set_t::ibm423 },
    { "ebcdic-cp-he", character_set_t::ibm424 },
    { "ebcdic-cp-is", character_set_t::ibm871 },
    { "ebcdic-cp-it", character_set_t::ibm280 },
    { "ebcdic-cp-nl", character_set_t::ibm037 },
    { "ebcdic-cp-no", character_set_t::ibm277 },
    { "ebcdic-cp-roece", character_set_t::ibm870 },
    { "ebcdic-cp-se", character_set_t::ibm278 },
    { "ebcdic-cp-tr", character_set_t::ibm905 },
    { "ebcdic-cp-us", character_set_t::ibm037 },
    { "ebcdic-cp-wt", character_set_t::ibm037 },
    { "ebcdic-cp-yu", character_set_t::ibm870 },
    { "ebcdic-cyrillic", character_set_t::ibm880 },
    { "ebcdic-de-273+euro", character_set_t::ibm01141 },
    { "ebcdic-dk-277+euro", character_set_t::ibm01142 },
    { "ebcdic-dk-no", character_set_t::ebcdic_dk_no },
    { "ebcdic-dk-no-a", character_set_t::ebcdic_dk_no_a },
    { "ebcdic-es", character_set_t::ebcdic_es },
    { "ebcdic-es-284+euro", character_set_t::ibm01145 },
    { "ebcdic-es-a", character_set_t::ebcdic_es_a },
    { "ebcdic-es-s", character_set_t::ebcdic_es_s },
    { "ebcdic-fi-278+euro", character_set_t::ibm01143 },
    { "ebcdic-fi-se", character_set_t::ebcdic_fi_se },
    { "ebcdic-fi-se-a", character_set_t::ebcdic_fi_se_a },
    { "ebcdic-fr", character_set_t::ebcdic_fr },
    { "ebcdic-fr-297+euro", character_set_t::ibm01147 },
    { "ebcdic-gb-285+euro", character_set_t::ibm01146 },
    { "ebcdic-int", character_set_t::ibm038 },
    { "ebcdic-international-500+euro", character_set_t::ibm01148 },
    { "ebcdic-is-871+euro", character_set_t::ibm01149 },
    { "ebcdic-it", character_set_t::ebcdic_it },
    { "ebcdic-it-280+euro", character_set_t::ibm01144 },
    { "ebcdic-jp-e", character_set_t::ibm281 },
    { "ebcdic-jp-kana", character_set_t::ibm290 },
    { "ebcdic-latin9--euro", character_set_t::ibm00924 },
    { "ebcdic-no-277+euro", character_set_t::ibm01142 },
    { "ebcdic-pt", character_set_t::ebcdic_pt },
    { "ebcdic-se-278+euro", character_set_t::ibm01143 },
    { "ebcdic-uk", character_set_t::ebcdic_uk },
    { "ebcdic-us", character_set_t::ebcdic_us },
    { "ebcdic-us-37+euro", character_set_t::ibm01140 },
    { "ecma-114", character_set_t::iso_8859_6 },
    { "ecma-118", character_set_t::iso_8859_7 },
    { "ecma-cyrillic", character_set_t::ecma_cyrillic },
    { "elot_928", character_set_t::iso_8859_7 },
    { "es", character_set_t::es },
    { "es2", character_set_t::es2 },
    { "euc-jp", character_set_t::euc_jp },
    { "euc-kr", character_set_t::euc_kr },
    { "extended_unix_code_fixed_width_for_japanese", character_set_t::extended_unix_code_fixed_width_for_japanese },
    { "extended_unix_code_packed_format_for_japanese", character_set_t::euc_jp },
    { "fi", character_set_t::sen_850200_b },
    { "fr", character_set_t::nf_z_62_010 },
    { "gb", character_set_t::bs_4730 },
    { "gb18030", character_set_t::gb18030 },
    { "gb2312", character_set_t::gb2312 },
    { "gb_1988-80", character_set_t::gb_1988_80 },
    { "gb_2312-80", character_set_t::gb_2312_80 },
    { "gbk", character_set_t::gbk },
    { "gost_19768-74", character_set_t::gost_19768_74 },
    { "greek", character_set_t::iso_8859_7 },
    { "greek-ccitt", character_set_t::greek_ccitt },
    { "greek7", character_set_t::greek7 },
    { "greek7-old", character_set_t::greek7_old },
    { "greek8", character_set_t::iso_8859_7 },
    { "hebrew", character_set_t::iso_8859_8 },
    { "hp-desktop", character_set_t::hp_desktop },
    { "hp-legal", character_set_t::hp_legal },
    { "hp-math8", character_set_t::hp_math8 },
    { "hp-pi-font", character_set_t::hp_pi_font },
    { "hp-roman8", character_set_t::hp_roman8 },
    { "hu", character_set_t::msz_7795_3 },
    { "hz-gb-2312", character_set_t::hz_gb_2312 },
    { "ibm-1047", character_set_t::ibm1047 },
    { "ibm-symbols", character_set_t::ibm_symbols },
    { "ibm-thai", character_set_t::ibm_thai },
    { "ibm00858", character_set_t::ibm00858 },
    { "ibm00924", character_set_t::ibm00924 },
    { "ibm01140", character_set_t::ibm01140 },
    { "ibm01141", character_set_t::ibm01141 },
    { "ibm01142", character_set_t::ibm01142 },
    { "ibm01143", character_set_t::ibm01143 },
    { "ibm01144", character_set_t::ibm01144 },
    { "ibm01145", character_set_t::ibm01145 },
    { "ibm01146", character_set_t::ibm01146 },
    { "ibm01147", character_set_t::ibm01147 },
    { "ibm01148", character_set_t::ibm01148 },
    { "ibm01149", character_set_t::ibm01149 },
    { "ibm037", character_set_t::ibm037 },
    { "ibm038", character_set_t::ibm038 },
    { "ibm1026", character_set_t::ibm1026 },
    { "ibm1047", character_set_t::ibm1047 },
    { "ibm273", character_set_t::ibm273 },
    { "ibm274", character_set_t::ibm274 },
    { "ibm275", character_set_t::ibm275 },
    { "ibm277", character_set_t::ibm277 },
    { "ibm278", character_set_t::ibm278 },
    { "ibm280", character_set_t::ibm280 },
    { "ibm281", character_set_t::ibm281 },
    { "ibm284", character_set_t::ibm284 },
    { "ibm285", character_set_t::ibm285 },
    { "ibm290", character_set_t::ibm290 },
    { "ibm297", character_set_t::ibm297 },
    { "ibm367", character_set_t::us_ascii },
    { "ibm420", character_set_t::ibm420 },
    { "ibm423", character_set_t::ibm423 },
    { "ibm424", character_set_t::ibm424 },
    { "ibm437", character_set_t::ibm437 },
    { "ibm500", character_set_t::ibm500 },
    { "ibm775", character_set_t::ibm775 },
    { "ibm819", character_set_t::iso_8859_1 },
    { "ibm850", character_set_t::ibm850 },
    { "ibm851", character_set_t::ibm851 },
    { "ibm852", character_set_t::ibm852 },
    { "ibm855", character_set_t::ibm855 },
    { "ibm857", character_set_t::ibm857 },
    { "ibm860", character_set_t::ibm860 },
    { "ibm861", character_set_t::ibm861 },
    { "ibm862", character_set_t::ibm862 },
    { "ibm863", character_set_t::ibm863 },
    { "ibm864", character_set_t::ibm864 },
    { "ibm865", character_set_t::ibm865 },
    { "ibm866", character_set_t::ibm866 },
    { "ibm868", character_set_t::ibm868 },
    { "ibm869", character_set_t::ibm869 },
    { "ibm870", character_set_t::ibm870 },
    { "ibm871", character_set_t::ibm871 },
    { "ibm880", character_set_t::ibm880 },
    { "ibm891", character_set_t::ibm891 },
    { "ibm903", character_set_t::ibm903 },
    { "ibm904", character_set_t::ibm904 },
    { "ibm905", character_set_t::ibm905 },
    { "ibm918", character_set_t::ibm918 },
    { "iec_p27-1", character_set_t::iec_p27_1 },
    { "inis", character_set_t::inis },
    { "inis-8", character_set_t::inis_8 },
    { "inis-cyrillic", character_set_t::inis_cyrillic },
    { "invariant", character_set_t::invariant },
    { "irv", character_set_t::iso_646_irv_1983 },
    { "iso-10646", character_set_t::iso_10646_unicode_latin1 },
    { "iso-10646-j-1", character_set_t::iso_10646_j_1 },
    { "iso-10646-ucs-2", character_set_t::iso_10646_ucs_2 },
    { "iso-10646-ucs-4", character_set_t::iso_10646_ucs_4 },
    { "iso-10646-ucs-basic", character_set_t::iso_10646_ucs_basic },
    { "iso-10646-unicode-latin1", character_set_t::iso_10646_unicode_latin1 },
    { "iso-10646-utf-1", character_set_t::iso_10646_utf_1 },
    { "iso-11548-1", character_set_t::iso_11548_1 },
    { "iso-2022-cn", character_set_t::iso_2022_cn },
    { "iso-2022-cn-ext", character_set_t::iso_2022_cn_ext },
    { "iso-2022-jp", character_set_t::iso_2022_jp },
    { "iso-2022-jp-2", character_set_t::iso_2022_jp_2 },
    { "iso-2022-kr", character_set_t::iso_2022_kr },
    { "iso-8859-1", character_set_t::iso_8859_1 },
    { "iso-8859-1-windows-3.0-latin-1", character_set_t::iso_8859_1_windows_3_0_latin_1 },
    { "iso-8859-1-windows-3.1-latin-1", character_set_t::iso_8859_1_windows_3_1_latin_1 },
    { "iso-8859-10", character_set_t::iso_8859_10 },
    { "iso-8859-11", character_set_t::tis_620 },
    { "iso-8859-13", character_set_t::iso_8859_13 },
    { "iso-8859-14", character_set_t::iso_8859_14 },
    { "iso-8859-15", character_set_t::iso_8859_15 },
    { "iso-8859-16", character_set_t::iso_8859_16 },
    { "iso-8859-2", character_set_t::iso_8859_2 },
    { "iso-8859-2-windows-latin-2", character_set_t::iso_8859_2_windows_latin_2 },
    { "iso-8859-3", character_set_t::iso_8859_3 },
    { "iso-8859-4", character_set_t::iso_8859_4 },
    { "iso-8859-5", character_set_t::iso_8859_5 },
    { "iso-8859-6", character_set_t::iso_8859_6 },
    { "iso-8859-6-e", character_set_t::iso_8859_6_e },
    { "iso-8859-6-i", character_set_t::iso_8859_6_i },
    { "iso-8859-7", character_set_t::iso_8859_7 },
    { "iso-8859-8", character_set_t::iso_8859_8 },
    { "iso-8859-8-e", character_set_t::iso_8859_8_e },
    { "iso-8859-8-i", character_set_t::iso_8859_8_i },
    { "iso-8859-9", character_set_t::iso_8859_9 },
    { "iso-8859-9-windows-latin-5", character_set_t::iso_8859_9_windows_latin_5 },
    { "iso-celtic", character_set_t::iso_8859_14 },
    { "iso-ir-10", character_set_t::sen_850200_b },
    { "iso-ir-100", character_set_t::iso_8859_1 },
    { "iso-ir-101", character_set_t::iso_8859_2 },
    { "iso-ir-102", character_set_t::t_61_7bit },
    { "iso-ir-103", character_set_t::t_61_8bit },
    { "iso-ir-109", character_set_t::iso_8859_3 },
    { "iso-ir-11", character_set_t::sen_850200_c },
    { "iso-ir-110", character_set_t::iso_8859_4 },
    { "iso-ir-111", character_set_t::ecma_cyrillic },
    { "iso-ir-121", character_set_t::csa_z243_4_1985_1 },
    { "iso-ir-122", character_set_t::csa_z243_4_1985_2 },
    { "iso-ir-123", character_set_t::csa_z243_4_1985_gr },
    { "iso-ir-126", character_set_t::iso_8859_7 },
    { "iso-ir-127", character_set_t::iso_8859_6 },
    { "iso-ir-128", character_set_t::t_101_g2 },
    { "iso-ir-13", character_set_t::jis_c6220_1969_jp },
    { "iso-ir-138", character_set_t::iso_8859_8 },
    { "iso-ir-139", character_set_t::csn_369103 },
    { "iso-ir-14", character_set_t::jis_c6220_1969_ro },
    { "iso-ir-141", character_set_t::jus_i_b1_002 },
    { "iso-ir-142", character_set_t::iso_6937_2_add },
    { "iso-ir-143", character_set_t::iec_p27_1 },
    { "iso-ir-144", character_set_t::iso_8859_5 },
    { "iso-ir-146", character_set_t::jus_i_b1_003_serb },
    { "iso-ir-147", character_set_t::jus_i_b1_003_mac },
    { "iso-ir-148", character_set_t::iso_8859_9 },
    { "iso-ir-149", character_set_t::ks_c_5601_1987 },
    { "iso-ir-15", character_set_t::it },
    { "iso-ir-150", character_set_t::greek_ccitt },
    { "iso-ir-151", character_set_t::nc_nc00_10_81 },
    { "iso-ir-152", character_set_t::iso_6937_2_25 },
    { "iso-ir-153", character_set_t::gost_19768_74 },
    { "iso-ir-154", character_set_t::iso_8859_supp },
    { "iso-ir-155", character_set_t::iso_10367_box },
    { "iso-ir-157", character_set_t::iso_8859_10 },
    { "iso-ir-158", character_set_t::latin_lap },
    { "iso-ir-159", character_set_t::jis_x0212_1990 },
    { "iso-ir-16", character_set_t::pt },
    { "iso-ir-17", character_set_t::es },
    { "iso-ir-18", character_set_t::greek7_old },
    { "iso-ir-19", character_set_t::latin_greek },
    { "iso-ir-199", character_set_t::iso_8859_14 },
    { "iso-ir-2", character_set_t::iso_646_irv_1983 },
    { "iso-ir-21", character_set_t::din_66003 },
    { "iso-ir-226", character_set_t::iso_8859_16 },
    { "iso-ir-25", character_set_t::nf_z_62_010_1973 },
    { "iso-ir-27", character_set_t::latin_greek_1 },
    { "iso-ir-37", character_set_t::iso_5427 },
    { "iso-ir-4", character_set_t::bs_4730 },
    { "iso-ir-42", character_set_t::jis_c6226_1978 },
    { "iso-ir-47", character_set_t::bs_viewdata },
    { "iso-ir-49", character_set_t::inis },
    { "iso-ir-50", character_set_t::inis_8 },
    { "iso-ir-51", character_set_t::inis_cyrillic },
    { "iso-ir-54", character_set_t::iso_5427_1981 },
    { "iso-ir-55", character_set_t::iso_5428_1980 },
    { "iso-ir-57", character_set_t::gb_1988_80 },
    { "iso-ir-58", character_set_t::gb_2312_80 },
    { "iso-ir-6", character_set_t::us_ascii },
    { "iso-ir-60", character_set_t::ns_4551_1 },
    { "iso-ir-61", character_set_t::ns_4551_2 },
    { "iso-ir-69", character_set_t::nf_z_62_010 },
    { "iso-ir-70", character_set_t::videotex_suppl },
    { "iso-ir-8-1", character_set_t::nats_sefi },
    { "iso-ir-8-2", character_set_t::nats_sefi_add },
    { "iso-ir-84", character_set_t::pt2 },
    { "iso-ir-85", character_set_t::es2 },
    { "iso-ir-86", character_set_t::msz_7795_3 },
    { "iso-ir-87", character_set_t::jis_c6226_1983 },
    { "iso-ir-88", character_set_t::greek7 },
    { "iso-ir-89", character_set_t::asmo_449 },
    { "iso-ir-9-1", character_set_t::nats_dano },
    { "iso-ir-9-2", character_set_t::nats_dano_add },
    { "iso-ir-90", character_set_t::iso_ir_90 },
    { "iso-ir-91", character_set_t::jis_c6229_1984_a },
    { "iso-ir-92", character_set_t::jis_c6229_1984_b },
    { "iso-ir-93", character_set_t::jis_c6229_1984_b_add },
    { "iso-ir-94", character_set_t::jis_c6229_1984_hand },
    { "iso-ir-95", character_set_t::jis_c6229_1984_hand_add },
    { "iso-ir-96", character_set_t::jis_c6229_1984_kana },
    { "iso-ir-98", character_set_t::iso_2033_1983 },
    { "iso-ir-99", character_set_t::ansi_x3_110_1983 },
    { "iso-unicode-ibm-1261", character_set_t::iso_unicode_ibm_1261 },
    { "iso-unicode-ibm-1264", character_set_t::iso_unicode_ibm_1264 },
    { "iso-unicode-ibm-1265", character_set_t::iso_unicode_ibm_1265 },
    { "iso-unicode-ibm-1268", character_set_t::iso_unicode_ibm_1268 },
    { "iso-unicode-ibm-1276", character_set_t::iso_unicode_ibm_1276 },
    { "iso5427cyrillic1981", character_set_t::iso_5427_1981 },
    { "iso646-ca", character_set_t::csa_z243_4_1985_1 },
    { "iso646-ca2", character_set_t::csa_z243_4_1985_2 },
    { "iso646-cn", character_set_t::gb_1988_80 },
    { "iso646-cu", character_set_t::nc_nc00_10_81 },
    { "iso646-de", character_set_t::din_66003 },
    { "iso646-dk", character_set_t::ds_2089 },
    { "iso646-es", character_set_t::es },
    { "iso646-es2", character_set_t::es2 },
    { "iso646-fi", character_set_t::sen_850200_b },
    { "iso646-fr", character_set_t::nf_z_62_010 },
    { "iso646-fr1", character_set_t::nf_z_62_010_1973 },
    { "iso646-gb", character_set_t::bs_4730 },
    { "iso646-hu", character_set_t::msz_7795_3 },
    { "iso646-it", character_set_t::it },
    { "iso646-jp", character_set_t::jis_c6220_1969_ro },
    { "iso646-jp-ocr-b", character_set_t::jis_c6229_1984_b },
    { "iso646-kr", character_set_t::ksc5636 },
    { "iso646-no", character_set_t::ns_4551_1 },
    { "iso646-no2", character_set_t::ns_4551_2 },
    { "iso646-pt", character_set_t::pt },
    { "iso646-pt2", character_set_t::pt2 },
    { "iso646-se", character_set_t::sen_850200_b },
    { "iso646-se2", character_set_t::sen_850200_c },
    { "iso646-us", character_set_t::us_ascii },
    { "iso646-yu", character_set_t::jus_i_b1_002 },
    { "iso_10367-box", character_set_t::iso_10367_box },
    { "iso_11548-1", character_set_t::iso_11548_1 },
    { "iso_2033-1983", character_set_t::iso_2033_1983 },
    { "iso_5427", character_set_t::iso_5427 },
    { "iso_5427:1981", character_set_t::iso_5427_1981 },
    { "iso_5428:1980", character_set_t::iso_5428_1980 },
    { "iso_646.basic:1983", character_set_t::iso_646_basic_1983 },
    { "iso_646.irv:1983", character_set_t::iso_646_irv_1983 },
    { "iso_646.irv:1991", character_set_t::us_ascii },
    { "iso_6937-2-25", character_set_t::iso_6937_2_25 },
    { "iso_6937-2-add", character_set_t::iso_6937_2_add },
    { "iso_8859-1", character_set_t::iso_8859_1 },
    { "iso_8859-10:1992", character_set_t::iso_8859_10 },
    { "iso_8859-14", character_set_t::iso_8859_14 },
    { "iso_8859-14:1998", character_set_t::iso_8859_14 },
    { "iso_8859-15", character_set_t::iso_8859_15 },
    { "iso_8859-16", character_set_t::iso_8859_16 },
    { "iso_8859-16:2001", character_set_t::iso_8859_16 },
    { "iso_8859-1:1987", character_set_t::iso_8859_1 },
    { "iso_8859-2", character_set_t::iso_8859_2 },
    { "iso_8859-2:1987", character_set_t::iso_8859_2 },
    { "iso_8859-3", character_set_t::iso_8859_3 },
    { "iso_8859-3:1988", character_set_t::iso_8859_3 },
    { "iso_8859-4", character_set_t::iso_8859_4 },
    { "iso_8859-4:1988", character_set_t::iso_8859_4 },
    { "iso_8859-5", character_set_t::iso_8859_5 },
    { "iso_8859-5:1988", character_set_t::iso_8859_5 },
    { "iso_8859-6", character_set_t::iso_8859_6 },
    { "iso_8859-6-e", character_set_t::iso_8859_6_e },
    { "iso_8859-6-i", character_set_t::iso_8859_6_i },
    { "iso_8859-6:1987", character_set_t::iso_8859_6 },
    { "iso_8859-7", character_set_t::iso_8859_7 },
    { "iso_8859-7:1987", character_set_t::iso_8859_7 },
    { "iso_8859-8", character_set_t::iso_8859_8 },
    { "iso_8859-8-e", character_set_t::iso_8859_8_e },
    { "iso_8859-8-i", character_set_t::iso_8859_8_i },
    { "iso_8859-8:1988", character_set_t::iso_8859_8 },
    { "iso_8859-9", character_set_t::iso_8859_9 },
    { "iso_8859-9:1989", character_set_t::iso_8859_9 },
    { "iso_8859-supp", character_set_t::iso_8859_supp },
    { "iso_9036", character_set_t::asmo_449 },
    { "iso_tr_11548-1", character_set_t::iso_11548_1 },
    { "it", character_set_t::it },
    { "jis_c6220-1969", character_set_t::jis_c6220_1969_jp },
    { "jis_c6220-1969-jp", character_set_t::jis_c6220_1969_jp },
    { "jis_c6220-1969-ro", character_set_t::jis_c6220_1969_ro },
    { "jis_c6226-1978", character_set_t::jis_c6226_1978 },
    { "jis_c6226-1983", character_set_t::jis_c6226_1983 },
    { "jis_c6229-1984-a", character_set_t::jis_c6229_1984_a },
    { "jis_c6229-1984-b", character_set_t::jis_c6229_1984_b },
    { "jis_c6229-1984-b-add", character_set_t::jis_c6229_1984_b_add },
    { "jis_c6229-1984-hand", character_set_t::jis_c6229_1984_hand },
    { "jis_c6229-1984-hand-add", character_set_t::jis_c6229_1984_hand_add },
    { "jis_c6229-1984-kana", character_set_t::jis_c6229_1984_kana },
    { "jis_encoding", character_set_t::jis_encoding },
    { "jis_x0201", character_set_t::jis_x0201 },
    { "jis_x0208-1983", character_set_t::jis_c6226_1983 },
    { "jis_x0212-1990", character_set_t::jis_x0212_1990 },
    { "jp", character_set_t::jis_c6220_1969_ro },
    { "jp-ocr-a", character_set_t::jis_c6229_1984_a },
    { "jp-ocr-b", character_set_t::jis_c6229_1984_b },
    { "jp-ocr-b-add", character_set_t::jis_c6229_1984_b_add },
    { "jp-ocr-hand", character_set_t::jis_c6229_1984_hand },
    { "jp-ocr-hand-add", character_set_t::jis_c6229_1984_hand_add },
    { "js", character_set_t::jus_i_b1_002 },
    { "jus_i.b1.002", character_set_t::jus_i_b1_002 },
    { "jus_i.b1.003-mac", character_set_t::jus_i_b1_003_mac },
    { "jus_i.b1.003-serb", character_set_t::jus_i_b1_003_serb },
    { "katakana", character_set_t::jis_c6220_1969_jp },
    { "koi7-switched", character_set_t::koi7_switched },
    { "koi8-e", character_set_t::ecma_cyrillic },
    { "koi8-r", character_set_t::koi8_r },
    { "koi8-u", character_set_t::koi8_u },
    { "korean", character_set_t::ks_c_5601_1987 },
    { "ks_c_5601-1987", character_set_t::ks_c_5601_1987 },
    { "ks_c_5601-1989", character_set_t::ks_c_5601_1987 },
    { "ksc5636", character_set_t::ksc5636 },
    { "ksc_5601", character_set_t::ks_c_5601_1987 },
    { "kz-1048", character_set_t::kz_1048 },
    { "l1", character_set_t::iso_8859_1 },
    { "l10", character_set_t::iso_8859_16 },
    { "l2", character_set_t::iso_8859_2 },
    { "l3", character_set_t::iso_8859_3 },
    { "l4", character_set_t::iso_8859_4 },
    { "l5", character_set_t::iso_8859_9 },
    { "l6", character_set_t::iso_8859_10 },
    { "l8", character_set_t::iso_8859_14 },
    { "lap", character_set_t::latin_lap },
    { "latin-9", character_set_t::iso_8859_15 },
    { "latin-greek", character_set_t::latin_greek },
    { "latin-greek-1", character_set_t::latin_greek_1 },
    { "latin-lap", character_set_t::latin_lap },
    { "latin1", character_set_t::iso_8859_1 },
    { "latin1-2-5", character_set_t::iso_8859_supp },
    { "latin10", character_set_t::iso_8859_16 },
    { "latin2", character_set_t::iso_8859_2 },
    { "latin3", character_set_t::iso_8859_3 },
    { "latin4", character_set_t::iso_8859_4 },
    { "latin5", character_set_t::iso_8859_9 },
    { "latin6", character_set_t::iso_8859_10 },
    { "latin8", character_set_t::iso_8859_14 },
    { "mac", character_set_t::macintosh },
    { "macedonian", character_set_t::jus_i_b1_003_mac },
    { "macintosh", character_set_t::macintosh },
    { "microsoft-publishing", character_set_t::microsoft_publishing },
    { "mnem", character_set_t::mnem },
    { "mnemonic", character_set_t::mnemonic },
    { "ms936", character_set_t::gbk },
    { "ms_kanji", character_set_t::shift_jis },
    { "msz_7795.3", character_set_t::msz_7795_3 },
    { "naplps", character_set_t::ansi_x3_110_1983 },
    { "nats-dano", character_set_t::nats_dano },
    { "nats-dano-add", character_set_t::nats_dano_add },
    { "nats-sefi", character_set_t::nats_sefi },
    { "nats-sefi-add", character_set_t::nats_sefi_add },
    { "nc_nc00-10:81", character_set_t::nc_nc00_10_81 },
    { "nf_z_62-010", character_set_t::nf_z_62_010 },
    { "nf_z_62-010_(1973)", character_set_t::nf_z_62_010_1973 },
    { "no", character_set_t::ns_4551_1 },
    { "no2", character_set_t::ns_4551_2 },
    { "ns_4551-1", character_set_t::ns_4551_1 },
    { "ns_4551-2", character_set_t::ns_4551_2 },
    { "osd_ebcdic_df03_irv", character_set_t::osd_ebcdic_df03_irv },
    { "osd_ebcdic_df04_1", character_set_t::osd_ebcdic_df04_1 },
    { "osd_ebcdic_df04_15", character_set_t::osd_ebcdic_df04_15 },
    { "pc-multilingual-850+euro", character_set_t::ibm00858 },
    { "pc8-danish-norwegian", character_set_t::pc8_danish_norwegian },
    { "pc8-turkish", character_set_t::pc8_turkish },
    { "pt", character_set_t::pt },
    { "pt154", character_set_t::ptcp154 },
    { "pt2", character_set_t::pt2 },
    { "ptcp154", character_set_t::ptcp154 },
    { "r8", character_set_t::hp_roman8 },
    { "ref", character_set_t::iso_646_basic_1983 },
    { "rk1048", character_set_t::kz_1048 },
    { "roman8", character_set_t::hp_roman8 },
    { "scsu", character_set_t::scsu },
    { "se", character_set_t::sen_850200_b },
    { "se2", character_set_t::sen_850200_c },
    { "sen_850200_b", character_set_t::sen_850200_b },
    { "sen_850200_c", character_set_t::sen_850200_c },
    { "serbian", character_set_t::jus_i_b1_003_serb },
    { "shift_jis", character_set_t::shift_jis },
    { "st_sev_358-88", character_set_t::gost_19768_74 },
    { "strk1048-2002", character_set_t::kz_1048 },
    { "t.101-g2", character_set_t::t_101_g2 },
    { "t.61", character_set_t::t_61_8bit },
    { "t.61-7bit", character_set_t::t_61_7bit },
    { "t.61-8bit", character_set_t::t_61_8bit },
    { "tis-620", character_set_t::tis_620 },
    { "tscii", character_set_t::tscii },
    { "uk", character_set_t::bs_4730 },
    { "unicode-1-1", character_set_t::unicode_1_1 },
    { "unicode-1-1-utf-7", character_set_t::unicode_1_1_utf_7 },
    { "unknown-8bit", character_set_t::unknown_8bit },
    { "us", character_set_t::us_ascii },
    { "us-ascii", character_set_t::us_ascii },
    { "us-dk", character_set_t::us_dk },
    { "utf-16", character_set_t::utf_16 },
    { "utf-16be", character_set_t::utf_16be },
    { "utf-16le", character_set_t::utf_16le },
    { "utf-32", character_set_t::utf_32 },
    { "utf-32be", character_set_t::utf_32be },
    { "utf-32le", character_set_t::utf_32le },
    { "utf-7", character_set_t::utf_7 },
    { "utf-7-imap", character_set_t::utf_7_imap },
    { "utf-8", character_set_t::utf_8 },
    { "ventura-international", character_set_t::ventura_international },
    { "ventura-math", character_set_t::ventura_math },
    { "ventura-us", character_set_t::ventura_us },
    { "videotex-suppl", character_set_t::videotex_suppl },
    { "viqr", character_set_t::viqr },
    { "viscii", character_set_t::viscii },
    { "windows-1250", character_set_t::windows_1250 },
    { "windows-1251", character_set_t::windows_1251 },
    { "windows-1252", character_set_t::windows_1252 },
    { "windows-1253", character_set_t::windows_1253 },
    { "windows-1254", character_set_t::windows_1254 },
    { "windows-1255", character_set_t::windows_1255 },
    { "windows-1256", character_set_t::windows_1256 },
    { "windows-1257", character_set_t::windows_1257 },
    { "windows-1258", character_set_t::windows_1258 },
    { "windows-31j", character_set_t::windows_31j },
    { "windows-874", character_set_t::windows_874 },
    { "windows-936", character_set_t::gbk },
    { "x0201", character_set_t::jis_x0201 },
    { "x0201-7", character_set_t::jis_c6220_1969_jp },
    { "x0208", character_set_t::jis_c6226_1983 },
    { "x0212", character_set_t::jis_x0212_1990 },
    { "yu", character_set_t::jus_i_b1_002 },
};const map_type& get()
{
    static map_type mt(entries, std::size(entries), character_set_t::unspecified);
    return mt;
}

} // namespace charset

} // anonymous namespace

dump_format_t to_dump_format_enum(std::string_view s)
{
    return dump_format::get().find(s);
}

character_set_t to_character_set(std::string_view s)
{
    // Convert the source encoding string to all lower-case first.
    std::string val_lower{s};
    std::transform(val_lower.begin(), val_lower.end(), val_lower.begin(),
        [](unsigned char c)
        {
            return std::tolower(c);
        }
    );

    return charset::get().find(val_lower);
}

std::vector<std::pair<std::string_view, dump_format_t>> get_dump_format_entries()
{
    std::vector<std::pair<std::string_view, dump_format_t>> ret;
    for (const auto& e : dump_format::entries)
        ret.emplace_back(e.key, e.value);

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
