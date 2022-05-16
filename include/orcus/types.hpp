/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_TYPES_HPP
#define INCLUDED_ORCUS_TYPES_HPP

#include <cstdlib>
#include <vector>
#include <string>
#include <unordered_set>
#include "env.hpp"

namespace orcus {

class xmlns_context;
class xmlns_repository;

/**
 * Generic constant to be used to indicate that a valid index value is
 * expected but not found.
 */
ORCUS_PSR_DLLPUBLIC extern const size_t index_not_found;

// XML specific types

using xml_token_t = size_t;
using xmlns_id_t = const char*;

using xml_token_pair_t = std::pair<xmlns_id_t, xml_token_t>;

struct ORCUS_PSR_DLLPUBLIC xml_token_pair_hash
{
    size_t operator()(const xml_token_pair_t& v) const;
};

using xml_elem_stack_t = std::vector<xml_token_pair_t>;
using xml_elem_set_t = std::unordered_set<xml_token_pair_t, xml_token_pair_hash>;

ORCUS_PSR_DLLPUBLIC extern const xmlns_id_t XMLNS_UNKNOWN_ID;
ORCUS_PSR_DLLPUBLIC extern const xml_token_t XML_UNKNOWN_TOKEN;

struct ORCUS_PSR_DLLPUBLIC parse_error_value_t
{
    std::string_view str;
    std::ptrdiff_t offset;

    parse_error_value_t();
    parse_error_value_t(std::string_view _str, std::ptrdiff_t _offset);

    bool operator==(const parse_error_value_t& other) const;
    bool operator!=(const parse_error_value_t& other) const;
};

struct ORCUS_PSR_DLLPUBLIC xml_name_t
{
    enum to_string_type { use_alias, use_short_name };

    xmlns_id_t ns;
    std::string_view name;

    xml_name_t();
    xml_name_t(xmlns_id_t _ns, std::string_view _name);
    xml_name_t(const xml_name_t& r);

    xml_name_t& operator= (const xml_name_t& other);

    bool operator== (const xml_name_t& other) const;
    bool operator!= (const xml_name_t& other) const;

    std::string to_string(const xmlns_context& cxt, to_string_type type) const;

    std::string to_string(const xmlns_repository& repo) const;
};

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
    xml_token_attr_t(
        xmlns_id_t _ns, xml_token_t _name, std::string_view _value, bool _transient);
    xml_token_attr_t(
        xmlns_id_t _ns, xml_token_t _name, std::string_view _raw_name,
        std::string_view _value, bool _transient);
};

/**
 * Element properties passed to its handler via start_element() and
 * end_element() calls.
 */
struct ORCUS_PSR_DLLPUBLIC xml_token_element_t
{
    xmlns_id_t ns;
    xml_token_t name;
    std::string_view raw_name;
    std::vector<xml_token_attr_t> attrs;

    xml_token_element_t& operator= (xml_token_element_t) = delete;

    xml_token_element_t();
    xml_token_element_t(xmlns_id_t _ns, xml_token_t _name, std::string_view _raw_name, std::vector<xml_token_attr_t>&& _attrs);
    xml_token_element_t(const xml_token_element_t& other);
    xml_token_element_t(xml_token_element_t&& other);
};

/**
 * Character set types.
 *
 * @see https://www.iana.org/assignments/character-sets/character-sets.xhtml
 */
