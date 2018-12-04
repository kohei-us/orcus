/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "html_dumper.hpp"
#include "impl_types.hpp"

#include "orcus/spreadsheet/styles.hpp"
#include "orcus/spreadsheet/shared_strings.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/sheet.hpp"
#include "orcus/global.hpp"

#include <ixion/address.hpp>
#include <ixion/model_context.hpp>
#include <ixion/formula.hpp>
#include <ixion/formula_result.hpp>

#include <sstream>

namespace orcus { namespace spreadsheet { namespace detail {

namespace {

void build_rgb_color(std::ostringstream& os, const color_t& color_value)
{
    // Special colors.
    if (color_value.alpha == 255 && color_value.red == 0 && color_value.green == 0 && color_value.blue == 0)
    {
        os << "black";
        return;
    }

    if (color_value.alpha == 255 && color_value.red == 255 && color_value.green == 0 && color_value.blue == 0)
    {
        os << "red";
        return;
    }

    if (color_value.alpha == 255 && color_value.red == 0 && color_value.green == 255 && color_value.blue == 0)
    {
        os << "green";
        return;
    }

    if (color_value.alpha == 255 && color_value.red == 0 && color_value.green == 0 && color_value.blue == 255)
    {
        os << "blue";
        return;
    }

    os << "rgb("
        << static_cast<short>(color_value.red) << ","
        << static_cast<short>(color_value.green) << ","
        << static_cast<short>(color_value.blue) << ")";
}

const char* css_style_global =
"table, td { "
    "border-collapse : collapse; "
"}\n"

"table { "
    "border-spacing : 0px; "
"}\n"

"td { "
    "width : 1in; "
"}\n"

"td.empty { "
    "color : white; "
"}\n";

class html_elem
{
public:
    struct attr
    {
        std::string name;
        std::string value;

        attr(const std::string& _name, const std::string& _value) : name(_name), value(_value) {}
    };

    typedef std::vector<attr> attrs_type;

    html_elem(std::ostream& strm, const char* name, const char* style = nullptr, const char* style_class = nullptr) :
        m_strm(strm), m_name(name)
    {
        m_strm << '<' << m_name;

        if (style)
            m_strm << " style=\"" << style << "\"";

        if (style_class)
            m_strm << " class=\"" << style_class << "\"";

        m_strm << '>';
    }

    html_elem(std::ostream& strm, const char* name, const attrs_type& attrs) :
        m_strm(strm), m_name(name)
    {
        m_strm << '<' << m_name;

        attrs_type::const_iterator it = attrs.begin(), it_end = attrs.end();
        for (; it != it_end; ++it)
            m_strm << " " << it->name << "=\"" << it->value << "\"";

        m_strm << '>';
    }

