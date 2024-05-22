/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/spreadsheet/factory.hpp"
#include "orcus/spreadsheet/styles.hpp"
#include "orcus/string_pool.hpp"

#include "factory_strikethrough.hpp"

#include <unordered_map>
#include <cassert>

namespace orcus { namespace spreadsheet {

namespace {

class import_font_style : public iface::import_font_style
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    import_font_style() = delete;
    import_font_style(styles& _styles_model, string_pool& sp);
    import_font_style(std::shared_ptr<import_factory_config> _config, styles& _styles_model, string_pool& sp);
    virtual ~import_font_style() override;

    virtual void set_bold(bool b) override;
    virtual void set_bold_asian(bool b) override;
    virtual void set_bold_complex(bool b) override;

    virtual void set_italic(bool b) override;
    virtual void set_italic_asian(bool b) override;
    virtual void set_italic_complex(bool b) override;

    virtual void set_name(std::string_view s) override;
    virtual void set_name_asian(std::string_view s) override;
    virtual void set_name_complex(std::string_view s) override;

    virtual void set_size(double point) override;
    virtual void set_size_asian(double point) override;
    virtual void set_size_complex(double point) override;

    virtual void set_underline(underline_t e) override;
    virtual void set_underline_width(underline_width_t e) override;
    virtual void set_underline_mode(underline_mode_t e) override;
    virtual void set_underline_type(underline_count_t e) override;
    virtual void set_underline_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) override;
    virtual void set_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) override;
    virtual iface::import_strikethrough* start_strikethrough() override;
    virtual std::size_t commit() override;

    void reset();
};

class import_fill_style : public iface::import_fill_style
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    import_fill_style() = delete;
    import_fill_style(styles& _styles_model, string_pool& sp);
    virtual ~import_fill_style() override;

    virtual void set_pattern_type(fill_pattern_t fp) override;
    virtual void set_fg_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) override;
    virtual void set_bg_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) override;
    virtual size_t commit() override;

    void reset();
};

class import_border_style : public iface::import_border_style
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    import_border_style() = delete;
    import_border_style(styles& _styles_model, string_pool& sp);
    virtual ~import_border_style() override;

    virtual void set_style(border_direction_t dir, border_style_t style) override;
    virtual void set_color(
        border_direction_t dir, color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) override;
    virtual void set_width(border_direction_t dir, double width, orcus::length_unit_t unit) override;
    virtual size_t commit() override;

    void reset();
};

class import_cell_protection : public iface::import_cell_protection
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    import_cell_protection() = delete;
    import_cell_protection(styles& _styles_model, string_pool& sp);
    virtual ~import_cell_protection() override;

    virtual void set_hidden(bool b) override;
    virtual void set_locked(bool b) override;
    virtual void set_print_content(bool b) override;
    virtual void set_formula_hidden(bool b) override;
    virtual size_t commit() override;

    void reset();
};

class import_number_format : public iface::import_number_format
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    import_number_format() = delete;
    import_number_format(styles& _styles_model, string_pool& sp);
    virtual ~import_number_format() override;

    virtual void set_identifier(std::size_t id) override;
    virtual void set_code(std::string_view s) override;
    virtual size_t commit() override;

    void reset();
};

class import_xf : public iface::import_xf
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    import_xf() = delete;
    import_xf(styles& _styles_model, string_pool& sp);
    virtual ~import_xf() override;

    virtual void set_font(size_t index) override;
    virtual void set_fill(size_t index) override;
    virtual void set_border(size_t index) override;
    virtual void set_protection(size_t index) override;
    virtual void set_number_format(size_t index) override;
    virtual void set_style_xf(size_t index) override;
    virtual void set_apply_alignment(bool b) override;
    virtual void set_horizontal_alignment(hor_alignment_t align) override;
    virtual void set_vertical_alignment(ver_alignment_t align) override;
    virtual void set_wrap_text(bool b) override;
    virtual void set_shrink_to_fit(bool b) override;
    virtual size_t commit() override;

    void reset(xf_category_t cat);
};

class import_cell_style : public iface::import_cell_style
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    import_cell_style() = delete;
    import_cell_style(styles& _styles_model, string_pool& sp);
    virtual ~import_cell_style() override;

    void set_name(std::string_view s) override;
    void set_display_name(std::string_view s) override;
    void set_xf(size_t index) override;
    void set_builtin(size_t index) override;
    void set_parent_name(std::string_view s) override;
    void commit() override;

    void reset();
};

