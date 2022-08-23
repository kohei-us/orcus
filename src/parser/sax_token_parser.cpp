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

typedef mdds::sorted_string_map<character_set_t> map_type;

// Keys must be sorted.
const std::vector<map_type::entry> entries = {
    { ORCUS_ASCII("adobe-standard-encoding"), character_set_t::adobe_standard_encoding },
    { ORCUS_ASCII("adobe-symbol-encoding"), character_set_t::adobe_symbol_encoding },
    { ORCUS_ASCII("amiga-1251"), character_set_t::amiga_1251 },
    { ORCUS_ASCII("ansi_x3.110-1983"), character_set_t::ansi_x3_110_1983 },
    { ORCUS_ASCII("asmo_449"), character_set_t::asmo_449 },
    { ORCUS_ASCII("big5"), character_set_t::big5 },
    { ORCUS_ASCII("big5-hkscs"), character_set_t::big5_hkscs },
    { ORCUS_ASCII("bocu-1"), character_set_t::bocu_1 },
    { ORCUS_ASCII("brf"), character_set_t::brf },
    { ORCUS_ASCII("bs_4730"), character_set_t::bs_4730 },
    { ORCUS_ASCII("bs_viewdata"), character_set_t::bs_viewdata },
    { ORCUS_ASCII("cesu-8"), character_set_t::cesu_8 },
    { ORCUS_ASCII("cp50220"), character_set_t::cp50220 },
    { ORCUS_ASCII("cp51932"), character_set_t::cp51932 },
    { ORCUS_ASCII("csa_z243.4-1985-1"), character_set_t::csa_z243_4_1985_1 },
    { ORCUS_ASCII("csa_z243.4-1985-2"), character_set_t::csa_z243_4_1985_2 },
    { ORCUS_ASCII("csa_z243.4-1985-gr"), character_set_t::csa_z243_4_1985_gr },
    { ORCUS_ASCII("csn_369103"), character_set_t::csn_369103 },
    { ORCUS_ASCII("dec-mcs"), character_set_t::dec_mcs },
    { ORCUS_ASCII("din_66003"), character_set_t::din_66003 },
    { ORCUS_ASCII("dk-us"), character_set_t::dk_us },
    { ORCUS_ASCII("ds_2089"), character_set_t::ds_2089 },
    { ORCUS_ASCII("ebcdic-at-de"), character_set_t::ebcdic_at_de },
    { ORCUS_ASCII("ebcdic-at-de-a"), character_set_t::ebcdic_at_de_a },
    { ORCUS_ASCII("ebcdic-ca-fr"), character_set_t::ebcdic_ca_fr },
    { ORCUS_ASCII("ebcdic-dk-no"), character_set_t::ebcdic_dk_no },
    { ORCUS_ASCII("ebcdic-dk-no-a"), character_set_t::ebcdic_dk_no_a },
    { ORCUS_ASCII("ebcdic-es"), character_set_t::ebcdic_es },
    { ORCUS_ASCII("ebcdic-es-a"), character_set_t::ebcdic_es_a },
    { ORCUS_ASCII("ebcdic-es-s"), character_set_t::ebcdic_es_s },
    { ORCUS_ASCII("ebcdic-fi-se"), character_set_t::ebcdic_fi_se },
    { ORCUS_ASCII("ebcdic-fi-se-a"), character_set_t::ebcdic_fi_se_a },
    { ORCUS_ASCII("ebcdic-fr"), character_set_t::ebcdic_fr },
    { ORCUS_ASCII("ebcdic-it"), character_set_t::ebcdic_it },
    { ORCUS_ASCII("ebcdic-pt"), character_set_t::ebcdic_pt },
    { ORCUS_ASCII("ebcdic-uk"), character_set_t::ebcdic_uk },
    { ORCUS_ASCII("ebcdic-us"), character_set_t::ebcdic_us },
    { ORCUS_ASCII("ecma-cyrillic"), character_set_t::ecma_cyrillic },
    { ORCUS_ASCII("es"), character_set_t::es },
    { ORCUS_ASCII("es2"), character_set_t::es2 },
    { ORCUS_ASCII("euc-kr"), character_set_t::euc_kr },
    { ORCUS_ASCII("extended_unix_code_fixed_width_for_japanese"), character_set_t::extended_unix_code_fixed_width_for_japanese },
    { ORCUS_ASCII("extended_unix_code_packed_format_for_japanese"), character_set_t::extended_unix_code_packed_format_for_japanese },
    { ORCUS_ASCII("gb18030"), character_set_t::gb18030 },
    { ORCUS_ASCII("gb2312"), character_set_t::gb2312 },
    { ORCUS_ASCII("gb_1988-80"), character_set_t::gb_1988_80 },
    { ORCUS_ASCII("gb_2312-80"), character_set_t::gb_2312_80 },
    { ORCUS_ASCII("gbk"), character_set_t::gbk },
    { ORCUS_ASCII("gost_19768-74"), character_set_t::gost_19768_74 },
    { ORCUS_ASCII("greek-ccitt"), character_set_t::greek_ccitt },
    { ORCUS_ASCII("greek7"), character_set_t::greek7 },
    { ORCUS_ASCII("greek7-old"), character_set_t::greek7_old },
    { ORCUS_ASCII("hp-desktop"), character_set_t::hp_desktop },
    { ORCUS_ASCII("hp-legal"), character_set_t::hp_legal },
    { ORCUS_ASCII("hp-math8"), character_set_t::hp_math8 },
    { ORCUS_ASCII("hp-pi-font"), character_set_t::hp_pi_font },
    { ORCUS_ASCII("hp-roman8"), character_set_t::hp_roman8 },
    { ORCUS_ASCII("hz-gb-2312"), character_set_t::hz_gb_2312 },
    { ORCUS_ASCII("ibm-symbols"), character_set_t::ibm_symbols },
    { ORCUS_ASCII("ibm-thai"), character_set_t::ibm_thai },
    { ORCUS_ASCII("ibm00858"), character_set_t::ibm00858 },
    { ORCUS_ASCII("ibm00924"), character_set_t::ibm00924 },
    { ORCUS_ASCII("ibm01140"), character_set_t::ibm01140 },
    { ORCUS_ASCII("ibm01141"), character_set_t::ibm01141 },
    { ORCUS_ASCII("ibm01142"), character_set_t::ibm01142 },
    { ORCUS_ASCII("ibm01143"), character_set_t::ibm01143 },
    { ORCUS_ASCII("ibm01144"), character_set_t::ibm01144 },
    { ORCUS_ASCII("ibm01145"), character_set_t::ibm01145 },
    { ORCUS_ASCII("ibm01146"), character_set_t::ibm01146 },
    { ORCUS_ASCII("ibm01147"), character_set_t::ibm01147 },
    { ORCUS_ASCII("ibm01148"), character_set_t::ibm01148 },
    { ORCUS_ASCII("ibm01149"), character_set_t::ibm01149 },
    { ORCUS_ASCII("ibm037"), character_set_t::ibm037 },
    { ORCUS_ASCII("ibm038"), character_set_t::ibm038 },
    { ORCUS_ASCII("ibm1026"), character_set_t::ibm1026 },
    { ORCUS_ASCII("ibm1047"), character_set_t::ibm1047 },
    { ORCUS_ASCII("ibm273"), character_set_t::ibm273 },
    { ORCUS_ASCII("ibm274"), character_set_t::ibm274 },
    { ORCUS_ASCII("ibm275"), character_set_t::ibm275 },
    { ORCUS_ASCII("ibm277"), character_set_t::ibm277 },
    { ORCUS_ASCII("ibm278"), character_set_t::ibm278 },
    { ORCUS_ASCII("ibm280"), character_set_t::ibm280 },
    { ORCUS_ASCII("ibm281"), character_set_t::ibm281 },
    { ORCUS_ASCII("ibm284"), character_set_t::ibm284 },
    { ORCUS_ASCII("ibm285"), character_set_t::ibm285 },
    { ORCUS_ASCII("ibm290"), character_set_t::ibm290 },
    { ORCUS_ASCII("ibm297"), character_set_t::ibm297 },
    { ORCUS_ASCII("ibm420"), character_set_t::ibm420 },
    { ORCUS_ASCII("ibm423"), character_set_t::ibm423 },
    { ORCUS_ASCII("ibm424"), character_set_t::ibm424 },
    { ORCUS_ASCII("ibm437"), character_set_t::ibm437 },
    { ORCUS_ASCII("ibm500"), character_set_t::ibm500 },
    { ORCUS_ASCII("ibm775"), character_set_t::ibm775 },
    { ORCUS_ASCII("ibm850"), character_set_t::ibm850 },
    { ORCUS_ASCII("ibm851"), character_set_t::ibm851 },
    { ORCUS_ASCII("ibm852"), character_set_t::ibm852 },
    { ORCUS_ASCII("ibm855"), character_set_t::ibm855 },
    { ORCUS_ASCII("ibm857"), character_set_t::ibm857 },
    { ORCUS_ASCII("ibm860"), character_set_t::ibm860 },
    { ORCUS_ASCII("ibm861"), character_set_t::ibm861 },
    { ORCUS_ASCII("ibm862"), character_set_t::ibm862 },
    { ORCUS_ASCII("ibm863"), character_set_t::ibm863 },
    { ORCUS_ASCII("ibm864"), character_set_t::ibm864 },
    { ORCUS_ASCII("ibm865"), character_set_t::ibm865 },
    { ORCUS_ASCII("ibm866"), character_set_t::ibm866 },
    { ORCUS_ASCII("ibm868"), character_set_t::ibm868 },
    { ORCUS_ASCII("ibm869"), character_set_t::ibm869 },
    { ORCUS_ASCII("ibm870"), character_set_t::ibm870 },
    { ORCUS_ASCII("ibm871"), character_set_t::ibm871 },
    { ORCUS_ASCII("ibm880"), character_set_t::ibm880 },
    { ORCUS_ASCII("ibm891"), character_set_t::ibm891 },
    { ORCUS_ASCII("ibm903"), character_set_t::ibm903 },
    { ORCUS_ASCII("ibm904"), character_set_t::ibm904 },
    { ORCUS_ASCII("ibm905"), character_set_t::ibm905 },
    { ORCUS_ASCII("ibm918"), character_set_t::ibm918 },
    { ORCUS_ASCII("iec_p27-1"), character_set_t::iec_p27_1 },
    { ORCUS_ASCII("inis"), character_set_t::inis },
    { ORCUS_ASCII("inis-8"), character_set_t::inis_8 },
    { ORCUS_ASCII("inis-cyrillic"), character_set_t::inis_cyrillic },
    { ORCUS_ASCII("invariant"), character_set_t::invariant },
    { ORCUS_ASCII("iso-10646-j-1"), character_set_t::iso_10646_j_1 },
    { ORCUS_ASCII("iso-10646-ucs-2"), character_set_t::iso_10646_ucs_2 },
    { ORCUS_ASCII("iso-10646-ucs-4"), character_set_t::iso_10646_ucs_4 },
    { ORCUS_ASCII("iso-10646-ucs-basic"), character_set_t::iso_10646_ucs_basic },
    { ORCUS_ASCII("iso-10646-unicode-latin1"), character_set_t::iso_10646_unicode_latin1 },
    { ORCUS_ASCII("iso-10646-utf-1"), character_set_t::iso_10646_utf_1 },
    { ORCUS_ASCII("iso-11548-1"), character_set_t::iso_11548_1 },
    { ORCUS_ASCII("iso-2022-cn"), character_set_t::iso_2022_cn },
    { ORCUS_ASCII("iso-2022-cn-ext"), character_set_t::iso_2022_cn_ext },
    { ORCUS_ASCII("iso-2022-jp"), character_set_t::iso_2022_jp },
    { ORCUS_ASCII("iso-2022-jp-2"), character_set_t::iso_2022_jp_2 },
    { ORCUS_ASCII("iso-2022-kr"), character_set_t::iso_2022_kr },
    { ORCUS_ASCII("iso-8859-1-windows-3.0-latin-1"), character_set_t::iso_8859_1_windows_3_0_latin_1 },
    { ORCUS_ASCII("iso-8859-1-windows-3.1-latin-1"), character_set_t::iso_8859_1_windows_3_1_latin_1 },
    { ORCUS_ASCII("iso-8859-10"), character_set_t::iso_8859_10 },
    { ORCUS_ASCII("iso-8859-13"), character_set_t::iso_8859_13 },
    { ORCUS_ASCII("iso-8859-14"), character_set_t::iso_8859_14 },
    { ORCUS_ASCII("iso-8859-15"), character_set_t::iso_8859_15 },
    { ORCUS_ASCII("iso-8859-16"), character_set_t::iso_8859_16 },
    { ORCUS_ASCII("iso-8859-2-windows-latin-2"), character_set_t::iso_8859_2_windows_latin_2 },
    { ORCUS_ASCII("iso-8859-9-windows-latin-5"), character_set_t::iso_8859_9_windows_latin_5 },
    { ORCUS_ASCII("iso-ir-90"), character_set_t::iso_ir_90 },
    { ORCUS_ASCII("iso-unicode-ibm-1261"), character_set_t::iso_unicode_ibm_1261 },
    { ORCUS_ASCII("iso-unicode-ibm-1264"), character_set_t::iso_unicode_ibm_1264 },
    { ORCUS_ASCII("iso-unicode-ibm-1265"), character_set_t::iso_unicode_ibm_1265 },
    { ORCUS_ASCII("iso-unicode-ibm-1268"), character_set_t::iso_unicode_ibm_1268 },
    { ORCUS_ASCII("iso-unicode-ibm-1276"), character_set_t::iso_unicode_ibm_1276 },
    { ORCUS_ASCII("iso_10367-box"), character_set_t::iso_10367_box },
    { ORCUS_ASCII("iso_2033-1983"), character_set_t::iso_2033_1983 },
    { ORCUS_ASCII("iso_5427"), character_set_t::iso_5427 },
    { ORCUS_ASCII("iso_5427:1981"), character_set_t::iso_5427_1981 },
    { ORCUS_ASCII("iso_5428:1980"), character_set_t::iso_5428_1980 },
    { ORCUS_ASCII("iso_646.basic:1983"), character_set_t::iso_646_basic_1983 },
    { ORCUS_ASCII("iso_646.irv:1983"), character_set_t::iso_646_irv_1983 },
    { ORCUS_ASCII("iso_6937-2-25"), character_set_t::iso_6937_2_25 },
    { ORCUS_ASCII("iso_6937-2-add"), character_set_t::iso_6937_2_add },
    { ORCUS_ASCII("iso_8859-1:1987"), character_set_t::iso_8859_1_1987 },
    { ORCUS_ASCII("iso_8859-2:1987"), character_set_t::iso_8859_2_1987 },
    { ORCUS_ASCII("iso_8859-3:1988"), character_set_t::iso_8859_3_1988 },
    { ORCUS_ASCII("iso_8859-4:1988"), character_set_t::iso_8859_4_1988 },
    { ORCUS_ASCII("iso_8859-5:1988"), character_set_t::iso_8859_5_1988 },
    { ORCUS_ASCII("iso_8859-6-e"), character_set_t::iso_8859_6_e },
    { ORCUS_ASCII("iso_8859-6-i"), character_set_t::iso_8859_6_i },
    { ORCUS_ASCII("iso_8859-6:1987"), character_set_t::iso_8859_6_1987 },
    { ORCUS_ASCII("iso_8859-7:1987"), character_set_t::iso_8859_7_1987 },
    { ORCUS_ASCII("iso_8859-8-e"), character_set_t::iso_8859_8_e },
    { ORCUS_ASCII("iso_8859-8-i"), character_set_t::iso_8859_8_i },
    { ORCUS_ASCII("iso_8859-8:1988"), character_set_t::iso_8859_8_1988 },
    { ORCUS_ASCII("iso_8859-9:1989"), character_set_t::iso_8859_9_1989 },
    { ORCUS_ASCII("iso_8859-supp"), character_set_t::iso_8859_supp },
    { ORCUS_ASCII("it"), character_set_t::it },
    { ORCUS_ASCII("jis_c6220-1969-jp"), character_set_t::jis_c6220_1969_jp },
    { ORCUS_ASCII("jis_c6220-1969-ro"), character_set_t::jis_c6220_1969_ro },
    { ORCUS_ASCII("jis_c6226-1978"), character_set_t::jis_c6226_1978 },
    { ORCUS_ASCII("jis_c6226-1983"), character_set_t::jis_c6226_1983 },
    { ORCUS_ASCII("jis_c6229-1984-a"), character_set_t::jis_c6229_1984_a },
    { ORCUS_ASCII("jis_c6229-1984-b"), character_set_t::jis_c6229_1984_b },
    { ORCUS_ASCII("jis_c6229-1984-b-add"), character_set_t::jis_c6229_1984_b_add },
    { ORCUS_ASCII("jis_c6229-1984-hand"), character_set_t::jis_c6229_1984_hand },
    { ORCUS_ASCII("jis_c6229-1984-hand-add"), character_set_t::jis_c6229_1984_hand_add },
    { ORCUS_ASCII("jis_c6229-1984-kana"), character_set_t::jis_c6229_1984_kana },
    { ORCUS_ASCII("jis_encoding"), character_set_t::jis_encoding },
    { ORCUS_ASCII("jis_x0201"), character_set_t::jis_x0201 },
    { ORCUS_ASCII("jis_x0212-1990"), character_set_t::jis_x0212_1990 },
    { ORCUS_ASCII("jus_i.b1.002"), character_set_t::jus_i_b1_002 },
    { ORCUS_ASCII("jus_i.b1.003-mac"), character_set_t::jus_i_b1_003_mac },
    { ORCUS_ASCII("jus_i.b1.003-serb"), character_set_t::jus_i_b1_003_serb },
    { ORCUS_ASCII("koi7-switched"), character_set_t::koi7_switched },
    { ORCUS_ASCII("koi8-r"), character_set_t::koi8_r },
    { ORCUS_ASCII("koi8-u"), character_set_t::koi8_u },
    { ORCUS_ASCII("ks_c_5601-1987"), character_set_t::ks_c_5601_1987 },
    { ORCUS_ASCII("ksc5636"), character_set_t::ksc5636 },
    { ORCUS_ASCII("kz-1048"), character_set_t::kz_1048 },
    { ORCUS_ASCII("latin-greek"), character_set_t::latin_greek },
    { ORCUS_ASCII("latin-greek-1"), character_set_t::latin_greek_1 },
    { ORCUS_ASCII("latin-lap"), character_set_t::latin_lap },
    { ORCUS_ASCII("macintosh"), character_set_t::macintosh },
    { ORCUS_ASCII("microsoft-publishing"), character_set_t::microsoft_publishing },
    { ORCUS_ASCII("mnem"), character_set_t::mnem },
    { ORCUS_ASCII("mnemonic"), character_set_t::mnemonic },
    { ORCUS_ASCII("msz_7795.3"), character_set_t::msz_7795_3 },
    { ORCUS_ASCII("nats-dano"), character_set_t::nats_dano },
    { ORCUS_ASCII("nats-dano-add"), character_set_t::nats_dano_add },
    { ORCUS_ASCII("nats-sefi"), character_set_t::nats_sefi },
    { ORCUS_ASCII("nats-sefi-add"), character_set_t::nats_sefi_add },
    { ORCUS_ASCII("nc_nc00-10:81"), character_set_t::nc_nc00_10_81 },
    { ORCUS_ASCII("nf_z_62-010"), character_set_t::nf_z_62_010 },
    { ORCUS_ASCII("nf_z_62-010_(1973)"), character_set_t::nf_z_62_010_1973 },
    { ORCUS_ASCII("ns_4551-1"), character_set_t::ns_4551_1 },
    { ORCUS_ASCII("ns_4551-2"), character_set_t::ns_4551_2 },
    { ORCUS_ASCII("osd_ebcdic_df03_irv"), character_set_t::osd_ebcdic_df03_irv },
    { ORCUS_ASCII("osd_ebcdic_df04_1"), character_set_t::osd_ebcdic_df04_1 },
    { ORCUS_ASCII("osd_ebcdic_df04_15"), character_set_t::osd_ebcdic_df04_15 },
    { ORCUS_ASCII("pc8-danish-norwegian"), character_set_t::pc8_danish_norwegian },
    { ORCUS_ASCII("pc8-turkish"), character_set_t::pc8_turkish },
    { ORCUS_ASCII("pt"), character_set_t::pt },
    { ORCUS_ASCII("pt2"), character_set_t::pt2 },
    { ORCUS_ASCII("ptcp154"), character_set_t::ptcp154 },
    { ORCUS_ASCII("scsu"), character_set_t::scsu },
    { ORCUS_ASCII("sen_850200_b"), character_set_t::sen_850200_b },
    { ORCUS_ASCII("sen_850200_c"), character_set_t::sen_850200_c },
    { ORCUS_ASCII("shift_jis"), character_set_t::shift_jis },
    { ORCUS_ASCII("t.101-g2"), character_set_t::t_101_g2 },
    { ORCUS_ASCII("t.61-7bit"), character_set_t::t_61_7bit },
    { ORCUS_ASCII("t.61-8bit"), character_set_t::t_61_8bit },
    { ORCUS_ASCII("tis-620"), character_set_t::tis_620 },
    { ORCUS_ASCII("tscii"), character_set_t::tscii },
    { ORCUS_ASCII("unicode-1-1"), character_set_t::unicode_1_1 },
    { ORCUS_ASCII("unicode-1-1-utf-7"), character_set_t::unicode_1_1_utf_7 },
    { ORCUS_ASCII("unknown-8bit"), character_set_t::unknown_8bit },
    { ORCUS_ASCII("us-ascii"), character_set_t::us_ascii },
    { ORCUS_ASCII("us-dk"), character_set_t::us_dk },
    { ORCUS_ASCII("utf-16"), character_set_t::utf_16 },
    { ORCUS_ASCII("utf-16be"), character_set_t::utf_16be },
    { ORCUS_ASCII("utf-16le"), character_set_t::utf_16le },
    { ORCUS_ASCII("utf-32"), character_set_t::utf_32 },
    { ORCUS_ASCII("utf-32be"), character_set_t::utf_32be },
    { ORCUS_ASCII("utf-32le"), character_set_t::utf_32le },
    { ORCUS_ASCII("utf-7"), character_set_t::utf_7 },
    { ORCUS_ASCII("utf-8"), character_set_t::utf_8 },
    { ORCUS_ASCII("ventura-international"), character_set_t::ventura_international },
    { ORCUS_ASCII("ventura-math"), character_set_t::ventura_math },
    { ORCUS_ASCII("ventura-us"), character_set_t::ventura_us },
    { ORCUS_ASCII("videotex-suppl"), character_set_t::videotex_suppl },
    { ORCUS_ASCII("viqr"), character_set_t::viqr },
    { ORCUS_ASCII("viscii"), character_set_t::viscii },
    { ORCUS_ASCII("windows-1250"), character_set_t::windows_1250 },
    { ORCUS_ASCII("windows-1251"), character_set_t::windows_1251 },
    { ORCUS_ASCII("windows-1252"), character_set_t::windows_1252 },
    { ORCUS_ASCII("windows-1253"), character_set_t::windows_1253 },
    { ORCUS_ASCII("windows-1254"), character_set_t::windows_1254 },
    { ORCUS_ASCII("windows-1255"), character_set_t::windows_1255 },
    { ORCUS_ASCII("windows-1256"), character_set_t::windows_1256 },
    { ORCUS_ASCII("windows-1257"), character_set_t::windows_1257 },
    { ORCUS_ASCII("windows-1258"), character_set_t::windows_1258 },
    { ORCUS_ASCII("windows-31j"), character_set_t::windows_31j },
    { ORCUS_ASCII("windows-874"), character_set_t::windows_874 },
};

const map_type& get()
{
    static map_type mt(entries.data(), entries.size(), character_set_t::unspecified);
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

            m_declaration.encoding = encoding::get().find(val_lower.data(), val_lower.size());
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