    ~html_elem()
    {
        m_strm << "</" << m_name << '>';
    }

private:
    std::ostream& m_strm;
    const char* m_name;
};

void print_formatted_text(std::ostream& strm, const std::string& text, const format_runs_t& formats)
{
    typedef html_elem elem;

    const char* p_span = "span";

    size_t pos = 0;
    format_runs_t::const_iterator itr = formats.begin(), itr_end = formats.end();
    for (; itr != itr_end; ++itr)
    {
        const format_run& run = *itr;
        if (pos < run.pos)
        {
            // flush unformatted text.
            strm << std::string(&text[pos], run.pos-pos);
            pos = run.pos;
        }

        if (!run.size)
            continue;

        std::string style = "";
        if (run.bold)
            style += "font-weight: bold;";
        else
            style += "font-weight: normal;";

        if (run.italic)
            style += "font-style: italic;";
        else
            style += "font-style: normal;";

        if (!run.font.empty())
            style += "font-family: " + run.font.str() + ";";

        if (run.font_size)
        {
            std::ostringstream os;
            os << "font-size: " << run.font_size << "pt;";
            style += os.str();
        }

        const color_t& col = run.color;
        if (col.red || col.green || col.blue)
        {
            std::ostringstream os;
            os << "color: ";
            build_rgb_color(os, col);
            os << ";";
            style += os.str();
        }

        if (style.empty())
            strm << std::string(&text[pos], run.size);
        else
        {
            elem span(strm, p_span, style.c_str());
            strm << std::string(&text[pos], run.size);
        }

        pos += run.size;
    }

    if (pos < text.size())
    {
        // flush the remaining unformatted text.
        strm << std::string(&text[pos], text.size() - pos);
    }
}

void build_border_style(std::ostringstream& os, const char* style_name, const border_attrs_t& attrs)
{
    os << style_name << ": ";
    if (attrs.style == border_style_t::thin)
    {
        os << "solid 1px ";
    }
    else if (attrs.style == border_style_t::medium)
    {
        os << "solid 2px ";
    }
    else if (attrs.style == border_style_t::thick)
    {
        os << "solid 3px ";
    }
    else if (attrs.style == border_style_t::hair)
    {
        os << "solid 0.5px ";
    }
    else if (attrs.style == border_style_t::dotted)
    {
        os << "dotted 1px ";
    }
    else if (attrs.style == border_style_t::dashed)
    {
        os << "dashed 1px ";
    }
    else if (attrs.style == border_style_t::double_border)
    {
        os << "3px double ";
    }
    else if (attrs.style == border_style_t::dash_dot)
    {
        // CSS doesn't support dash-dot.
        os << "dashed 1px ";
    }
    else if (attrs.style == border_style_t::dash_dot_dot)
    {
        // CSS doesn't support dash-dot-dot.
        os << "dashed 1px ";
    }
    else if (attrs.style == border_style_t::medium_dashed)
    {
        os << "dashed 2px ";
    }
    else if (attrs.style == border_style_t::medium_dash_dot)
    {
        // CSS doesn't support dash-dot.
        os << "dashed 2px ";
    }
    else if (attrs.style == border_style_t::medium_dash_dot_dot)
    {
        // CSS doesn't support dash-dot-dot.
        os << "dashed 2px ";
    }
    else if (attrs.style == border_style_t::slant_dash_dot)
    {
        // CSS doesn't support dash-dot.
        os << "dashed 2px ";
    }

    build_rgb_color(os, attrs.border_color);
    os << "; ";
}

void build_style_string(std::string& str, const styles& styles, const cell_format_t& fmt)
{
    std::ostringstream os;
    if (fmt.font)
    {
        const font_t* p = styles.get_font(fmt.font);
        if (p)
        {
            if (!p->name.empty())
                os << "font-family: " << p->name << ";";
            if (p->size)
                os << "font-size: " << p->size << "pt;";
            if (p->bold)
                os << "font-weight: bold;";
            if (p->italic)
                os << "font-style: italic;";

            const color_t& r = p->color;
            if (r.red || r.green || r.blue)
            {
                os << "color: ";
                build_rgb_color(os, r);
                os << ";";
            }
        }
    }
    if (fmt.fill)
    {
        const fill_t* p = styles.get_fill(fmt.fill);
        if (p)
        {
            if (p->pattern_type == fill_pattern_t::solid)
            {
                const color_t& r = p->fg_color;
                os << "background-color: ";
                build_rgb_color(os, r);
                os << ";";
            }
        }
    }

    if (fmt.border)
    {
        const border_t* p = styles.get_border(fmt.border);
        if (p)
        {
            build_border_style(os, "border-top", p->top);
            build_border_style(os, "border-bottom", p->bottom);
            build_border_style(os, "border-left", p->left);
            build_border_style(os, "border-right", p->right);
        }
    }

    if (fmt.apply_alignment)
    {
        if (fmt.hor_align != hor_alignment_t::unknown)
        {
            os << "text-align: ";
            switch (fmt.hor_align)
            {
                case hor_alignment_t::left:
                    os << "left";
                break;
                case hor_alignment_t::center:
                    os << "center";
                break;
                case hor_alignment_t::right:
                    os << "right";
                break;
                default:
                    ;
            }
            os << ";";
        }

        if (fmt.ver_align != ver_alignment_t::unknown)
        {
            os << "vertical-align: ";
            switch (fmt.ver_align)
            {
                case ver_alignment_t::top:
                    os << "top";
                break;
                case ver_alignment_t::middle:
                    os << "middle";
                break;
                case ver_alignment_t::bottom:
                    os << "bottom";
                break;
                default:
                    ;
            }
            os << ";";
        }
    }

    str += os.str();
}

void dump_html_head(std::ostream& os)
{
    typedef html_elem elem;

    const char* p_head = "head";
    const char* p_style = "style";

    elem elem_head(os, p_head);
    {
        elem elem_style(os, p_style);
        os << css_style_global;
    }
}

void build_html_elem_attributes(html_elem::attrs_type& attrs, const std::string& style, const merge_size* p_merge_size)
{
    attrs.push_back(html_elem::attr("style", style));
    if (p_merge_size)
    {
        if (p_merge_size->width > 1)
        {
            std::ostringstream os2;
            os2 << p_merge_size->width;
            attrs.push_back(html_elem::attr("colspan", os2.str()));
        }

        if (p_merge_size->height > 1)
        {
            std::ostringstream os2;
            os2 << p_merge_size->height;
            attrs.push_back(html_elem::attr("rowspan", os2.str()));
        }
    }
}

}

html_dumper::html_dumper(
    const document& doc,
    const col_merge_size_type& merge_ranges,
    sheet_t sheet_id) :
    m_doc(doc),
    m_merge_ranges(merge_ranges)
{
    build_overlapped_ranges(sheet_id);
}

void html_dumper::dump(std::ostream& os, ixion::sheet_t sheet_id) const
{

    typedef html_elem elem;

    const char* p_html  = "html";
    const char* p_body  = "body";
    const char* p_table = "table";
    const char* p_tr    = "tr";
    const char* p_td    = "td";

    const sheet* sh = m_doc.get_sheet(sheet_id);
    if (!sh)
        return;

    ixion::abs_range_t range = sh->get_data_range();

    elem root(os, p_html);
    dump_html_head(os);

    {
        elem elem_body(os, p_body);

        if (!range.valid())
            // Sheet is empty.  Nothing to print.
            return;

        const ixion::model_context& cxt = m_doc.get_model_context();
        const ixion::formula_name_resolver* resolver = m_doc.get_formula_name_resolver();
        const import_shared_strings* sstrings = m_doc.get_shared_strings();

        elem table(os, p_table);

        row_t row_count = range.last.row + 1;
        col_t col_count = range.last.column + 1;
        for (row_t row = 0; row < row_count; ++row)
        {
            // Set the row height.
            std::string row_style;
            row_height_t rh = sh->get_row_height(row, nullptr, nullptr);

            // Convert height from twip to inches.
            if (rh != get_default_row_height())
            {
                std::string style;
                double val = orcus::convert(rh, length_unit_t::twip, length_unit_t::inch);
                std::ostringstream os_style;
                os_style << "height: " << val << "in;";
                row_style += os_style.str();
            }

            const char* style_str = nullptr;
            if (!row_style.empty())
                style_str = row_style.c_str();
            elem tr(os, p_tr, style_str);

            const detail::overlapped_col_index_type* p_overlapped = get_overlapped_ranges(row);

            for (col_t col = 0; col < col_count; ++col)
            {
                ixion::abs_address_t pos(sheet_id, row, col);

                const merge_size* p_merge_size = get_merge_size(row, col);
                if (!p_merge_size && p_overlapped)
                {
                    // Check if this cell is overlapped by a merged cell.
                    bool overlapped = false;
                    col_t last_col;
                    if (p_overlapped->search_tree(col, overlapped, nullptr, &last_col).second && overlapped)
                    {
                        // Skip all overlapped cells on this row.
                        col = last_col - 1;
                        continue;
                    }
                }
                size_t xf_id = sh->get_cell_format(row, col);
                std::string style;

                if (row == 0)
                {
                    // Set the column width.
                    col_width_t cw = sh->get_col_width(col, nullptr, nullptr);

                    // Convert width from twip to inches.
                    if (cw != get_default_column_width())
                    {
                        double val = orcus::convert(cw, length_unit_t::twip, length_unit_t::inch);
                        std::ostringstream os_style;
                        os_style << "width: " << val << "in;";
                        style += os_style.str();
                    }
                }

                if (xf_id)
                {
                    // Apply cell format.
                    const styles& styles = m_doc.get_styles();
                    const cell_format_t* fmt = styles.get_cell_format(xf_id);
                    if (fmt)
                        build_style_string(style, styles, *fmt);
                }

                ixion::celltype_t ct = cxt.get_celltype(pos);
                if (ct == ixion::celltype_t::empty)
                {
                    html_elem::attrs_type attrs;
                    build_html_elem_attributes(attrs, style, p_merge_size);
                    attrs.push_back(html_elem::attr("class", "empty"));
                    elem td(os, p_td, attrs);
                    os << '-'; // empty cell.
                    continue;
                }

                html_elem::attrs_type attrs;
                build_html_elem_attributes(attrs, style, p_merge_size);
                elem td(os, p_td, attrs);

                switch (ct)
                {
                    case ixion::celltype_t::string:
                    {
                        size_t sindex = cxt.get_string_identifier(pos);
                        const std::string* p = cxt.get_string(sindex);
                        assert(p);
                        const format_runs_t* pformat = sstrings->get_format_runs(sindex);
                        if (pformat)
                            print_formatted_text(os, *p, *pformat);
                        else
                            os << *p;

                        break;
                    }
                    case ixion::celltype_t::numeric:
                        os << cxt.get_numeric_value(pos);
                        break;
                    case ixion::celltype_t::boolean:
                        os << (cxt.get_boolean_value(pos) ? "true" : "false");
                        break;
                    case ixion::celltype_t::formula:
                    {
                        // print the formula and the formula result.
                        const ixion::formula_cell* cell = cxt.get_formula_cell(pos);
                        assert(cell);
                        const ixion::formula_tokens_store_ptr_t& ts = cell->get_tokens();
                        if (ts)
                        {
                            const ixion::formula_tokens_t& tokens = ts->get();

                            std::string formula;
                            if (resolver)
                            {
                                pos = cell->get_parent_position(pos);
                                formula = ixion::print_formula_tokens(
                                    m_doc.get_model_context(), pos, *resolver, tokens);
                            }
                            else
                                formula = "???";

                            ixion::formula_group_t fg = cell->get_group_properties();

                            if (fg.grouped)
                                os << '{' << formula << '}';
                            else
                                os << formula;

                            ixion::formula_result res = cell->get_result_cache();
                            os << " (" << res.str(m_doc.get_model_context()) << ")";
                        }

                        break;
                    }
                    default:
                        ;
                }
            }
        }
    }
}

const overlapped_col_index_type* html_dumper::get_overlapped_ranges(row_t row) const
{
    overlapped_cells_type::const_iterator it = m_overlapped_ranges.find(row);
    if (it == m_overlapped_ranges.end())
        return nullptr;

    return it->second.get();
}

const merge_size* html_dumper::get_merge_size(row_t row, col_t col) const
{
    col_merge_size_type::const_iterator it_col = m_merge_ranges.find(col);
    if (it_col == m_merge_ranges.end())
        return nullptr;

    merge_size_type& col_merge_sizes = *it_col->second;
    merge_size_type::const_iterator it_row = col_merge_sizes.find(row);
    if (it_row == col_merge_sizes.end())
        return nullptr;

    return &it_row->second;
}

void html_dumper::build_overlapped_ranges(sheet_t sheet_id)
{
    const sheet* sh = m_doc.get_sheet(sheet_id);
    if (!sh)
        return;

    detail::col_merge_size_type::const_iterator it_col = m_merge_ranges.begin(), it_col_end = m_merge_ranges.end();
    for (; it_col != it_col_end; ++it_col)
    {
        col_t col = it_col->first;
        const detail::merge_size_type& data = *it_col->second;
        detail::merge_size_type::const_iterator it = data.begin(), it_end = data.end();
        for (; it != it_end; ++it)
        {
            row_t row = it->first;
            const detail::merge_size& item = it->second;
            for (row_t i = 0; i < item.height; ++i, ++row)
            {
                // Get the container for this row.
                detail::overlapped_cells_type::iterator it_cont = m_overlapped_ranges.find(row);
                if (it_cont == m_overlapped_ranges.end())
                {
                    auto p = orcus::make_unique<detail::overlapped_col_index_type>(0, sh->col_size(), false);
                    std::pair<detail::overlapped_cells_type::iterator, bool> r =
                        m_overlapped_ranges.insert(detail::overlapped_cells_type::value_type(row, std::move(p)));

                    if (!r.second)
                    {
                        // Insertion failed.
                        return;
                    }

                    it_cont = r.first;
                }

                detail::overlapped_col_index_type& cont = *it_cont->second;
                cont.insert_back(col, col+item.width, true);
            }
        }
    }

    // Build trees.
    for (auto& range : m_overlapped_ranges)
        range.second->build_tree();
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