struct import_font_style::impl
{
    std::shared_ptr<import_factory_config> config;
    styles& styles_model;
    string_pool& str_pool;

    std::unordered_map<font_t, std::size_t, font_t::hash> font_cache;
    font_t cur_font;
    detail::import_strikethrough strikethrough_import;

    impl(styles& _styles_model, string_pool& sp) :
        config(std::make_shared<import_factory_config>()),
        styles_model(_styles_model),
        str_pool(sp),
        strikethrough_import() {}

    impl(std::shared_ptr<import_factory_config> _config, styles& _styles_model, string_pool& sp) :
        config(_config),
        styles_model(_styles_model),
        str_pool(sp),
        strikethrough_import() {}
};

import_font_style::import_font_style(styles& _styles_model, string_pool& sp) :
    mp_impl(std::make_unique<impl>(_styles_model, sp))
{
}

import_font_style::import_font_style(
    std::shared_ptr<import_factory_config> _config, styles& _styles_model, string_pool& sp) :
    mp_impl(std::make_unique<impl>(_config, _styles_model, sp))
{
}

import_font_style::~import_font_style() = default;

void import_font_style::set_bold(bool b)
{
    mp_impl->cur_font.bold = b;
}

void import_font_style::set_bold_asian(bool b)
{
    mp_impl->cur_font.bold_asian = b;
}

void import_font_style::set_bold_complex(bool b)
{
    mp_impl->cur_font.bold_complex = b;
}

void import_font_style::set_italic(bool b)
{
    mp_impl->cur_font.italic = b;
}

void import_font_style::set_italic_asian(bool b)
{
    mp_impl->cur_font.italic_asian = b;
}

void import_font_style::set_italic_complex(bool b)
{
    mp_impl->cur_font.italic_complex = b;
}

void import_font_style::set_name(std::string_view s)
{
    mp_impl->cur_font.name = mp_impl->str_pool.intern(s).first;
}

void import_font_style::set_name_asian(std::string_view s)
{
    mp_impl->cur_font.name_asian = mp_impl->str_pool.intern(s).first;
}

void import_font_style::set_name_complex(std::string_view s)
{
    mp_impl->cur_font.name_complex = mp_impl->str_pool.intern(s).first;
}

void import_font_style::set_size(double point)
{
    mp_impl->cur_font.size = point;
}

void import_font_style::set_size_asian(double point)
{
    mp_impl->cur_font.size_asian = point;
}

void import_font_style::set_size_complex(double point)
{
    mp_impl->cur_font.size_complex = point;
}

void import_font_style::set_underline(underline_t e)
{
    mp_impl->cur_font.underline_style = e;
}

void import_font_style::set_underline_width(underline_width_t e)
{
    mp_impl->cur_font.underline_width = e;
}

void import_font_style::set_underline_mode(underline_mode_t e)
{
    mp_impl->cur_font.underline_mode = e;
}

void import_font_style::set_underline_type(underline_count_t e)
{
    mp_impl->cur_font.underline_type = e;
}

void import_font_style::set_underline_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    mp_impl->cur_font.underline_color = color_t(alpha, red, green, blue);
}

void import_font_style::set_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    mp_impl->cur_font.color = color_t(alpha, red, green, blue);
}

iface::import_strikethrough* import_font_style::start_strikethrough()
{
    mp_impl->strikethrough_import.reset(&mp_impl->cur_font.strikethrough);
    return &mp_impl->strikethrough_import;
}

std::size_t import_font_style::commit()
{
    if (mp_impl->config->enable_font_cache)
    {
        auto it = mp_impl->font_cache.find(mp_impl->cur_font);
        if (it != mp_impl->font_cache.end())
            return it->second;
    }

    std::size_t font_id = mp_impl->styles_model.append_font(mp_impl->cur_font);
    mp_impl->font_cache.insert({mp_impl->cur_font, font_id});
    mp_impl->cur_font.reset();
    return font_id;
}

void import_font_style::reset()
{
    mp_impl->cur_font.reset();
}

struct import_fill_style::impl
{
    styles& styles_model;
    string_pool& str_pool;

    fill_t cur_fill;

    impl(styles& _styles_model, string_pool& sp) :
        styles_model(_styles_model), str_pool(sp) {}
};

