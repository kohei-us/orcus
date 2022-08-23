/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/sax_token_parser.hpp"
#include "orcus/tokens.hpp"

#include <mdds/sorted_string_map.hpp>
#include <cctype>

namespace orcus {

namespace {

enum class decl_attr_type { unknown, version, encoding, standalone };

namespace decl_attr {

using map_type = mdds::sorted_string_map<decl_attr_type, mdds::string_view_map_entry>;

// Keys must be sorted.
constexpr map_type::entry entries[] = {
    { "encoding",   decl_attr_type::encoding   },
    { "standalone", decl_attr_type::standalone },
    { "version",    decl_attr_type::version    },
};

const map_type& get()
{
    static map_type mt(entries, std::size(entries), decl_attr_type::unknown);
    return mt;
}

} // namespace decl_attr

namespace encoding {

using map_type = mdds::sorted_string_map<character_set_t, mdds::string_view_map_entry>;

// Keys must be sorted.
constexpr map_type::entry entries[] = {
    { "adobe-standard-encoding", character_set_t::adobe_standard_encoding },
    { "adobe-symbol-encoding", character_set_t::adobe_symbol_encoding },
    { "amiga-1251", character_set_t::amiga_1251 },
    { "ansi_x3.110-1983", character_set_t::ansi_x3_110_1983 },
    { "asmo_449", character_set_t::asmo_449 },
    { "big5", character_set_t::big5 },
    { "big5-hkscs", character_set_t::big5_hkscs },
    { "bocu-1", character_set_t::bocu_1 },
    { "brf", character_set_t::brf },
    { "bs_4730", character_set_t::bs_4730 },
    { "bs_viewdata", character_set_t::bs_viewdata },
    { "cesu-8", character_set_t::cesu_8 },
    { "cp50220", character_set_t::cp50220 },
    { "cp51932", character_set_t::cp51932 },
    { "csa_z243.4-1985-1", character_set_t::csa_z243_4_1985_1 },
    { "csa_z243.4-1985-2", character_set_t::csa_z243_4_1985_2 },
    { "csa_z243.4-1985-gr", character_set_t::csa_z243_4_1985_gr },
    { "csn_369103", character_set_t::csn_369103 },
    { "dec-mcs", character_set_t::dec_mcs },
    { "din_66003", character_set_t::din_66003 },
    { "dk-us", character_set_t::dk_us },
    { "ds_2089", character_set_t::ds_2089 },
    { "ebcdic-at-de", character_set_t::ebcdic_at_de },
    { "ebcdic-at-de-a", character_set_t::ebcdic_at_de_a },
    { "ebcdic-ca-fr", character_set_t::ebcdic_ca_fr },
    { "ebcdic-dk-no", character_set_t::ebcdic_dk_no },
    { "ebcdic-dk-no-a", character_set_t::ebcdic_dk_no_a },
    { "ebcdic-es", character_set_t::ebcdic_es },
    { "ebcdic-es-a", character_set_t::ebcdic_es_a },
    { "ebcdic-es-s", character_set_t::ebcdic_es_s },
    { "ebcdic-fi-se", character_set_t::ebcdic_fi_se },
    { "ebcdic-fi-se-a", character_set_t::ebcdic_fi_se_a },
    { "ebcdic-fr", character_set_t::ebcdic_fr },
    { "ebcdic-it", character_set_t::ebcdic_it },
    { "ebcdic-pt", character_set_t::ebcdic_pt },
    { "ebcdic-uk", character_set_t::ebcdic_uk },
    { "ebcdic-us", character_set_t::ebcdic_us },
    { "ecma-cyrillic", character_set_t::ecma_cyrillic },
    { "es", character_set_t::es },
    { "es2", character_set_t::es2 },
    { "euc-kr", character_set_t::euc_kr },
    { "extended_unix_code_fixed_width_for_japanese", character_set_t::extended_unix_code_fixed_width_for_japanese },
    { "extended_unix_code_packed_format_for_japanese", character_set_t::extended_unix_code_packed_format_for_japanese },
    { "gb18030", character_set_t::gb18030 },
    { "gb2312", character_set_t::gb2312 },
    { "gb_1988-80", character_set_t::gb_1988_80 },
    { "gb_2312-80", character_set_t::gb_2312_80 },
    { "gbk", character_set_t::gbk },
    { "gost_19768-74", character_set_t::gost_19768_74 },
    { "greek-ccitt", character_set_t::greek_ccitt },
    { "greek7", character_set_t::greek7 },
    { "greek7-old", character_set_t::greek7_old },
    { "hp-desktop", character_set_t::hp_desktop },
    { "hp-legal", character_set_t::hp_legal },
    { "hp-math8", character_set_t::hp_math8 },
    { "hp-pi-font", character_set_t::hp_pi_font },
    { "hp-roman8", character_set_t::hp_roman8 },
    { "hz-gb-2312", character_set_t::hz_gb_2312 },
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
    { "ibm420", character_set_t::ibm420 },
    { "ibm423", character_set_t::ibm423 },
    { "ibm424", character_set_t::ibm424 },
    { "ibm437", character_set_t::ibm437 },
    { "ibm500", character_set_t::ibm500 },
    { "ibm775", character_set_t::ibm775 },
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
    { "iso-8859-1-windows-3.0-latin-1", character_set_t::iso_8859_1_windows_3_0_latin_1 },
    { "iso-8859-1-windows-3.1-latin-1", character_set_t::iso_8859_1_windows_3_1_latin_1 },
    { "iso-8859-10", character_set_t::iso_8859_10 },
    { "iso-8859-13", character_set_t::iso_8859_13 },
    { "iso-8859-14", character_set_t::iso_8859_14 },
    { "iso-8859-15", character_set_t::iso_8859_15 },
    { "iso-8859-16", character_set_t::iso_8859_16 },
    { "iso-8859-2-windows-latin-2", character_set_t::iso_8859_2_windows_latin_2 },
    { "iso-8859-9-windows-latin-5", character_set_t::iso_8859_9_windows_latin_5 },
    { "iso-ir-90", character_set_t::iso_ir_90 },
    { "iso-unicode-ibm-1261", character_set_t::iso_unicode_ibm_1261 },
    { "iso-unicode-ibm-1264", character_set_t::iso_unicode_ibm_1264 },
    { "iso-unicode-ibm-1265", character_set_t::iso_unicode_ibm_1265 },
    { "iso-unicode-ibm-1268", character_set_t::iso_unicode_ibm_1268 },
    { "iso-unicode-ibm-1276", character_set_t::iso_unicode_ibm_1276 },
    { "iso_10367-box", character_set_t::iso_10367_box },
    { "iso_2033-1983", character_set_t::iso_2033_1983 },
    { "iso_5427", character_set_t::iso_5427 },
    { "iso_5427:1981", character_set_t::iso_5427_1981 },
    { "iso_5428:1980", character_set_t::iso_5428_1980 },
    { "iso_646.basic:1983", character_set_t::iso_646_basic_1983 },
    { "iso_646.irv:1983", character_set_t::iso_646_irv_1983 },
    { "iso_6937-2-25", character_set_t::iso_6937_2_25 },
    { "iso_6937-2-add", character_set_t::iso_6937_2_add },
    { "iso_8859-1:1987", character_set_t::iso_8859_1_1987 },
    { "iso_8859-2:1987", character_set_t::iso_8859_2_1987 },
    { "iso_8859-3:1988", character_set_t::iso_8859_3_1988 },
    { "iso_8859-4:1988", character_set_t::iso_8859_4_1988 },
    { "iso_8859-5:1988", character_set_t::iso_8859_5_1988 },
    { "iso_8859-6-e", character_set_t::iso_8859_6_e },
    { "iso_8859-6-i", character_set_t::iso_8859_6_i },
    { "iso_8859-6:1987", character_set_t::iso_8859_6_1987 },
    { "iso_8859-7:1987", character_set_t::iso_8859_7_1987 },
    { "iso_8859-8-e", character_set_t::iso_8859_8_e },
    { "iso_8859-8-i", character_set_t::iso_8859_8_i },
    { "iso_8859-8:1988", character_set_t::iso_8859_8_1988 },
    { "iso_8859-9:1989", character_set_t::iso_8859_9_1989 },
    { "iso_8859-supp", character_set_t::iso_8859_supp },
    { "it", character_set_t::it },
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
    { "jis_x0212-1990", character_set_t::jis_x0212_1990 },
    { "jus_i.b1.002", character_set_t::jus_i_b1_002 },
    { "jus_i.b1.003-mac", character_set_t::jus_i_b1_003_mac },
    { "jus_i.b1.003-serb", character_set_t::jus_i_b1_003_serb },
    { "koi7-switched", character_set_t::koi7_switched },
    { "koi8-r", character_set_t::koi8_r },
    { "koi8-u", character_set_t::koi8_u },
    { "ks_c_5601-1987", character_set_t::ks_c_5601_1987 },
    { "ksc5636", character_set_t::ksc5636 },
    { "kz-1048", character_set_t::kz_1048 },
    { "latin-greek", character_set_t::latin_greek },
    { "latin-greek-1", character_set_t::latin_greek_1 },
    { "latin-lap", character_set_t::latin_lap },
    { "macintosh", character_set_t::macintosh },
    { "microsoft-publishing", character_set_t::microsoft_publishing },
    { "mnem", character_set_t::mnem },
    { "mnemonic", character_set_t::mnemonic },
    { "msz_7795.3", character_set_t::msz_7795_3 },
    { "nats-dano", character_set_t::nats_dano },
    { "nats-dano-add", character_set_t::nats_dano_add },
    { "nats-sefi", character_set_t::nats_sefi },
    { "nats-sefi-add", character_set_t::nats_sefi_add },
    { "nc_nc00-10:81", character_set_t::nc_nc00_10_81 },
    { "nf_z_62-010", character_set_t::nf_z_62_010 },
    { "nf_z_62-010_(1973", character_set_t::nf_z_62_010_1973 },
    { "ns_4551-1", character_set_t::ns_4551_1 },
    { "ns_4551-2", character_set_t::ns_4551_2 },
    { "osd_ebcdic_df03_irv", character_set_t::osd_ebcdic_df03_irv },
    { "osd_ebcdic_df04_1", character_set_t::osd_ebcdic_df04_1 },
    { "osd_ebcdic_df04_15", character_set_t::osd_ebcdic_df04_15 },
    { "pc8-danish-norwegian", character_set_t::pc8_danish_norwegian },
    { "pc8-turkish", character_set_t::pc8_turkish },
    { "pt", character_set_t::pt },
    { "pt2", character_set_t::pt2 },
    { "ptcp154", character_set_t::ptcp154 },
    { "scsu", character_set_t::scsu },
    { "sen_850200_b", character_set_t::sen_850200_b },
    { "sen_850200_c", character_set_t::sen_850200_c },
    { "shift_jis", character_set_t::shift_jis },
    { "t.101-g2", character_set_t::t_101_g2 },
    { "t.61-7bit", character_set_t::t_61_7bit },
    { "t.61-8bit", character_set_t::t_61_8bit },
    { "tis-620", character_set_t::tis_620 },
    { "tscii", character_set_t::tscii },
    { "unicode-1-1", character_set_t::unicode_1_1 },
    { "unicode-1-1-utf-7", character_set_t::unicode_1_1_utf_7 },
    { "unknown-8bit", character_set_t::unknown_8bit },
    { "us-ascii", character_set_t::us_ascii },
    { "us-dk", character_set_t::us_dk },
    { "utf-16", character_set_t::utf_16 },
    { "utf-16be", character_set_t::utf_16be },
    { "utf-16le", character_set_t::utf_16le },
    { "utf-32", character_set_t::utf_32 },
    { "utf-32be", character_set_t::utf_32be },
    { "utf-32le", character_set_t::utf_32le },
    { "utf-7", character_set_t::utf_7 },
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
};

const map_type& get()
{
    static map_type mt(entries, std::size(entries), character_set_t::unspecified);
    return mt;
}

} // namespace encoding

}

sax_token_handler_wrapper_base::sax_token_handler_wrapper_base(const tokens& _tokens) :
    m_tokens(_tokens) {}

xml_token_t sax_token_handler_wrapper_base::tokenize(std::string_view name) const
{
    xml_token_t token = XML_UNKNOWN_TOKEN;
    if (!name.empty())
        token = m_tokens.get_token(name);
    return token;
}

void sax_token_handler_wrapper_base::set_element(const sax_ns_parser_element& elem)
{
    m_elem.ns = elem.ns;
    m_elem.name = tokenize(elem.name);
    m_elem.raw_name = elem.name;
}

void sax_token_handler_wrapper_base::attribute(std::string_view name, std::string_view val)
{
    decl_attr_type dat = decl_attr::get().find(name);

    switch (dat)
    {
        case decl_attr_type::version:
        {
            const char* p = val.data();
            const char* p_end = p + val.size();

            long v;
            const char* endptr = parse_integer(p, p_end, v);

            if (!endptr || endptr >= p_end || *endptr != '.')
                break;

            m_declaration.version_major = v;
            p = endptr + 1;

            endptr = parse_integer(p, p_end, v);

            if (!endptr || endptr > p_end)
                break;

            m_declaration.version_minor = v;
            break;
        }
        case decl_attr_type::encoding:
        {
            // Convert the source encoding string to all lower-case first.
            std::string val_lower{val};
            std::transform(val_lower.begin(), val_lower.end(), val_lower.begin(),
                [](unsigned char c)
                {
                    return std::tolower(c);
                }
            );

            m_declaration.encoding = encoding::get().find(val_lower);
            break;
        }
        case decl_attr_type::standalone:
            m_declaration.standalone = (val == "yes") ? true : false;
            break;
        default:
            ;
    }
}

void sax_token_handler_wrapper_base::attribute(const sax_ns_parser_attribute& attr)
{
    m_elem.attrs.push_back(
       xml_token_attr_t(
           attr.ns, tokenize(attr.name), attr.name,
           attr.value, attr.transient));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
