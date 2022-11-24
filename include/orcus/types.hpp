/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_TYPES_HPP
#define INCLUDED_ORCUS_TYPES_HPP

#include <cstdint>
#include <cstdlib>
#include <vector>
#include <string>
#include <unordered_set>
#include "env.hpp"

namespace orcus {

class xmlns_context;
class xmlns_repository;

/**
 * Integral type that represents a tokenized XML element name.
 */
using xml_token_t = std::size_t;

/**
 * Type that represents a normalized XML namespace identifier.  Internally it
 * is a pointer value that points to a static char buffer that stores a
 * namespace name.
 */
using xmlns_id_t = const char*;

/**
 * Parser token that represents the state of a parse error, used by
 * threaded_json_parser and threaded_sax_token_parser when transferring
 * parse status between threads.
 */
struct ORCUS_PSR_DLLPUBLIC parse_error_value_t
{
    /** error message associated with the parse error. */
    std::string_view str;
    /** offset in stream where the error occurred. */
    std::ptrdiff_t offset;

    parse_error_value_t();
    parse_error_value_t(const parse_error_value_t& other);
    parse_error_value_t(std::string_view _str, std::ptrdiff_t _offset);

    parse_error_value_t& operator=(const parse_error_value_t& other);

    bool operator==(const parse_error_value_t& other) const;
    bool operator!=(const parse_error_value_t& other) const;
};

/**
 * Represents a name with a normalized namespace in XML documents.  This can
 * be used either as an element name or as an attribute name.
 */
struct ORCUS_PSR_DLLPUBLIC xml_name_t
{
    enum to_string_type { use_alias, use_short_name };

    xmlns_id_t ns;
    std::string_view name;

    xml_name_t() noexcept;
    xml_name_t(xmlns_id_t _ns, std::string_view _name);
    xml_name_t(const xml_name_t& other);

    xml_name_t& operator= (const xml_name_t& other);

    bool operator== (const xml_name_t& other) const noexcept;
    bool operator!= (const xml_name_t& other) const noexcept;

    /**
     * Convert a namespace-name value pair to a string representation with the
     * namespace value converted to either an alias or a unique "short name".
     * Refer to @link xmlns_context::get_alias() get_alias() @endlink and
     * @link xmlns_context::get_short_name() get_short_name() @endlink
     * for the explanations of an alias and short name.
     *
     * @param cxt namespace context object associated with the XML stream
     *            currently being parsed.
     * @param type policy on how to convert a namespace identifier to a string
     *             representation.
     *
     * @return string representation of a namespace-name value pair.
     */
    std::string to_string(const xmlns_context& cxt, to_string_type type) const;

    /**
     * Convert a namespace-name value pair to a string representation with the
     * namespace value converted to a unique "short name". Refer to @link
     * xmlns_repository::get_short_name() get_short_name() @endlink for the
     * explanations of a short name.
     *
     * @param repo namespace repository.
     *
     * @return string representation of a namespace-name value pair.
     */
    std::string to_string(const xmlns_repository& repo) const;
};

/**
 * Struct containing properties of a tokenized XML attribute.
 */
struct ORCUS_PSR_DLLPUBLIC xml_token_attr_t
{
    xmlns_id_t ns;
    xml_token_t name;
    std::string_view raw_name;
    std::string_view value;

    /**
     * Whether or not the attribute value is transient. A transient value is
     * only guaranteed to be valid until the end of the start_element call,
     * after which its validity is not guaranteed. A non-transient value is
     * guaranteed to be valid during the life cycle of the xml stream it
     * belongs to.
     */
    bool transient;

    xml_token_attr_t();
    xml_token_attr_t(const xml_token_attr_t& other);
    xml_token_attr_t(
        xmlns_id_t _ns, xml_token_t _name, std::string_view _value, bool _transient);
    xml_token_attr_t(
        xmlns_id_t _ns, xml_token_t _name, std::string_view _raw_name,
        std::string_view _value, bool _transient);