import_fill_style::import_fill_style(styles& _styles_model, string_pool& sp) :
    mp_impl(std::make_unique<impl>(_styles_model, sp))
{
}

import_fill_style::~import_fill_style()
{
}

void import_fill_style::set_pattern_type(fill_pattern_t fp)
{
    mp_impl->cur_fill.pattern_type = fp;
}

void import_fill_style::set_fg_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    mp_impl->cur_fill.fg_color = color_t{alpha, red, green, blue};
}

void import_fill_style::set_bg_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    mp_impl->cur_fill.bg_color = color_t{alpha, red, green, blue};
}

size_t import_fill_style::commit()
{
    size_t fill_id = mp_impl->styles_model.append_fill(mp_impl->cur_fill);
    mp_impl->cur_fill.reset();
    return fill_id;
}

void import_fill_style::reset()
{
    mp_impl->cur_fill.reset();
}

struct import_border_style::impl
{
    styles& styles_model;
    string_pool& str_pool;

    border_t cur_border;

    impl(styles& _styles_model, string_pool& sp) :
        styles_model(_styles_model), str_pool(sp) {}

    border_attrs_t* get_border_attrs(border_direction_t dir)
    {
        switch (dir)
        {
            case border_direction_t::top:
                return &cur_border.top;
            case border_direction_t::bottom:
                return &cur_border.bottom;
            case border_direction_t::left:
                return &cur_border.left;
            case border_direction_t::right:
                return &cur_border.right;
            case border_direction_t::diagonal:
                return &cur_border.diagonal;
            case border_direction_t::diagonal_bl_tr:
                return &cur_border.diagonal_bl_tr;
            case border_direction_t::diagonal_tl_br:
                return &cur_border.diagonal_tl_br;
            case border_direction_t::unknown:
                ;
        }

        return nullptr;
    }
};

import_border_style::import_border_style(styles& _styles_model, string_pool& sp) :
    mp_impl(std::make_unique<impl>(_styles_model, sp))
{
}

import_border_style::~import_border_style()
{
}

void import_border_style::set_style(border_direction_t dir, border_style_t style)
{
    border_attrs_t* v = mp_impl->get_border_attrs(dir);
    if (!v)
        return;

    v->style = style;
}

void import_border_style::set_color(
    border_direction_t dir, color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    border_attrs_t* v = mp_impl->get_border_attrs(dir);
    if (!v)
        return;

    v->border_color = color_t(alpha, red, green, blue);
}

void import_border_style::set_width(border_direction_t dir, double width, orcus::length_unit_t unit)
{
    border_attrs_t* v = mp_impl->get_border_attrs(dir);
    if (!v)
        return;

    length_t bw{unit, width};
    v->border_width = bw;
}

size_t import_border_style::commit()
{
    size_t border_id = mp_impl->styles_model.append_border(mp_impl->cur_border);
    mp_impl->cur_border.reset();
    return border_id;
}

void import_border_style::reset()
{
    mp_impl->cur_border.reset();
}

struct import_cell_protection::impl
{
    styles& styles_model;
    string_pool& str_pool;

    protection_t cur_protection;

    impl(styles& _styles_model, string_pool& sp) :
        styles_model(_styles_model), str_pool(sp) {}
};

import_cell_protection::import_cell_protection(styles& _styles_model, string_pool& sp) :
    mp_impl(std::make_unique<impl>(_styles_model, sp))
{
}

import_cell_protection::~import_cell_protection()
{
}

void import_cell_protection::set_hidden(bool b)
{
    mp_impl->cur_protection.hidden = b;
}

void import_cell_protection::set_locked(bool b)
{
    mp_impl->cur_protection.locked = b;
}

void import_cell_protection::set_print_content(bool b)
{
    mp_impl->cur_protection.print_content = b;
}

void import_cell_protection::set_formula_hidden(bool b)
{
    mp_impl->cur_protection.formula_hidden = b;
}

std::size_t import_cell_protection::commit()
{
    std::size_t cp_id = mp_impl->styles_model.append_protection(mp_impl->cur_protection);
    mp_impl->cur_protection.reset();
    return cp_id;
}

void import_cell_protection::reset()
{
    mp_impl->cur_protection.reset();
}

struct import_number_format::impl
{
    styles& styles_model;
    string_pool& str_pool;

    number_format_t cur_numfmt;

    impl(styles& _styles_model, string_pool& sp) :
        styles_model(_styles_model), str_pool(sp) {}
};