enum class character_set_t
{
    unspecified = 0,
    us_ascii,                                      // US-ASCII
    iso_8859_1_1987,                               // ISO_8859-1:1987
    iso_8859_2_1987,                               // ISO_8859-2:1987
    iso_8859_3_1988,                               // ISO_8859-3:1988
    iso_8859_4_1988,                               // ISO_8859-4:1988
    iso_8859_5_1988,                               // ISO_8859-5:1988
    iso_8859_6_1987,                               // ISO_8859-6:1987
    iso_8859_7_1987,                               // ISO_8859-7:1987
    iso_8859_8_1988,                               // ISO_8859-8:1988
    iso_8859_9_1989,                               // ISO_8859-9:1989
    iso_8859_10,                                   // ISO-8859-10
    iso_6937_2_add,                                // ISO_6937-2-add
    jis_x0201,                                     // JIS_X0201
    jis_encoding,                                  // JIS_Encoding
    shift_jis,                                     // Shift_JIS
    extended_unix_code_packed_format_for_japanese, // Extended_UNIX_Code_Packed_Format_for_Japanese
    extended_unix_code_fixed_width_for_japanese,   // Extended_UNIX_Code_Fixed_Width_for_Japanese
    bs_4730,                                       // BS_4730
    sen_850200_c,                                  // SEN_850200_C
    it,                                            // IT
    es,                                            // ES
    din_66003,                                     // DIN_66003
    ns_4551_1,                                     // NS_4551-1
    nf_z_62_010,                                   // NF_Z_62-010
    iso_10646_utf_1,                               // ISO-10646-UTF-1
    iso_646_basic_1983,                            // ISO_646.basic:1983
    invariant,                                     // INVARIANT
    iso_646_irv_1983,                              // ISO_646.irv:1983
    nats_sefi,                                     // NATS-SEFI
    nats_sefi_add,                                 // NATS-SEFI-ADD
    nats_dano,                                     // NATS-DANO
    nats_dano_add,                                 // NATS-DANO-ADD
    sen_850200_b,                                  // SEN_850200_B
    ks_c_5601_1987,                                // KS_C_5601-1987
    iso_2022_kr,                                   // ISO-2022-KR
    euc_kr,                                        // EUC-KR
    iso_2022_jp,                                   // ISO-2022-JP
    iso_2022_jp_2,                                 // ISO-2022-JP-2
    jis_c6220_1969_jp,                             // JIS_C6220-1969-jp
    jis_c6220_1969_ro,                             // JIS_C6220-1969-ro
    pt,                                            // PT
    greek7_old,                                    // greek7-old
    latin_greek,                                   // latin-greek
    nf_z_62_010_1973,                              // NF_Z_62-010_(1973)
    latin_greek_1,                                 // Latin-greek-1
    iso_5427,                                      // ISO_5427
    jis_c6226_1978,                                // JIS_C6226-1978
    bs_viewdata,                                   // BS_viewdata
    inis,                                          // INIS
    inis_8,                                        // INIS-8
    inis_cyrillic,                                 // INIS-cyrillic
    iso_5427_1981,                                 // ISO_5427:1981
    iso_5428_1980,                                 // ISO_5428:1980
    gb_1988_80,                                    // GB_1988-80
    gb_2312_80,                                    // GB_2312-80
    ns_4551_2,                                     // NS_4551-2
    videotex_suppl,                                // videotex-suppl
    pt2,                                           // PT2
    es2,                                           // ES2
    msz_7795_3,                                    // MSZ_7795.3
    jis_c6226_1983,                                // JIS_C6226-1983
    greek7,                                        // greek7
    asmo_449,                                      // ASMO_449
    iso_ir_90,                                     // iso-ir-90
    jis_c6229_1984_a,                              // JIS_C6229-1984-a
    jis_c6229_1984_b,                              // JIS_C6229-1984-b
    jis_c6229_1984_b_add,                          // JIS_C6229-1984-b-add
    jis_c6229_1984_hand,                           // JIS_C6229-1984-hand
    jis_c6229_1984_hand_add,                       // JIS_C6229-1984-hand-add
    jis_c6229_1984_kana,                           // JIS_C6229-1984-kana
    iso_2033_1983,                                 // ISO_2033-1983
    ansi_x3_110_1983,                              // ANSI_X3.110-1983
    t_61_7bit,                                     // T.61-7bit
    t_61_8bit,                                     // T.61-8bit
    ecma_cyrillic,                                 // ECMA-cyrillic
    csa_z243_4_1985_1,                             // CSA_Z243.4-1985-1
    csa_z243_4_1985_2,                             // CSA_Z243.4-1985-2
    csa_z243_4_1985_gr,                            // CSA_Z243.4-1985-gr
    iso_8859_6_e,                                  // ISO_8859-6-E
    iso_8859_6_i,                                  // ISO_8859-6-I
    t_101_g2,                                      // T.101-G2
    iso_8859_8_e,                                  // ISO_8859-8-E
    iso_8859_8_i,                                  // ISO_8859-8-I
    csn_369103,                                    // CSN_369103
    jus_i_b1_002,                                  // JUS_I.B1.002
    iec_p27_1,                                     // IEC_P27-1
    jus_i_b1_003_serb,                             // JUS_I.B1.003-serb
    jus_i_b1_003_mac,                              // JUS_I.B1.003-mac
    greek_ccitt,                                   // greek-ccitt
    nc_nc00_10_81,                                 // NC_NC00-10:81
    iso_6937_2_25,                                 // ISO_6937-2-25
    gost_19768_74,                                 // GOST_19768-74
    iso_8859_supp,                                 // ISO_8859-supp
    iso_10367_box,                                 // ISO_10367-box
    latin_lap,                                     // latin-lap
    jis_x0212_1990,                                // JIS_X0212-1990
    ds_2089,                                       // DS_2089
    us_dk,                                         // us-dk
    dk_us,                                         // dk-us
    ksc5636,                                       // KSC5636
    unicode_1_1_utf_7,                             // UNICODE-1-1-UTF-7
    iso_2022_cn,                                   // ISO-2022-CN
    iso_2022_cn_ext,                               // ISO-2022-CN-EXT
    utf_8,                                         // UTF-8
    iso_8859_13,                                   // ISO-8859-13
    iso_8859_14,                                   // ISO-8859-14
    iso_8859_15,                                   // ISO-8859-15
    iso_8859_16,                                   // ISO-8859-16
    gbk,                                           // GBK
    gb18030,                                       // GB18030
    osd_ebcdic_df04_15,                            // OSD_EBCDIC_DF04_15
    osd_ebcdic_df03_irv,                           // OSD_EBCDIC_DF03_IRV
    osd_ebcdic_df04_1,                             // OSD_EBCDIC_DF04_1
    iso_11548_1,                                   // ISO-11548-1
    kz_1048,                                       // KZ-1048
    iso_10646_ucs_2,                               // ISO-10646-UCS-2
    iso_10646_ucs_4,                               // ISO-10646-UCS-4
    iso_10646_ucs_basic,                           // ISO-10646-UCS-Basic
    iso_10646_unicode_latin1,                      // ISO-10646-Unicode-Latin1
    iso_10646_j_1,                                 // ISO-10646-J-1
    iso_unicode_ibm_1261,                          // ISO-Unicode-IBM-1261
    iso_unicode_ibm_1268,                          // ISO-Unicode-IBM-1268
    iso_unicode_ibm_1276,                          // ISO-Unicode-IBM-1276
    iso_unicode_ibm_1264,                          // ISO-Unicode-IBM-1264
    iso_unicode_ibm_1265,                          // ISO-Unicode-IBM-1265
    unicode_1_1,                                   // UNICODE-1-1
    scsu,                                          // SCSU
    utf_7,                                         // UTF-7
    utf_16be,                                      // UTF-16BE
    utf_16le,                                      // UTF-16LE
    utf_16,                                        // UTF-16
    cesu_8,                                        // CESU-8
    utf_32,                                        // UTF-32
    utf_32be,                                      // UTF-32BE
    utf_32le,                                      // UTF-32LE
    bocu_1,                                        // BOCU-1
    iso_8859_1_windows_3_0_latin_1,                // ISO-8859-1-Windows-3.0-Latin-1
    iso_8859_1_windows_3_1_latin_1,                // ISO-8859-1-Windows-3.1-Latin-1
    iso_8859_2_windows_latin_2,                    // ISO-8859-2-Windows-Latin-2
    iso_8859_9_windows_latin_5,                    // ISO-8859-9-Windows-Latin-5
    hp_roman8,                                     // hp-roman8
    adobe_standard_encoding,                       // Adobe-Standard-Encoding
    ventura_us,                                    // Ventura-US
    ventura_international,                         // Ventura-International
    dec_mcs,                                       // DEC-MCS
    ibm850,                                        // IBM850
    pc8_danish_norwegian,                          // PC8-Danish-Norwegian
    ibm862,                                        // IBM862
    pc8_turkish,                                   // PC8-Turkish
    ibm_symbols,                                   // IBM-Symbols
    ibm_thai,                                      // IBM-Thai
    hp_legal,                                      // HP-Legal
    hp_pi_font,                                    // HP-Pi-font
    hp_math8,                                      // HP-Math8
    adobe_symbol_encoding,                         // Adobe-Symbol-Encoding
    hp_desktop,                                    // HP-DeskTop
    ventura_math,                                  // Ventura-Math
    microsoft_publishing,                          // Microsoft-Publishing
    windows_31j,                                   // Windows-31J
    gb2312,                                        // GB2312
    big5,                                          // Big5
    macintosh,                                     // macintosh
    ibm037,                                        // IBM037
    ibm038,                                        // IBM038
    ibm273,                                        // IBM273
    ibm274,                                        // IBM274
    ibm275,                                        // IBM275
    ibm277,                                        // IBM277
    ibm278,                                        // IBM278
    ibm280,                                        // IBM280
    ibm281,                                        // IBM281
    ibm284,                                        // IBM284
    ibm285,                                        // IBM285
    ibm290,                                        // IBM290
    ibm297,                                        // IBM297
    ibm420,                                        // IBM420
    ibm423,                                        // IBM423
    ibm424,                                        // IBM424
    ibm437,                                        // IBM437
    ibm500,                                        // IBM500
    ibm851,                                        // IBM851
    ibm852,                                        // IBM852
    ibm855,                                        // IBM855
    ibm857,                                        // IBM857
    ibm860,                                        // IBM860
    ibm861,                                        // IBM861
    ibm863,                                        // IBM863
    ibm864,                                        // IBM864
    ibm865,                                        // IBM865
    ibm868,                                        // IBM868
    ibm869,                                        // IBM869
    ibm870,                                        // IBM870
    ibm871,                                        // IBM871
    ibm880,                                        // IBM880
    ibm891,                                        // IBM891
    ibm903,                                        // IBM903
    ibm904,                                        // IBM904
    ibm905,                                        // IBM905
    ibm918,                                        // IBM918
    ibm1026,                                       // IBM1026
    ebcdic_at_de,                                  // EBCDIC-AT-DE
    ebcdic_at_de_a,                                // EBCDIC-AT-DE-A
    ebcdic_ca_fr,                                  // EBCDIC-CA-FR
    ebcdic_dk_no,                                  // EBCDIC-DK-NO
    ebcdic_dk_no_a,                                // EBCDIC-DK-NO-A
    ebcdic_fi_se,                                  // EBCDIC-FI-SE
    ebcdic_fi_se_a,                                // EBCDIC-FI-SE-A
    ebcdic_fr,                                     // EBCDIC-FR
    ebcdic_it,                                     // EBCDIC-IT
    ebcdic_pt,                                     // EBCDIC-PT
    ebcdic_es,                                     // EBCDIC-ES
    ebcdic_es_a,                                   // EBCDIC-ES-A
    ebcdic_es_s,                                   // EBCDIC-ES-S
    ebcdic_uk,                                     // EBCDIC-UK
    ebcdic_us,                                     // EBCDIC-US
    unknown_8bit,                                  // UNKNOWN-8BIT
    mnemonic,                                      // MNEMONIC
    mnem,                                          // MNEM
    viscii,                                        // VISCII
    viqr,                                          // VIQR
    koi8_r,                                        // KOI8-R
    hz_gb_2312,                                    // HZ-GB-2312
    ibm866,                                        // IBM866
    ibm775,                                        // IBM775
    koi8_u,                                        // KOI8-U
    ibm00858,                                      // IBM00858
    ibm00924,                                      // IBM00924
    ibm01140,                                      // IBM01140
    ibm01141,                                      // IBM01141
    ibm01142,                                      // IBM01142
    ibm01143,                                      // IBM01143
    ibm01144,                                      // IBM01144
    ibm01145,                                      // IBM01145
    ibm01146,                                      // IBM01146
    ibm01147,                                      // IBM01147
    ibm01148,                                      // IBM01148
    ibm01149,                                      // IBM01149
    big5_hkscs,                                    // Big5-HKSCS
    ibm1047,                                       // IBM1047
    ptcp154,                                       // PTCP154
    amiga_1251,                                    // Amiga-1251
    koi7_switched,                                 // KOI7-switched
    brf,                                           // BRF
    tscii,                                         // TSCII
    cp51932,                                       // CP51932
    windows_874,                                   // windows-874
    windows_1250,                                  // windows-1250
    windows_1251,                                  // windows-1251
    windows_1252,                                  // windows-1252
    windows_1253,                                  // windows-1253
    windows_1254,                                  // windows-1254
    windows_1255,                                  // windows-1255
    windows_1256,                                  // windows-1256
    windows_1257,                                  // windows-1257
    windows_1258,                                  // windows-1258
    tis_620,                                       // TIS-620
    cp50220,                                       // CP50220
};

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