    xml_token_attr_t& operator=(const xml_token_attr_t& other);
};

using xml_token_attrs_t = std::vector<xml_token_attr_t>;

/**
 * Struct containing XML element properties passed to the handler of
 * sax_token_parser via its @p start_element() and @p end_element()
 * calls.
 *
 * @see
 *      @li sax_token_handler::start_element
 *      @li sax_token_handler::end_element
 */
struct ORCUS_PSR_DLLPUBLIC xml_token_element_t
{
    xmlns_id_t ns;
    xml_token_t name;
    std::string_view raw_name;
    xml_token_attrs_t attrs;

    xml_token_element_t& operator= (xml_token_element_t) = delete;

    xml_token_element_t();
    xml_token_element_t(xmlns_id_t _ns, xml_token_t _name, std::string_view _raw_name, std::vector<xml_token_attr_t>&& _attrs);
    xml_token_element_t(const xml_token_element_t& other);
    xml_token_element_t(xml_token_element_t&& other);
};

/**
 * Character set types, generated from IANA character-sets specifications.
 *
 * @see https://www.iana.org/assignments/character-sets/character-sets.xhtml
 */
enum class character_set_t
{
    unspecified = 0,
    adobe_standard_encoding,
    adobe_symbol_encoding,
    amiga_1251,
    ansi_x3_110_1983,
    asmo_449,
    big5,
    big5_hkscs,
    bocu_1,
    brf,
    bs_4730,
    bs_viewdata,
    cesu_8,
    cp50220,
    cp51932,
    csa_z243_4_1985_1,
    csa_z243_4_1985_2,
    csa_z243_4_1985_gr,
    csn_369103,
    dec_mcs,
    din_66003,
    dk_us,
    ds_2089,
    ebcdic_at_de,
    ebcdic_at_de_a,
    ebcdic_ca_fr,
    ebcdic_dk_no,
    ebcdic_dk_no_a,
    ebcdic_es,
    ebcdic_es_a,
    ebcdic_es_s,
    ebcdic_fi_se,
    ebcdic_fi_se_a,
    ebcdic_fr,
    ebcdic_it,
    ebcdic_pt,
    ebcdic_uk,
    ebcdic_us,
    ecma_cyrillic,
    es,
    es2,
    euc_jp,
    euc_kr,
    extended_unix_code_fixed_width_for_japanese,
    gb18030,
    gb2312,
    gb_1988_80,
    gb_2312_80,
    gbk,
    gost_19768_74,
    greek7,
    greek7_old,
    greek_ccitt,
    hp_desktop,
    hp_legal,
    hp_math8,
    hp_pi_font,
    hp_roman8,
    hz_gb_2312,
    ibm00858,
    ibm00924,
    ibm01140,
    ibm01141,
    ibm01142,
    ibm01143,
    ibm01144,
    ibm01145,
    ibm01146,
    ibm01147,
    ibm01148,
    ibm01149,
    ibm037,
    ibm038,
    ibm1026,
    ibm1047,
    ibm273,
    ibm274,
    ibm275,
    ibm277,
    ibm278,
    ibm280,
    ibm281,
    ibm284,
    ibm285,
    ibm290,
    ibm297,
    ibm420,
    ibm423,
    ibm424,
    ibm437,
    ibm500,
    ibm775,
    ibm850,
    ibm851,
    ibm852,
    ibm855,
    ibm857,
    ibm860,
    ibm861,
    ibm862,
    ibm863,
    ibm864,
    ibm865,
    ibm866,
    ibm868,
    ibm869,
    ibm870,
    ibm871,
    ibm880,
    ibm891,
    ibm903,
    ibm904,
    ibm905,
    ibm918,
    ibm_symbols,
    ibm_thai,
    iec_p27_1,
    inis,
    inis_8,
    inis_cyrillic,
    invariant,
    iso_10367_box,
    iso_10646_j_1,
    iso_10646_ucs_2,
    iso_10646_ucs_4,
    iso_10646_ucs_basic,
    iso_10646_unicode_latin1,
    iso_10646_utf_1,
    iso_11548_1,
    iso_2022_cn,
    iso_2022_cn_ext,
    iso_2022_jp,
    iso_2022_jp_2,
    iso_2022_kr,
    iso_2033_1983,
    iso_5427,
    iso_5427_1981,
    iso_5428_1980,
    iso_646_basic_1983,
    iso_646_irv_1983,
    iso_6937_2_25,
    iso_6937_2_add,
    iso_8859_1,
    iso_8859_10,
    iso_8859_13,
    iso_8859_14,
    iso_8859_15,
    iso_8859_16,
    iso_8859_1_windows_3_0_latin_1,
    iso_8859_1_windows_3_1_latin_1,
    iso_8859_2,
    iso_8859_2_windows_latin_2,
    iso_8859_3,
    iso_8859_4,
    iso_8859_5,
    iso_8859_6,
    iso_8859_6_e,
    iso_8859_6_i,
    iso_8859_7,
    iso_8859_8,
    iso_8859_8_e,
    iso_8859_8_i,
    iso_8859_9,
    iso_8859_9_windows_latin_5,
    iso_8859_supp,
    iso_ir_90,
    iso_unicode_ibm_1261,
    iso_unicode_ibm_1264,
    iso_unicode_ibm_1265,
    iso_unicode_ibm_1268,
    iso_unicode_ibm_1276,
    it,
    jis_c6220_1969_jp,
    jis_c6220_1969_ro,
    jis_c6226_1978,
    jis_c6226_1983,
    jis_c6229_1984_a,
    jis_c6229_1984_b,
    jis_c6229_1984_b_add,
    jis_c6229_1984_hand,
    jis_c6229_1984_hand_add,
    jis_c6229_1984_kana,
    jis_encoding,
    jis_x0201,
    jis_x0212_1990,
    jus_i_b1_002,
    jus_i_b1_003_mac,
    jus_i_b1_003_serb,
    koi7_switched,
    koi8_r,
    koi8_u,
    ks_c_5601_1987,
    ksc5636,
    kz_1048,
    latin_greek,
    latin_greek_1,
    latin_lap,
    macintosh,
    microsoft_publishing,
    mnem,
    mnemonic,
    msz_7795_3,
    nats_dano,
    nats_dano_add,
    nats_sefi,
    nats_sefi_add,
    nc_nc00_10_81,
    nf_z_62_010,
    nf_z_62_010_1973,
    ns_4551_1,
    ns_4551_2,
    osd_ebcdic_df03_irv,
    osd_ebcdic_df04_1,
    osd_ebcdic_df04_15,
    pc8_danish_norwegian,
    pc8_turkish,
    pt,
    pt2,
    ptcp154,
    scsu,
    sen_850200_b,
    sen_850200_c,
    shift_jis,
    t_101_g2,
    t_61_7bit,
    t_61_8bit,
    tis_620,
    tscii,
    unicode_1_1,
    unicode_1_1_utf_7,
    unknown_8bit,
    us_ascii,
    us_dk,
    utf_16,
    utf_16be,
    utf_16le,
    utf_32,
    utf_32be,
    utf_32le,
    utf_7,
    utf_7_imap,
    utf_8,
    ventura_international,
    ventura_math,
    ventura_us,
    videotex_suppl,
    viqr,
    viscii,
    windows_1250,
    windows_1251,
    windows_1252,
    windows_1253,
    windows_1254,
    windows_1255,
    windows_1256,
    windows_1257,
    windows_1258,
    windows_31j,
    windows_874,
};

/**
 * Struct holding XML declaration properties.
 */
struct ORCUS_PSR_DLLPUBLIC xml_declaration_t
{
    uint8_t version_major;
    uint8_t version_minor;
    character_set_t encoding;
    bool standalone;