import_number_format::import_number_format(styles& _styles_model, string_pool& sp) :
    mp_impl(std::make_unique<impl>(_styles_model, sp))
{
}

import_number_format::~import_number_format()
{
}

void import_number_format::set_identifier(std::size_t id)
{
    mp_impl->cur_numfmt.identifier = id;
}

void import_number_format::set_code(std::string_view s)
{
    mp_impl->cur_numfmt.format_string = s;
}

size_t import_number_format::commit()
{
    std::size_t fmt_id = mp_impl->styles_model.append_number_format(mp_impl->cur_numfmt);
    mp_impl->cur_numfmt.reset();

    return fmt_id;
}

void import_number_format::reset()
{
    mp_impl->cur_numfmt.reset();
}

struct import_xf::impl
{
    styles& styles_model;
    string_pool& str_pool;

    cell_format_t cur_cell_format;
    xf_category_t xf_category = xf_category_t::unknown;

    impl(styles& _styles_model, string_pool& sp) :
        styles_model(_styles_model), str_pool(sp) {}
};

import_xf::import_xf(styles& _styles_model, string_pool& sp) :
    mp_impl(std::make_unique<impl>(_styles_model, sp))
{
}

import_xf::~import_xf()
{
}

void import_xf::set_font(size_t index)
{
    mp_impl->cur_cell_format.font = index;
}

void import_xf::set_fill(size_t index)
{
    mp_impl->cur_cell_format.fill = index;
}

void import_xf::set_border(size_t index)
{
    mp_impl->cur_cell_format.border = index;

    // TODO : we need to decide whether to have interface methods for these
    // apply_foo attributes.  For now there is only one, for alignment.
    mp_impl->cur_cell_format.apply_border = index > 0;
}

void import_xf::set_protection(size_t index)
{
    mp_impl->cur_cell_format.protection = index;
}

void import_xf::set_number_format(size_t index)
{
    mp_impl->cur_cell_format.number_format = index;
}

void import_xf::set_style_xf(size_t index)
{
    mp_impl->cur_cell_format.style_xf = index;
}

void import_xf::set_apply_alignment(bool b)
{
    mp_impl->cur_cell_format.apply_alignment = b;
}

void import_xf::set_horizontal_alignment(hor_alignment_t align)
{
    mp_impl->cur_cell_format.hor_align = align;
}

void import_xf::set_vertical_alignment(ver_alignment_t align)
{
    mp_impl->cur_cell_format.ver_align = align;
}

void import_xf::set_wrap_text(bool b)
{
    mp_impl->cur_cell_format.wrap_text = b;
}

void import_xf::set_shrink_to_fit(bool b)
{
    mp_impl->cur_cell_format.shrink_to_fit = b;
}


size_t import_xf::commit()
{
    size_t xf_id = 0;

    switch (mp_impl->xf_category)
    {
        case xf_category_t::cell:
            xf_id = mp_impl->styles_model.append_cell_format(mp_impl->cur_cell_format);
            break;
        case xf_category_t::cell_style:
            xf_id = mp_impl->styles_model.append_cell_style_format(mp_impl->cur_cell_format);
            break;
        case xf_category_t::differential:
            xf_id = mp_impl->styles_model.append_diff_cell_format(mp_impl->cur_cell_format);
            break;
        case xf_category_t::unknown:
            throw std::logic_error("unknown cell format category");
    }

    mp_impl->cur_cell_format.reset();
    return xf_id;
}

void import_xf::reset(xf_category_t cat)
{
    if (cat == xf_category_t::unknown)
        throw std::invalid_argument("The specified category is 'unknown'.");

    mp_impl->cur_cell_format.reset();
    mp_impl->xf_category = cat;
}

struct import_cell_style::impl
{
    styles& styles_model;
    string_pool& str_pool;

    cell_style_t cur_cell_style;

    impl(styles& _styles_model, string_pool& sp) :
        styles_model(_styles_model), str_pool(sp) {}
};

import_cell_style::import_cell_style(styles& _styles_model, string_pool& sp) :
    mp_impl(std::make_unique<impl>(_styles_model, sp))
{
}

import_cell_style::~import_cell_style() {}

void import_cell_style::set_name(std::string_view s)
{
    mp_impl->cur_cell_style.name = mp_impl->str_pool.intern(s).first;
}

void import_cell_style::set_display_name(std::string_view s)
{
    mp_impl->cur_cell_style.display_name = mp_impl->str_pool.intern(s).first;
}

