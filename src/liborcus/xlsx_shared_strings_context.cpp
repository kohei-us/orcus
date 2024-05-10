/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xlsx_shared_strings_context.hpp"
#include "ooxml_token_constants.hpp"
#include "ooxml_namespace_types.hpp"
#include "xlsx_helper.hpp"
#include "xml_context_global.hpp"

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/measurement.hpp>

#include <optional>

namespace ss = orcus::spreadsheet;

namespace orcus {

xlsx_shared_strings_context::xlsx_shared_strings_context(
    session_context& session_cxt, const tokens& tokens, spreadsheet::iface::import_shared_strings* strings) :
    xml_context_base(session_cxt, tokens), mp_strings(strings), m_in_segments(false)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_ooxml_xlsx, XML_sst }, // root element
        { NS_ooxml_xlsx, XML_r, NS_ooxml_xlsx, XML_rPr },
        { NS_ooxml_xlsx, XML_r, NS_ooxml_xlsx, XML_t },
        { NS_ooxml_xlsx, XML_rPh, NS_ooxml_xlsx, XML_t },
        { NS_ooxml_xlsx, XML_rPr, NS_ooxml_xlsx, XML_b },
        { NS_ooxml_xlsx, XML_rPr, NS_ooxml_xlsx, XML_color },
        { NS_ooxml_xlsx, XML_rPr, NS_ooxml_xlsx, XML_family },
        { NS_ooxml_xlsx, XML_rPr, NS_ooxml_xlsx, XML_i },
        { NS_ooxml_xlsx, XML_rPr, NS_ooxml_xlsx, XML_rFont },
        { NS_ooxml_xlsx, XML_rPr, NS_ooxml_xlsx, XML_scheme },
        { NS_ooxml_xlsx, XML_rPr, NS_ooxml_xlsx, XML_sz },
        { NS_ooxml_xlsx, XML_rPr, NS_ooxml_xlsx, XML_vertAlign },
        { NS_ooxml_xlsx, XML_si, NS_ooxml_xlsx, XML_r },
        { NS_ooxml_xlsx, XML_si, NS_ooxml_xlsx, XML_t },
        { NS_ooxml_xlsx, XML_sst, NS_ooxml_xlsx, XML_si },
    };

    init_element_validator(rules, std::size(rules));
}

xlsx_shared_strings_context::~xlsx_shared_strings_context() = default;

void xlsx_shared_strings_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
{
    push_stack(ns, name);

    if (ns == NS_ooxml_xlsx)
    {
        switch (name)
        {
            case XML_sst:
            {
                // root element for the shared string part.
                long count = -1;
                long unique_count = -1;

                for (const auto& attr : attrs)
                {
                    switch (attr.name)
                    {
                        case XML_count:
                            count = to_long(attr.value);
                            break;
                        case XML_uniqueCount:
                            unique_count = to_long(attr.value);
                            break;
                    }
                }

                if (get_config().debug)
                    std::cout << "count: " << count << "  unique count: " << unique_count << std::endl;

                break;
            }
            case XML_si:
                // single shared string entry.
                m_in_segments = false;
                break;
            case XML_r:
                // rich text run
                m_in_segments = true;
            case XML_rPr:
                // rich text run property
                break;
            case XML_b:
                // bold
                break;
            case XML_i:
                // italic
                break;
            case XML_sz:
            {
                // font size
                std::string_view s = for_each(attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_val)).get_value();
                double point = to_double(s);
                mp_strings->set_segment_font_size(point);
                break;
            }
            case XML_color:
            {
                // font color
                std::optional<std::string_view> rgb;

                for (const xml_token_attr_t& attr : attrs)
                {
                    switch (attr.name)
                    {
                        case XML_rgb:
                            rgb = attr.value;
                            break;
                        case XML_theme:
                            // TODO : handle this.
                            break;
                    }
                }

                if (rgb)
                {
                    ss::color_elem_t alpha;
                    ss::color_elem_t red;
                    ss::color_elem_t green;
                    ss::color_elem_t blue;
                    if (to_rgb(*rgb, alpha, red, green, blue))
                        mp_strings->set_segment_font_color(alpha, red, green, blue);
                }
                break;
            }
            case XML_rFont:
            {
                // font
                std::string_view font = for_each(attrs.begin(), attrs.end(), single_attr_getter(m_pool, NS_ooxml_xlsx, XML_val)).get_value();
                mp_strings->set_segment_font_name(font);
                break;
            }
            case XML_family:
                // font family
                break;
            case XML_scheme:
                // font scheme
                break;
            case XML_t:
            {
                // actual text stored as its content.
                break;
            }
            case XML_vertAlign:
            {
                for (const auto& attr : attrs)
                {
                    if (attr.name == XML_val)
                    {
                        if (attr.value == "superscript")
                            mp_strings->set_segment_superscript(true);
                        else if (attr.value == "subscript")
                            mp_strings->set_segment_subscript(true);
                    }
                }
                break;
            }
            default:
                warn_unhandled();
        }
    }
}

bool xlsx_shared_strings_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_ooxml_xlsx)
    {
        switch (name)
        {
            case XML_t:
                break;
            case XML_b:
                mp_strings->set_segment_bold(true);
                break;
            case XML_i:
                mp_strings->set_segment_italic(true);
                break;
            case XML_r:
                mp_strings->append_segment(m_cur_str);
                break;
            case XML_si:
            {
                if (m_in_segments)
                    // commit all formatted segments.
                    mp_strings->commit_segments();
                else
                {
                    // unformatted text should only have one text segment.
                    mp_strings->append(m_cur_str);
                }
                break;
            }
        }
    }

    return pop_stack(ns, name);
}

void xlsx_shared_strings_context::characters(std::string_view str, bool transient)
{
    xml_token_pair_t cur_token = get_current_element();
    if (cur_token.first == NS_ooxml_xlsx && cur_token.second == XML_t)
    {
        m_cur_str = str;

        // In case the string contains carriage returns (CRs), remove them.
        m_cell_buffer.reset();
        const char* p = m_cur_str.data();
        const char* p_end = p + m_cur_str.size();
        const char* p0 = nullptr;

        for (; p != p_end; ++p)
        {
            if (!p0)
                p0 = p;

            if (*p == 0x0D)
            {
                // Append the segment up to this CR, and skip the CR.
                m_cell_buffer.append(p0, std::distance(p0, p));
                p0 = nullptr;
            }
        }

        if (!m_cell_buffer.empty())
        {
            // This string contains at least one CR.

            if (p0)
                // Append the tail end.
                m_cell_buffer.append(p0, std::distance(p0, p));

            m_cur_str = m_pool.intern(m_cell_buffer.str()).first;
            transient = false;
        }

        if (transient)
            m_cur_str = m_pool.intern(m_cur_str).first;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