    xml_declaration_t();
    xml_declaration_t(uint8_t _version_major, uint8_t _version_minor, character_set_t _encoding, bool _standalone);
    xml_declaration_t(const xml_declaration_t& other);
    ~xml_declaration_t();

    xml_declaration_t& operator= (const xml_declaration_t& other);

    bool operator== (const xml_declaration_t& other) const;
    bool operator!= (const xml_declaration_t& other) const;
};

/**
 * Unit of length, as used in length_t.
 */
enum class length_unit_t
{
    unknown = 0,
    centimeter,
    millimeter,
    /**
     * Special unit of length used by Excel, defined as the maximum digit width
     * of font used as the "Normal" style font.
     *
     * @note Since it's not possible to determine the actual length using this
     * unit, it is approximated by 1.9 millimeters.
     */
    xlsx_column_digit,
    inch,
    point,
    /** One twip is a twentieth of a point equal to 1/1440 of an inch. */
    twip,
    pixel
};

/**
 * Input formats that orcus can import.
 */
enum class format_t
{
    unknown = 0,
    ods,
    xlsx,
    gnumeric,
    xls_xml,
    csv
};

/**
 * Formats supported by orcus as output formats.
 */
enum class dump_format_t
{
    unknown = 0,
    none,
    check,
    csv,
    flat,
    html,
    json,
    xml,
    yaml,
    debug_state
};

/**
 * Holds a length value with unit of measurement.
 */
struct ORCUS_PSR_DLLPUBLIC length_t
{
    length_unit_t unit;
    double value;