void import_cell_style::set_xf(size_t index)
{
    mp_impl->cur_cell_style.xf = index;
}

void import_cell_style::set_builtin(size_t index)
{
    mp_impl->cur_cell_style.builtin = index;
}

void import_cell_style::set_parent_name(std::string_view s)
{
    mp_impl->cur_cell_style.parent_name = mp_impl->str_pool.intern(s).first;
}

void import_cell_style::commit()
{
    mp_impl->styles_model.append_cell_style(mp_impl->cur_cell_style);
    mp_impl->cur_cell_style.reset();
}

void import_cell_style::reset()
{
    mp_impl->cur_cell_style.reset();
}

} // anonymous namespace

struct import_styles::impl
{
    styles& styles_model;
    string_pool& str_pool;

    import_font_style font_style;
    import_fill_style fill_style;
    import_border_style border_style;
    import_cell_protection cell_protection;
    import_number_format number_format;
    import_xf xf;
    import_cell_style cell_style;

    impl(styles& _styles_model, string_pool& sp) :
        styles_model(_styles_model),
        str_pool(sp),
        font_style(_styles_model, sp),
        fill_style(_styles_model, sp),
        border_style(_styles_model, sp),
        cell_protection(_styles_model, sp),
        number_format(_styles_model, sp),
        xf(_styles_model, sp),
        cell_style(_styles_model, sp)
    {}

    impl(std::shared_ptr<import_factory_config> config, styles& _styles_model, string_pool& sp) :
        styles_model(_styles_model),
        str_pool(sp),
        font_style(config, _styles_model, sp),
        fill_style(_styles_model, sp),
        border_style(_styles_model, sp),
        cell_protection(_styles_model, sp),
        number_format(_styles_model, sp),
        xf(_styles_model, sp),
        cell_style(_styles_model, sp)
    {}
};

import_styles::import_styles(styles& styles_store, string_pool& sp) :
    mp_impl(std::make_unique<impl>(styles_store, sp)) {}

import_styles::import_styles(std::shared_ptr<import_factory_config> config, styles& styles_store, string_pool& sp) :
    mp_impl(std::make_unique<impl>(config, styles_store, sp)) {}

import_styles::~import_styles() = default;

iface::import_font_style* import_styles::start_font_style()
{
    mp_impl->font_style.reset();
    return &mp_impl->font_style;
}

iface::import_fill_style* import_styles::start_fill_style()
{
    mp_impl->fill_style.reset();
    return &mp_impl->fill_style;
}

iface::import_border_style* import_styles::start_border_style()
{
    mp_impl->border_style.reset();
    return &mp_impl->border_style;
}

iface::import_cell_protection* import_styles::start_cell_protection()
{
    mp_impl->cell_protection.reset();
    return &mp_impl->cell_protection;
}

iface::import_number_format* import_styles::start_number_format()
{
    mp_impl->number_format.reset();
    return &mp_impl->number_format;
}

iface::import_xf* import_styles::start_xf(xf_category_t cat)
{
    mp_impl->xf.reset(cat);
    return &mp_impl->xf;
}

iface::import_cell_style* import_styles::start_cell_style()
{
    mp_impl->cell_style.reset();
    return &mp_impl->cell_style;
}

void import_styles::set_font_count(size_t n)
{
    mp_impl->styles_model.reserve_font_store(n);
}

void import_styles::set_fill_count(size_t n)
{
    mp_impl->styles_model.reserve_fill_store(n);
}

void import_styles::set_border_count(size_t n)
{
    mp_impl->styles_model.reserve_border_store(n);
}

void import_styles::set_number_format_count(size_t n)
{
    mp_impl->styles_model.reserve_number_format_store(n);
}

void import_styles::set_xf_count(xf_category_t cat, size_t n)
{
    switch (cat)
    {
        case xf_category_t::cell:
            mp_impl->styles_model.reserve_cell_format_store(n);
            break;
        case xf_category_t::cell_style:
            mp_impl->styles_model.reserve_cell_style_format_store(n);
            break;
        case xf_category_t::differential:
            mp_impl->styles_model.reserve_diff_cell_format_store(n);
            break;
        case xf_category_t::unknown:
            break;
    }
}

void import_styles::set_cell_style_count(size_t n)
{
    mp_impl->styles_model.reserve_cell_style_store(n);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
