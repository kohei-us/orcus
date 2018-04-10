/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/sax_token_parser.hpp"
#include "orcus/tokens.hpp"

#include <mdds/sorted_string_map.hpp>

namespace orcus {

namespace {

enum class decl_attr_type { unknown, version, encoding, standalone };

namespace decl_attr {

typedef mdds::sorted_string_map<decl_attr_type> map_type;

// Keys must be sorted.
const std::vector<map_type::entry> entries = {
    { ORCUS_ASCII("encoding"),   decl_attr_type::encoding   },
    { ORCUS_ASCII("standalone"), decl_attr_type::standalone },
    { ORCUS_ASCII("version"),    decl_attr_type::version    },
};

const map_type& get()
{
    static map_type mt(entries.data(), entries.size(), decl_attr_type::unknown);
    return mt;
}

} // namespace decl_attr

namespace encoding {

typedef mdds::sorted_string_map<xml_encoding_t> map_type;

// Keys must be sorted.
const std::vector<map_type::entry> entries = {
    { ORCUS_ASCII("adobe-standard-encoding"), xml_encoding_t::adobe_standard_encoding },
    { ORCUS_ASCII("adobe-symbol-encoding"), xml_encoding_t::adobe_symbol_encoding },
    { ORCUS_ASCII("amiga-1251"), xml_encoding_t::amiga_1251 },
    { ORCUS_ASCII("ansi_x3.110-1983"), xml_encoding_t::ansi_x3_110_1983 },
    { ORCUS_ASCII("asmo_449"), xml_encoding_t::asmo_449 },
    { ORCUS_ASCII("big5"), xml_encoding_t::big5 },
    { ORCUS_ASCII("big5-hkscs"), xml_encoding_t::big5_hkscs },
    { ORCUS_ASCII("bocu-1"), xml_encoding_t::bocu_1 },
    { ORCUS_ASCII("brf"), xml_encoding_t::brf },
    { ORCUS_ASCII("bs_4730"), xml_encoding_t::bs_4730 },
    { ORCUS_ASCII("bs_viewdata"), xml_encoding_t::bs_viewdata },
    { ORCUS_ASCII("cesu-8"), xml_encoding_t::cesu_8 },
    { ORCUS_ASCII("cp50220"), xml_encoding_t::cp50220 },
    { ORCUS_ASCII("cp51932"), xml_encoding_t::cp51932 },
    { ORCUS_ASCII("csa_z243.4-1985-1"), xml_encoding_t::csa_z243_4_1985_1 },
    { ORCUS_ASCII("csa_z243.4-1985-2"), xml_encoding_t::csa_z243_4_1985_2 },
    { ORCUS_ASCII("csa_z243.4-1985-gr"), xml_encoding_t::csa_z243_4_1985_gr },
    { ORCUS_ASCII("csn_369103"), xml_encoding_t::csn_369103 },
    { ORCUS_ASCII("dec-mcs"), xml_encoding_t::dec_mcs },
    { ORCUS_ASCII("din_66003"), xml_encoding_t::din_66003 },
    { ORCUS_ASCII("dk-us"), xml_encoding_t::dk_us },
    { ORCUS_ASCII("ds_2089"), xml_encoding_t::ds_2089 },
    { ORCUS_ASCII("ebcdic-at-de"), xml_encoding_t::ebcdic_at_de },
    { ORCUS_ASCII("ebcdic-at-de-a"), xml_encoding_t::ebcdic_at_de_a },
    { ORCUS_ASCII("ebcdic-ca-fr"), xml_encoding_t::ebcdic_ca_fr },
    { ORCUS_ASCII("ebcdic-dk-no"), xml_encoding_t::ebcdic_dk_no },
    { ORCUS_ASCII("ebcdic-dk-no-a"), xml_encoding_t::ebcdic_dk_no_a },
    { ORCUS_ASCII("ebcdic-es"), xml_encoding_t::ebcdic_es },
    { ORCUS_ASCII("ebcdic-es-a"), xml_encoding_t::ebcdic_es_a },
    { ORCUS_ASCII("ebcdic-es-s"), xml_encoding_t::ebcdic_es_s },
    { ORCUS_ASCII("ebcdic-fi-se"), xml_encoding_t::ebcdic_fi_se },
    { ORCUS_ASCII("ebcdic-fi-se-a"), xml_encoding_t::ebcdic_fi_se_a },
    { ORCUS_ASCII("ebcdic-fr"), xml_encoding_t::ebcdic_fr },
    { ORCUS_ASCII("ebcdic-it"), xml_encoding_t::ebcdic_it },
    { ORCUS_ASCII("ebcdic-pt"), xml_encoding_t::ebcdic_pt },
    { ORCUS_ASCII("ebcdic-uk"), xml_encoding_t::ebcdic_uk },
    { ORCUS_ASCII("ebcdic-us"), xml_encoding_t::ebcdic_us },
    { ORCUS_ASCII("ecma-cyrillic"), xml_encoding_t::ecma_cyrillic },
    { ORCUS_ASCII("es"), xml_encoding_t::es },
    { ORCUS_ASCII("es2"), xml_encoding_t::es2 },
    { ORCUS_ASCII("euc-kr"), xml_encoding_t::euc_kr },
    { ORCUS_ASCII("extended_unix_code_fixed_width_for_japanese"), xml_encoding_t::extended_unix_code_fixed_width_for_japanese },
    { ORCUS_ASCII("extended_unix_code_packed_format_for_japanese"), xml_encoding_t::extended_unix_code_packed_format_for_japanese },
    { ORCUS_ASCII("gb18030"), xml_encoding_t::gb18030 },
    { ORCUS_ASCII("gb2312"), xml_encoding_t::gb2312 },
    { ORCUS_ASCII("gb_1988-80"), xml_encoding_t::gb_1988_80 },
    { ORCUS_ASCII("gb_2312-80"), xml_encoding_t::gb_2312_80 },
    { ORCUS_ASCII("gbk"), xml_encoding_t::gbk },
    { ORCUS_ASCII("gost_19768-74"), xml_encoding_t::gost_19768_74 },
    { ORCUS_ASCII("greek-ccitt"), xml_encoding_t::greek_ccitt },
    { ORCUS_ASCII("greek7"), xml_encoding_t::greek7 },
    { ORCUS_ASCII("greek7-old"), xml_encoding_t::greek7_old },
    { ORCUS_ASCII("hp-desktop"), xml_encoding_t::hp_desktop },
    { ORCUS_ASCII("hp-legal"), xml_encoding_t::hp_legal },
    { ORCUS_ASCII("hp-math8"), xml_encoding_t::hp_math8 },
    { ORCUS_ASCII("hp-pi-font"), xml_encoding_t::hp_pi_font },
    { ORCUS_ASCII("hp-roman8"), xml_encoding_t::hp_roman8 },
    { ORCUS_ASCII("hz-gb-2312"), xml_encoding_t::hz_gb_2312 },
    { ORCUS_ASCII("ibm-symbols"), xml_encoding_t::ibm_symbols },
    { ORCUS_ASCII("ibm-thai"), xml_encoding_t::ibm_thai },
    { ORCUS_ASCII("ibm00858"), xml_encoding_t::ibm00858 },
    { ORCUS_ASCII("ibm00924"), xml_encoding_t::ibm00924 },
    { ORCUS_ASCII("ibm01140"), xml_encoding_t::ibm01140 },
    { ORCUS_ASCII("ibm01141"), xml_encoding_t::ibm01141 },
    { ORCUS_ASCII("ibm01142"), xml_encoding_t::ibm01142 },
    { ORCUS_ASCII("ibm01143"), xml_encoding_t::ibm01143 },
    { ORCUS_ASCII("ibm01144"), xml_encoding_t::ibm01144 },
    { ORCUS_ASCII("ibm01145"), xml_encoding_t::ibm01145 },
    { ORCUS_ASCII("ibm01146"), xml_encoding_t::ibm01146 },
    { ORCUS_ASCII("ibm01147"), xml_encoding_t::ibm01147 },
    { ORCUS_ASCII("ibm01148"), xml_encoding_t::ibm01148 },
    { ORCUS_ASCII("ibm01149"), xml_encoding_t::ibm01149 },
    { ORCUS_ASCII("ibm037"), xml_encoding_t::ibm037 },
    { ORCUS_ASCII("ibm038"), xml_encoding_t::ibm038 },
    { ORCUS_ASCII("ibm1026"), xml_encoding_t::ibm1026 },
    { ORCUS_ASCII("ibm1047"), xml_encoding_t::ibm1047 },
    { ORCUS_ASCII("ibm273"), xml_encoding_t::ibm273 },
    { ORCUS_ASCII("ibm274"), xml_encoding_t::ibm274 },
    { ORCUS_ASCII("ibm275"), xml_encoding_t::ibm275 },
    { ORCUS_ASCII("ibm277"), xml_encoding_t::ibm277 },
    { ORCUS_ASCII("ibm278"), xml_encoding_t::ibm278 },
    { ORCUS_ASCII("ibm280"), xml_encoding_t::ibm280 },
    { ORCUS_ASCII("ibm281"), xml_encoding_t::ibm281 },
    { ORCUS_ASCII("ibm284"), xml_encoding_t::ibm284 },
    { ORCUS_ASCII("ibm285"), xml_encoding_t::ibm285 },
    { ORCUS_ASCII("ibm290"), xml_encoding_t::ibm290 },
    { ORCUS_ASCII("ibm297"), xml_encoding_t::ibm297 },
    { ORCUS_ASCII("ibm420"), xml_encoding_t::ibm420 },
    { ORCUS_ASCII("ibm423"), xml_encoding_t::ibm423 },
    { ORCUS_ASCII("ibm424"), xml_encoding_t::ibm424 },
    { ORCUS_ASCII("ibm437"), xml_encoding_t::ibm437 },
    { ORCUS_ASCII("ibm500"), xml_encoding_t::ibm500 },
    { ORCUS_ASCII("ibm775"), xml_encoding_t::ibm775 },
    { ORCUS_ASCII("ibm850"), xml_encoding_t::ibm850 },
    { ORCUS_ASCII("ibm851"), xml_encoding_t::ibm851 },
    { ORCUS_ASCII("ibm852"), xml_encoding_t::ibm852 },
    { ORCUS_ASCII("ibm855"), xml_encoding_t::ibm855 },
    { ORCUS_ASCII("ibm857"), xml_encoding_t::ibm857 },
    { ORCUS_ASCII("ibm860"), xml_encoding_t::ibm860 },
    { ORCUS_ASCII("ibm861"), xml_encoding_t::ibm861 },
    { ORCUS_ASCII("ibm862"), xml_encoding_t::ibm862 },
    { ORCUS_ASCII("ibm863"), xml_encoding_t::ibm863 },
    { ORCUS_ASCII("ibm864"), xml_encoding_t::ibm864 },
    { ORCUS_ASCII("ibm865"), xml_encoding_t::ibm865 },
    { ORCUS_ASCII("ibm866"), xml_encoding_t::ibm866 },
    { ORCUS_ASCII("ibm868"), xml_encoding_t::ibm868 },
    { ORCUS_ASCII("ibm869"), xml_encoding_t::ibm869 },
    { ORCUS_ASCII("ibm870"), xml_encoding_t::ibm870 },
    { ORCUS_ASCII("ibm871"), xml_encoding_t::ibm871 },
    { ORCUS_ASCII("ibm880"), xml_encoding_t::ibm880 },
    { ORCUS_ASCII("ibm891"), xml_encoding_t::ibm891 },
    { ORCUS_ASCII("ibm903"), xml_encoding_t::ibm903 },
    { ORCUS_ASCII("ibm904"), xml_encoding_t::ibm904 },
    { ORCUS_ASCII("ibm905"), xml_encoding_t::ibm905 },
    { ORCUS_ASCII("ibm918"), xml_encoding_t::ibm918 },
    { ORCUS_ASCII("iec_p27-1"), xml_encoding_t::iec_p27_1 },
    { ORCUS_ASCII("inis"), xml_encoding_t::inis },
    { ORCUS_ASCII("inis-8"), xml_encoding_t::inis_8 },
    { ORCUS_ASCII("inis-cyrillic"), xml_encoding_t::inis_cyrillic },
    { ORCUS_ASCII("invariant"), xml_encoding_t::invariant },
    { ORCUS_ASCII("iso-10646-j-1"), xml_encoding_t::iso_10646_j_1 },
    { ORCUS_ASCII("iso-10646-ucs-2"), xml_encoding_t::iso_10646_ucs_2 },
    { ORCUS_ASCII("iso-10646-ucs-4"), xml_encoding_t::iso_10646_ucs_4 },
    { ORCUS_ASCII("iso-10646-ucs-basic"), xml_encoding_t::iso_10646_ucs_basic },
    { ORCUS_ASCII("iso-10646-unicode-latin1"), xml_encoding_t::iso_10646_unicode_latin1 },
    { ORCUS_ASCII("iso-10646-utf-1"), xml_encoding_t::iso_10646_utf_1 },
    { ORCUS_ASCII("iso-11548-1"), xml_encoding_t::iso_11548_1 },
    { ORCUS_ASCII("iso-2022-cn"), xml_encoding_t::iso_2022_cn },
    { ORCUS_ASCII("iso-2022-cn-ext"), xml_encoding_t::iso_2022_cn_ext },
    { ORCUS_ASCII("iso-2022-jp"), xml_encoding_t::iso_2022_jp },
    { ORCUS_ASCII("iso-2022-jp-2"), xml_encoding_t::iso_2022_jp_2 },
    { ORCUS_ASCII("iso-2022-kr"), xml_encoding_t::iso_2022_kr },
    { ORCUS_ASCII("iso-8859-1-windows-3.0-latin-1"), xml_encoding_t::iso_8859_1_windows_3_0_latin_1 },
    { ORCUS_ASCII("iso-8859-1-windows-3.1-latin-1"), xml_encoding_t::iso_8859_1_windows_3_1_latin_1 },
    { ORCUS_ASCII("iso-8859-10"), xml_encoding_t::iso_8859_10 },
    { ORCUS_ASCII("iso-8859-13"), xml_encoding_t::iso_8859_13 },
    { ORCUS_ASCII("iso-8859-14"), xml_encoding_t::iso_8859_14 },
    { ORCUS_ASCII("iso-8859-15"), xml_encoding_t::iso_8859_15 },
    { ORCUS_ASCII("iso-8859-16"), xml_encoding_t::iso_8859_16 },
    { ORCUS_ASCII("iso-8859-2-windows-latin-2"), xml_encoding_t::iso_8859_2_windows_latin_2 },
    { ORCUS_ASCII("iso-8859-9-windows-latin-5"), xml_encoding_t::iso_8859_9_windows_latin_5 },
    { ORCUS_ASCII("iso-ir-90"), xml_encoding_t::iso_ir_90 },
    { ORCUS_ASCII("iso-unicode-ibm-1261"), xml_encoding_t::iso_unicode_ibm_1261 },
    { ORCUS_ASCII("iso-unicode-ibm-1264"), xml_encoding_t::iso_unicode_ibm_1264 },
    { ORCUS_ASCII("iso-unicode-ibm-1265"), xml_encoding_t::iso_unicode_ibm_1265 },
    { ORCUS_ASCII("iso-unicode-ibm-1268"), xml_encoding_t::iso_unicode_ibm_1268 },
    { ORCUS_ASCII("iso-unicode-ibm-1276"), xml_encoding_t::iso_unicode_ibm_1276 },
    { ORCUS_ASCII("iso_10367-box"), xml_encoding_t::iso_10367_box },
    { ORCUS_ASCII("iso_2033-1983"), xml_encoding_t::iso_2033_1983 },
    { ORCUS_ASCII("iso_5427"), xml_encoding_t::iso_5427 },
    { ORCUS_ASCII("iso_5427:1981"), xml_encoding_t::iso_5427_1981 },
    { ORCUS_ASCII("iso_5428:1980"), xml_encoding_t::iso_5428_1980 },
    { ORCUS_ASCII("iso_646.basic:1983"), xml_encoding_t::iso_646_basic_1983 },
    { ORCUS_ASCII("iso_646.irv:1983"), xml_encoding_t::iso_646_irv_1983 },
    { ORCUS_ASCII("iso_6937-2-25"), xml_encoding_t::iso_6937_2_25 },
    { ORCUS_ASCII("iso_6937-2-add"), xml_encoding_t::iso_6937_2_add },
    { ORCUS_ASCII("iso_8859-1:1987"), xml_encoding_t::iso_8859_1_1987 },
    { ORCUS_ASCII("iso_8859-2:1987"), xml_encoding_t::iso_8859_2_1987 },
    { ORCUS_ASCII("iso_8859-3:1988"), xml_encoding_t::iso_8859_3_1988 },
    { ORCUS_ASCII("iso_8859-4:1988"), xml_encoding_t::iso_8859_4_1988 },
    { ORCUS_ASCII("iso_8859-5:1988"), xml_encoding_t::iso_8859_5_1988 },
    { ORCUS_ASCII("iso_8859-6-e"), xml_encoding_t::iso_8859_6_e },
    { ORCUS_ASCII("iso_8859-6-i"), xml_encoding_t::iso_8859_6_i },
    { ORCUS_ASCII("iso_8859-6:1987"), xml_encoding_t::iso_8859_6_1987 },
    { ORCUS_ASCII("iso_8859-7:1987"), xml_encoding_t::iso_8859_7_1987 },
    { ORCUS_ASCII("iso_8859-8-e"), xml_encoding_t::iso_8859_8_e },
    { ORCUS_ASCII("iso_8859-8-i"), xml_encoding_t::iso_8859_8_i },
    { ORCUS_ASCII("iso_8859-8:1988"), xml_encoding_t::iso_8859_8_1988 },
    { ORCUS_ASCII("iso_8859-9:1989"), xml_encoding_t::iso_8859_9_1989 },
    { ORCUS_ASCII("iso_8859-supp"), xml_encoding_t::iso_8859_supp },
    { ORCUS_ASCII("it"), xml_encoding_t::it },
    { ORCUS_ASCII("jis_c6220-1969-jp"), xml_encoding_t::jis_c6220_1969_jp },
    { ORCUS_ASCII("jis_c6220-1969-ro"), xml_encoding_t::jis_c6220_1969_ro },
    { ORCUS_ASCII("jis_c6226-1978"), xml_encoding_t::jis_c6226_1978 },
    { ORCUS_ASCII("jis_c6226-1983"), xml_encoding_t::jis_c6226_1983 },
    { ORCUS_ASCII("jis_c6229-1984-a"), xml_encoding_t::jis_c6229_1984_a },
    { ORCUS_ASCII("jis_c6229-1984-b"), xml_encoding_t::jis_c6229_1984_b },
    { ORCUS_ASCII("jis_c6229-1984-b-add"), xml_encoding_t::jis_c6229_1984_b_add },
    { ORCUS_ASCII("jis_c6229-1984-hand"), xml_encoding_t::jis_c6229_1984_hand },
    { ORCUS_ASCII("jis_c6229-1984-hand-add"), xml_encoding_t::jis_c6229_1984_hand_add },
    { ORCUS_ASCII("jis_c6229-1984-kana"), xml_encoding_t::jis_c6229_1984_kana },
    { ORCUS_ASCII("jis_encoding"), xml_encoding_t::jis_encoding },
    { ORCUS_ASCII("jis_x0201"), xml_encoding_t::jis_x0201 },
    { ORCUS_ASCII("jis_x0212-1990"), xml_encoding_t::jis_x0212_1990 },
    { ORCUS_ASCII("jus_i.b1.002"), xml_encoding_t::jus_i_b1_002 },
    { ORCUS_ASCII("jus_i.b1.003-mac"), xml_encoding_t::jus_i_b1_003_mac },
    { ORCUS_ASCII("jus_i.b1.003-serb"), xml_encoding_t::jus_i_b1_003_serb },
    { ORCUS_ASCII("koi7-switched"), xml_encoding_t::koi7_switched },
    { ORCUS_ASCII("koi8-r"), xml_encoding_t::koi8_r },
    { ORCUS_ASCII("koi8-u"), xml_encoding_t::koi8_u },
    { ORCUS_ASCII("ks_c_5601-1987"), xml_encoding_t::ks_c_5601_1987 },
    { ORCUS_ASCII("ksc5636"), xml_encoding_t::ksc5636 },
    { ORCUS_ASCII("kz-1048"), xml_encoding_t::kz_1048 },
    { ORCUS_ASCII("latin-greek"), xml_encoding_t::latin_greek },
    { ORCUS_ASCII("latin-greek-1"), xml_encoding_t::latin_greek_1 },
    { ORCUS_ASCII("latin-lap"), xml_encoding_t::latin_lap },
    { ORCUS_ASCII("macintosh"), xml_encoding_t::macintosh },
    { ORCUS_ASCII("microsoft-publishing"), xml_encoding_t::microsoft_publishing },
    { ORCUS_ASCII("mnem"), xml_encoding_t::mnem },
    { ORCUS_ASCII("mnemonic"), xml_encoding_t::mnemonic },
    { ORCUS_ASCII("msz_7795.3"), xml_encoding_t::msz_7795_3 },
    { ORCUS_ASCII("nats-dano"), xml_encoding_t::nats_dano },
    { ORCUS_ASCII("nats-dano-add"), xml_encoding_t::nats_dano_add },
    { ORCUS_ASCII("nats-sefi"), xml_encoding_t::nats_sefi },
    { ORCUS_ASCII("nats-sefi-add"), xml_encoding_t::nats_sefi_add },
    { ORCUS_ASCII("nc_nc00-10:81"), xml_encoding_t::nc_nc00_10_81 },
    { ORCUS_ASCII("nf_z_62-010"), xml_encoding_t::nf_z_62_010 },
    { ORCUS_ASCII("nf_z_62-010_(1973)"), xml_encoding_t::nf_z_62_010_1973 },
    { ORCUS_ASCII("ns_4551-1"), xml_encoding_t::ns_4551_1 },
    { ORCUS_ASCII("ns_4551-2"), xml_encoding_t::ns_4551_2 },
    { ORCUS_ASCII("osd_ebcdic_df03_irv"), xml_encoding_t::osd_ebcdic_df03_irv },
    { ORCUS_ASCII("osd_ebcdic_df04_1"), xml_encoding_t::osd_ebcdic_df04_1 },
    { ORCUS_ASCII("osd_ebcdic_df04_15"), xml_encoding_t::osd_ebcdic_df04_15 },
    { ORCUS_ASCII("pc8-danish-norwegian"), xml_encoding_t::pc8_danish_norwegian },
    { ORCUS_ASCII("pc8-turkish"), xml_encoding_t::pc8_turkish },
    { ORCUS_ASCII("pt"), xml_encoding_t::pt },
    { ORCUS_ASCII("pt2"), xml_encoding_t::pt2 },
    { ORCUS_ASCII("ptcp154"), xml_encoding_t::ptcp154 },
    { ORCUS_ASCII("scsu"), xml_encoding_t::scsu },
    { ORCUS_ASCII("sen_850200_b"), xml_encoding_t::sen_850200_b },
    { ORCUS_ASCII("sen_850200_c"), xml_encoding_t::sen_850200_c },
    { ORCUS_ASCII("shift_jis"), xml_encoding_t::shift_jis },
    { ORCUS_ASCII("t.101-g2"), xml_encoding_t::t_101_g2 },
    { ORCUS_ASCII("t.61-7bit"), xml_encoding_t::t_61_7bit },
    { ORCUS_ASCII("t.61-8bit"), xml_encoding_t::t_61_8bit },
    { ORCUS_ASCII("tis-620"), xml_encoding_t::tis_620 },
    { ORCUS_ASCII("tscii"), xml_encoding_t::tscii },
    { ORCUS_ASCII("unicode-1-1"), xml_encoding_t::unicode_1_1 },
    { ORCUS_ASCII("unicode-1-1-utf-7"), xml_encoding_t::unicode_1_1_utf_7 },
    { ORCUS_ASCII("unknown-8bit"), xml_encoding_t::unknown_8bit },
    { ORCUS_ASCII("us-ascii"), xml_encoding_t::us_ascii },
    { ORCUS_ASCII("us-dk"), xml_encoding_t::us_dk },
    { ORCUS_ASCII("utf-16"), xml_encoding_t::utf_16 },
    { ORCUS_ASCII("utf-16be"), xml_encoding_t::utf_16be },
    { ORCUS_ASCII("utf-16le"), xml_encoding_t::utf_16le },
    { ORCUS_ASCII("utf-32"), xml_encoding_t::utf_32 },
    { ORCUS_ASCII("utf-32be"), xml_encoding_t::utf_32be },
    { ORCUS_ASCII("utf-32le"), xml_encoding_t::utf_32le },
    { ORCUS_ASCII("utf-7"), xml_encoding_t::utf_7 },
    { ORCUS_ASCII("utf-8"), xml_encoding_t::utf_8 },
    { ORCUS_ASCII("ventura-international"), xml_encoding_t::ventura_international },
    { ORCUS_ASCII("ventura-math"), xml_encoding_t::ventura_math },
    { ORCUS_ASCII("ventura-us"), xml_encoding_t::ventura_us },
    { ORCUS_ASCII("videotex-suppl"), xml_encoding_t::videotex_suppl },
    { ORCUS_ASCII("viqr"), xml_encoding_t::viqr },
    { ORCUS_ASCII("viscii"), xml_encoding_t::viscii },
    { ORCUS_ASCII("windows-1250"), xml_encoding_t::windows_1250 },
    { ORCUS_ASCII("windows-1251"), xml_encoding_t::windows_1251 },
    { ORCUS_ASCII("windows-1252"), xml_encoding_t::windows_1252 },
    { ORCUS_ASCII("windows-1253"), xml_encoding_t::windows_1253 },
    { ORCUS_ASCII("windows-1254"), xml_encoding_t::windows_1254 },
    { ORCUS_ASCII("windows-1255"), xml_encoding_t::windows_1255 },
    { ORCUS_ASCII("windows-1256"), xml_encoding_t::windows_1256 },
    { ORCUS_ASCII("windows-1257"), xml_encoding_t::windows_1257 },
    { ORCUS_ASCII("windows-1258"), xml_encoding_t::windows_1258 },
    { ORCUS_ASCII("windows-31j"), xml_encoding_t::windows_31j },
    { ORCUS_ASCII("windows-874"), xml_encoding_t::windows_874 },
};

const map_type& get()
{
    static map_type mt(entries.data(), entries.size(), xml_encoding_t::unspecified);
    return mt;
}

} // namespace encoding

}

sax_token_handler_wrapper_base::sax_token_handler_wrapper_base(const tokens& _tokens) :
    m_tokens(_tokens) {}

xml_token_t sax_token_handler_wrapper_base::tokenize(const pstring& name) const
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

void sax_token_handler_wrapper_base::attribute(const pstring& name, const pstring& val)
{
    decl_attr_type dat = decl_attr::get().find(name.data(), name.size());

    switch (dat)
    {
        case decl_attr_type::version:
        {
            const char* p = val.data();
            const char* p_end = p + val.size();

            char* endptr = nullptr;
            long v = std::strtol(p, &endptr, 10);

            if (!endptr || endptr >= p_end || *endptr != '.')
                break;

            m_declaration.version_major = v;
            p = endptr + 1;

            v = std::strtol(p, &endptr, 10);

            if (!endptr || endptr > p_end)
                break;

            m_declaration.version_minor = v;
            break;
        }
        case decl_attr_type::encoding:
            m_declaration.encoding = encoding::get().find(val.data(), val.size());
            break;
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
