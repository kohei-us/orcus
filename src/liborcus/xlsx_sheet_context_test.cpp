/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "mock_spreadsheet.hpp"
#include "ooxml_namespace_types.hpp"
#include "ooxml_tokens.hpp"
#include "ooxml_schemas.hpp"
#include "xlsx_sheet_context.hpp"
#include "ooxml_token_constants.hpp"
#include "xlsx_session_data.hpp"
#include "orcus/types.hpp"
#include "orcus/config.hpp"

using namespace orcus;
using namespace std;
using namespace orcus::spreadsheet;
using namespace orcus::spreadsheet::mock;

namespace {

class mock_ref_resolver : public import_reference_resolver
{
    virtual src_address_t resolve_address(std::string_view) override
    {
        src_address_t ret;
        ret.sheet = 0;
        ret.row = 0;
        ret.column = 0;

        return ret;
    }

    virtual src_range_t resolve_range(std::string_view) override
    {
        src_range_t ret;
        ret.first.sheet = 0;
        ret.first.row = 0;
        ret.first.column = 0;
        ret.last.sheet = 0;
        ret.last.row = 0;
        ret.last.column = 0;

        return ret;
    }
};

class mock_array_formula : public import_array_formula
{
public:
    virtual void set_range(const range_t& range) override
    {
        assert(range.first.row == 2);
        assert(range.first.column == 1);
        assert(range.last.row == 3);
        assert(range.last.column == 1);
    }

    virtual void set_formula(formula_grammar_t grammar, std::string_view formula) override
    {
        assert(grammar == formula_grammar_t::xlsx);
        assert(formula == "A1:A2");
    }

    virtual void set_result_bool(row_t row, col_t col, bool value) override
    {
    }

    virtual void set_result_empty(row_t row, col_t col) override
    {
    }

    virtual void set_result_string(row_t row, col_t col, std::string_view) override
    {
    }

    virtual void set_result_value(row_t row, col_t col, double value) override
    {
    }

    virtual void commit() override
    {
    }
};

class mock_sheet : public import_sheet
{
    mock_array_formula m_array_formula;

public:
    virtual void set_value(row_t row, col_t col, double val) override
    {
        assert(row == -1);
        assert(col == 0);
        assert(val == 5.0);
    }

    virtual void set_bool(row_t row, col_t col, bool val) override
    {
        assert(row == -1);
        assert(col == 0);
        assert(val == true);
    }

    virtual iface::import_array_formula* get_array_formula() override
    {
        return &m_array_formula;
    }
};

class mock_sheet_properties : public import_sheet_properties
{
public:
    void set_column_hidden(col_t col, bool hidden)
    {
        assert(col == 1);
        assert(hidden);
    }