// Other types

enum class length_unit_t
{
    unknown = 0,
    centimeter,
    millimeter,
    xlsx_column_digit,
    inch,
    point,
    twip,
    pixel

    // TODO: Add more.
};

enum class format_t
{
    unknown = 0,
    ods,
    xlsx,
    gnumeric,
    xls_xml,
    csv
};

enum class dump_format_t
{
    unknown,
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

struct ORCUS_PSR_DLLPUBLIC length_t
{
    length_unit_t unit;
    double value;

    length_t();

    std::string to_string() const;

    bool operator== (const length_t& other) const noexcept;
    bool operator!= (const length_t& other) const noexcept;
};

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

    std::string to_string() const;

    void swap(date_time_t& other);
};

ORCUS_PSR_DLLPUBLIC dump_format_t to_dump_format_enum(std::string_view s);

ORCUS_PSR_DLLPUBLIC std::vector<std::pair<std::string_view, dump_format_t>> get_dump_format_entries();

ORCUS_PSR_DLLPUBLIC std::ostream& operator<< (std::ostream& os, const length_t& v);
ORCUS_PSR_DLLPUBLIC std::ostream& operator<< (std::ostream& os, const date_time_t& v);
ORCUS_PSR_DLLPUBLIC std::ostream& operator<< (std::ostream& os, format_t v);

typedef ::std::vector<xml_token_attr_t> xml_attrs_t;

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