    length_t();
    length_t(length_unit_t _unit, double _value);
    length_t(const length_t& other);
    length_t& operator= (const length_t& other);

    std::string to_string() const;

    bool operator== (const length_t& other) const noexcept;
    bool operator!= (const length_t& other) const noexcept;
};

/**
 * Struct that holds a date or date-time value.
 */
struct ORCUS_PSR_DLLPUBLIC date_time_t
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    double second;

    date_time_t();
    date_time_t(int _year, int _month, int _day);
    date_time_t(int _year, int _month, int _day, int _hour, int _minute, double _second);
    date_time_t(const date_time_t& other);
    ~date_time_t();

    date_time_t& operator= (date_time_t other);

    bool operator== (const date_time_t& other) const;
    bool operator!= (const date_time_t& other) const;
    bool operator< (const date_time_t& other) const;

    /**
     * Convert the date-time value to an ISO-formatted string representation.
     *
     * @return ISO-formatted string representation of the date-time value.
     */
    std::string to_string() const;

    /**
     * Swap the value with another instance.
     *
     * @param other another instance to swap values with.
     */
    void swap(date_time_t& other);

    /**
     * Parse an ISO-formatted string representation of a date-time value, and
     * convert it into a date_time_t value.  A string representation allows
     * either a date only or a date and time value, but it does not allow a time
     * only value.
     *
     * Here are some examples of ISO-formatted date and date-time values:
     *
     * @li <b>2013-04-09</b> (date only)
     * @li <b>2013-04-09T21:34:09.55</b> (date and time)
     *
     * @param str string representation of a date-time value.
     * @return converted date-time value consisting of a set of numeric values.
     */
    static date_time_t from_chars(std::string_view str);
};

/**
 * Parse a string that represents an output format type and convert it to a
 * corresponding enum value.
 *
 * @param s string representing an output format type.
 *
 * @return enum value representing a character set, or
 *         character_set_t::unknown in case it cannot be
 *         determined.
 */
ORCUS_PSR_DLLPUBLIC dump_format_t to_dump_format_enum(std::string_view s);

/**
 * Parse a string that represents a character set and convert it to a
 * corresponding enum value.
 *
 * @param s string representing a character set.
 *
 * @return enum value representing a character set, or
 *         character_set_t::unspecified in case it cannot be
 *         determined.
 */
ORCUS_PSR_DLLPUBLIC character_set_t to_character_set(std::string_view s);

/**
 * Get a list of available output format entries.  Each entry consists of the
 * name of a format and its enum value equivalent.
 *
 * @return list of available output format entries.
 */
ORCUS_PSR_DLLPUBLIC std::vector<std::pair<std::string_view, dump_format_t>> get_dump_format_entries();

ORCUS_PSR_DLLPUBLIC std::ostream& operator<< (std::ostream& os, const length_t& v);
ORCUS_PSR_DLLPUBLIC std::ostream& operator<< (std::ostream& os, const date_time_t& v);
ORCUS_PSR_DLLPUBLIC std::ostream& operator<< (std::ostream& os, format_t v);

/**
 * Generic constant to be used to indicate that a valid index value is
 * expected but not found.
 */
ORCUS_PSR_DLLPUBLIC extern const std::size_t INDEX_NOT_FOUND;

/**
 * Value associated with an unknown XML namespace.
 */
ORCUS_PSR_DLLPUBLIC extern const xmlns_id_t XMLNS_UNKNOWN_ID;

/**
 * Value associated with an unknown XML token.
 */
ORCUS_PSR_DLLPUBLIC extern const xml_token_t XML_UNKNOWN_TOKEN;

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