    void set_row_hidden(row_t row, bool hidden)
    {
        assert(row == 3);
        assert(hidden);
    }
};

class mock_sheet2 : public import_sheet
{
public:
    virtual import_sheet_properties* get_sheet_properties()
    {
        return &m_sheet_prop;
    }

private:
    mock_sheet_properties m_sheet_prop;
};

void test_cell_value()
{
    mock_sheet sheet;
    mock_ref_resolver resolver;
    session_context cxt(new xlsx_session_data);
    config opt(format_t::xlsx);
    opt.structure_check = false;

    orcus::xlsx_sheet_context context(cxt, orcus::ooxml_tokens, 0, resolver, sheet);
    context.set_config(opt);

    orcus::xmlns_id_t ns = NS_ooxml_xlsx;
    orcus::xml_token_t elem = XML_c;
    orcus::xml_attrs_t attrs;
    context.start_element(ns, elem, attrs);

    {
        xml_attrs_t val_attrs;
        context.start_element(ns, XML_v, val_attrs);
        context.characters("5", false);
        context.end_element(ns, XML_v);
    }

    context.end_element(ns, elem);
}

void test_cell_bool()
{
    mock_sheet sheet;
    mock_ref_resolver resolver;
    session_context cxt(new xlsx_session_data);
    config opt(format_t::xlsx);
    opt.structure_check = false;

    orcus::xlsx_sheet_context context(cxt, orcus::ooxml_tokens, 0, resolver, sheet);
    context.set_config(opt);

    orcus::xmlns_id_t ns = NS_ooxml_xlsx;
    orcus::xml_token_t elem = XML_c;
    orcus::xml_attrs_t attrs;
    attrs.push_back(xml_token_attr_t(NS_ooxml_xlsx, XML_t, "b", false));
    context.start_element(ns, elem, attrs);

    {
        xml_attrs_t val_attrs;
        context.start_element(ns, XML_v, val_attrs);
        context.characters("1", false);
        context.end_element(ns, XML_v);
    }

    context.end_element(ns, elem);
}

void test_array_formula()
{
    mock_sheet sheet;
    mock_ref_resolver resolver;
    session_context cxt(new xlsx_session_data);
    config opt(format_t::xlsx);
    opt.structure_check = false;

    orcus::xlsx_sheet_context context(cxt, orcus::ooxml_tokens, 0, resolver, sheet);
    context.set_config(opt);

    orcus::xmlns_id_t ns = NS_ooxml_xlsx;
    orcus::xml_token_t elem = XML_c;
    orcus::xml_attrs_t attrs;
    context.start_element(ns, elem, attrs);

    {
        xml_attrs_t formula_attrs;
        formula_attrs.push_back(xml_token_attr_t(NS_ooxml_xlsx, XML_t, "array", false));
        formula_attrs.push_back(xml_token_attr_t(NS_ooxml_xlsx, XML_ref, "B3:B4", false));
        context.start_element(ns, XML_f, formula_attrs);
        context.characters("A1:A2", false);
        context.end_element(ns, XML_f);
    }
    {
        xml_attrs_t val_attrs;
        context.start_element(ns, XML_v, val_attrs);
        context.characters("5", false);
        context.end_element(ns, XML_v);
    }

    context.end_element(ns, elem);
}

void test_hidden_col()
{
    mock_sheet2 sheet;
    mock_ref_resolver resolver;
    session_context cxt(new xlsx_session_data);
    config opt(format_t::xlsx);
    opt.structure_check = false;

    orcus::xlsx_sheet_context context(cxt, orcus::ooxml_tokens, 0, resolver, sheet);
    context.set_config(opt);

    orcus::xmlns_id_t ns = NS_ooxml_xlsx;
    orcus::xml_token_t elem = XML_col;
    orcus::xml_attrs_t attrs;
    attrs.push_back(orcus::xml_token_attr_t(ns, XML_min, "2", false));
    attrs.push_back(orcus::xml_token_attr_t(ns, XML_max, "2", false));
    attrs.push_back(orcus::xml_token_attr_t(ns, XML_hidden, "1", false));
    context.start_element(ns, elem, attrs);
    context.end_element(ns, elem);
}

void test_hidden_row()
{
    mock_sheet2 sheet;
    mock_ref_resolver resolver;
    session_context cxt(new xlsx_session_data);
    config opt(format_t::xlsx);
    opt.structure_check = false;

    orcus::xlsx_sheet_context context(cxt, orcus::ooxml_tokens, 0, resolver, sheet);
    context.set_config(opt);

    orcus::xmlns_id_t ns = NS_ooxml_xlsx;
    orcus::xml_token_t elem = XML_row;
    orcus::xml_attrs_t attrs;
    attrs.push_back(orcus::xml_token_attr_t(ns, XML_r, "4", false));
    attrs.push_back(orcus::xml_token_attr_t(ns, XML_hidden, "1", false));
    context.start_element(ns, elem, attrs);
    context.end_element(ns, elem);
}

}

int main()
{
    test_cell_value();
    test_cell_bool();
    test_array_formula();
    test_hidden_col();
    test_hidden_row();
    return 0;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
