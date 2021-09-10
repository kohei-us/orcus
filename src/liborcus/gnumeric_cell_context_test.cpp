/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gnumeric_cell_context.hpp"
#include "gnumeric_tokens.hpp"
#include "gnumeric_namespace_types.hpp"
#include "gnumeric_token_constants.hpp"
#include "mock_spreadsheet.hpp"
#include "session_context.hpp"
#include "orcus/types.hpp"

#include <iostream>
#include <string>
#include <cstdlib>

using namespace orcus;
using namespace std;
using namespace orcus::spreadsheet;
using namespace orcus::spreadsheet::mock;

namespace {

class mock_array_formula : public import_array_formula
{
public:
    virtual void set_range(const range_t& range) override
    {
        assert(range.first.row == 19);
        assert(range.first.column == 111);
        assert(range.last.row == 20);
        assert(range.last.column == 113);
    }

    virtual void set_formula(formula_grammar_t grammar, const char* p, size_t n) override
    {
        assert(grammar == formula_grammar_t::gnumeric);
        assert(string(p, n) == "=arrayFormula");
    }

    virtual void set_result_bool(row_t row, col_t col, bool value) override
    {
    }

    virtual void set_result_empty(row_t row, col_t col) override
    {
    }

    virtual void set_result_string(row_t row, col_t col, const char* p, size_t n) override
    {
    }

    virtual void set_result_value(row_t row, col_t col, double value) override
    {
    }

    virtual void commit() override
    {
    }
};

class mock_formula : public import_formula
{
public:
    virtual void set_position(row_t row, col_t col) override
    {
        assert(row == 9);
        assert(col == 11);
    }

    virtual void set_formula(formula_grammar_t grammar, const char* p, size_t n) override
    {
        assert(grammar == formula_grammar_t::gnumeric);
        assert(string(p, n) == "=formula");
    }

    virtual void set_shared_formula_index(size_t index) override
    {
    }

    virtual void set_result_bool(bool value) override
    {
    }

    virtual void set_result_empty() override
    {
    }

    virtual void set_result_string(const char* p, size_t n) override
    {
    }

    virtual void set_result_value(double value) override
    {
    }

    virtual void commit() override
    {
    }
};

class mock_sheet : public import_sheet
{
    mock_formula m_formula;
    mock_array_formula m_array_formula;
public:
    virtual void set_value(row_t row, col_t col, double val)
    {
        assert(row == 1);
        assert(col == 2);
        assert(val == 5.0);
    }

    virtual void set_bool(row_t row, col_t col, bool val)
    {
        assert(row == 31);
        assert(col == 32);
        assert(val == true);
    }

    virtual void set_string(row_t row, col_t col, string_id_t id)
    {
        assert(row == 10);
        assert(col == 321);
        assert(id == 2);
    }

    virtual iface::import_array_formula* get_array_formula()
    {
        return &m_array_formula;
    }

    virtual iface::import_formula* get_formula()
    {
        return &m_formula;
    }
};

class mock_shared_strings : public import_shared_strings
{
public:
    virtual size_t add(const char* s, size_t n)
    {
        assert(n == 14);
        assert(string(s, n) == "14 char string");
        return 2;
    }

};

class mock_factory : public import_factory
{
public:
    virtual iface::import_shared_strings* get_shared_strings()
    {
        return &m_shared_strings;
    }

private:
    mock_shared_strings m_shared_strings;
};

void test_cell_value()
{
    mock_sheet sheet;
    import_factory factory;
    session_context cxt;

    orcus::gnumeric_cell_context context(cxt, orcus::gnumeric_tokens, &factory, &sheet);

    orcus::xmlns_id_t ns = NS_gnumeric_gnm;
    orcus::xml_token_t elem = XML_Cell;
    orcus::xml_attrs_t attrs;
    attrs.push_back(xml_token_attr_t(NS_gnumeric_gnm, XML_Row, "1", false));
    attrs.push_back(xml_token_attr_t(NS_gnumeric_gnm, XML_Col, "2", false));
    attrs.push_back(xml_token_attr_t(NS_gnumeric_gnm, XML_ValueType, "40", false));
    context.start_element(ns, elem, attrs);
    context.characters("5", false);
    context.end_element(ns, elem);
}

void test_cell_bool()
{
    mock_sheet sheet;
    import_factory factory;
    session_context cxt;

    orcus::gnumeric_cell_context context(cxt, orcus::gnumeric_tokens, &factory, &sheet);

    orcus::xmlns_id_t ns = NS_gnumeric_gnm;
    orcus::xml_token_t elem = XML_Cell;
    orcus::xml_attrs_t attrs;
    attrs.push_back(xml_token_attr_t(NS_gnumeric_gnm, XML_Row, "31", false));
    attrs.push_back(xml_token_attr_t(NS_gnumeric_gnm, XML_Col, "32", false));
    attrs.push_back(xml_token_attr_t(NS_gnumeric_gnm, XML_ValueType, "20", false));
    context.start_element(ns, elem, attrs);
    context.characters("TRUE", false);
    context.end_element(ns, elem);
}

void test_cell_string()
{
    mock_sheet sheet;
    mock_factory factory;
    session_context cxt;

    orcus::gnumeric_cell_context context(cxt, orcus::gnumeric_tokens, &factory, &sheet);

    orcus::xmlns_id_t ns = NS_gnumeric_gnm;
    orcus::xml_token_t elem = XML_Cell;
    orcus::xml_attrs_t attrs;
    attrs.push_back(xml_token_attr_t(NS_gnumeric_gnm, XML_Row, "10", false));
    attrs.push_back(xml_token_attr_t(NS_gnumeric_gnm, XML_Col, "321", false));
    attrs.push_back(xml_token_attr_t(NS_gnumeric_gnm, XML_ValueType, "60", false));
    context.start_element(ns, elem, attrs);
    context.characters("14 char string", false);
    context.end_element(ns, elem);
}

void test_shared_formula_with_string()
{
    class mock_formula_local : public import_formula
    {
    public:
        void set_position(row_t row, col_t col) override
        {
            assert(row == 5);
            assert(col == 15);
        }

        void set_formula(formula_grammar_t grammar, const char* p, size_t n) override
        {
            assert(grammar == formula_grammar_t::gnumeric);
            assert(string(p, n) == "=basicFormulaString");
        }

        void set_shared_formula_index(size_t index) override
        {
            assert(index == 2);
        }

        void commit() override
        {
        }
    };

    class mock_sheet_local : public import_sheet
    {
        mock_formula_local m_formula;
    public:
        virtual iface::import_formula* get_formula()
        {
            return &m_formula;
        }
    };

    mock_sheet_local sheet;
    mock_factory factory;
    session_context cxt;

    orcus::gnumeric_cell_context context(cxt, orcus::gnumeric_tokens, &factory, &sheet);

    orcus::xmlns_id_t ns = NS_gnumeric_gnm;
    orcus::xml_token_t elem = XML_Cell;
    orcus::xml_attrs_t attrs;

    attrs.push_back(xml_token_attr_t(NS_gnumeric_gnm, XML_Row, "5", false));
    attrs.push_back(xml_token_attr_t(NS_gnumeric_gnm, XML_Col, "15", false));
    attrs.push_back(xml_token_attr_t(NS_gnumeric_gnm, XML_ExprID, "2", false));

    context.start_element(ns, elem, attrs);
    context.characters("=basicFormulaString", false);
    context.end_element(ns, elem);
}

void test_shared_formula_without_string()
{
    class mock_formula_local : public import_formula
    {
    public:
        void set_position(row_t row, col_t col) override
        {
            assert(row == 6);
            assert(col == 16);
        }

        void set_shared_formula_index(size_t index) override
        {
            assert(index == 3);
        }

        void commit() override
        {
        }
    };

    class mock_sheet_local : public import_sheet
    {
        mock_formula_local m_formula;
    public:
        virtual iface::import_formula* get_formula()
        {
            return &m_formula;
        }
    };

    mock_sheet_local sheet;
    mock_factory factory;
    session_context cxt;

    orcus::gnumeric_cell_context context(cxt, orcus::gnumeric_tokens, &factory, &sheet);

    orcus::xmlns_id_t ns = NS_gnumeric_gnm;
    orcus::xml_token_t elem = XML_Cell;
    orcus::xml_attrs_t attrs;

    attrs.push_back(xml_token_attr_t(NS_gnumeric_gnm, XML_Row, "6", false));
    attrs.push_back(xml_token_attr_t(NS_gnumeric_gnm, XML_Col, "16", false));
    attrs.push_back(xml_token_attr_t(NS_gnumeric_gnm, XML_ExprID, "3", false));

    context.start_element(ns, elem, attrs);
    context.end_element(ns, elem);
}

void test_cell_formula()
{
    mock_sheet sheet;
    mock_factory factory;
    session_context cxt;

    orcus::gnumeric_cell_context context(cxt, orcus::gnumeric_tokens, &factory, &sheet);

    orcus::xmlns_id_t ns = NS_gnumeric_gnm;
    orcus::xml_token_t elem = XML_Cell;
    orcus::xml_attrs_t attrs;
    attrs.push_back(xml_token_attr_t(NS_gnumeric_gnm, XML_Row, "9", false));
    attrs.push_back(xml_token_attr_t(NS_gnumeric_gnm, XML_Col, "11", false));
    context.start_element(ns, elem, attrs);
    context.characters("=formula", false);
    context.end_element(ns, elem);
}

void test_cell_array_formula()
{
    mock_sheet sheet;
    mock_factory factory;
    session_context cxt;

    orcus::gnumeric_cell_context context(cxt, orcus::gnumeric_tokens, &factory, &sheet);

    orcus::xmlns_id_t ns = NS_gnumeric_gnm;
    orcus::xml_token_t elem = XML_Cell;
    orcus::xml_attrs_t attrs;
    attrs.push_back(xml_token_attr_t(NS_gnumeric_gnm, XML_Row, "19", false));
    attrs.push_back(xml_token_attr_t(NS_gnumeric_gnm, XML_Col, "111", false));
    attrs.push_back(xml_token_attr_t(NS_gnumeric_gnm, XML_Rows, "2", false));
    attrs.push_back(xml_token_attr_t(NS_gnumeric_gnm, XML_Cols, "3", false));
    context.start_element(ns, elem, attrs);
    context.characters("=arrayFormula", false);
    context.end_element(ns, elem);
}

}

int main()
{
    test_cell_value();
    test_cell_bool();
    test_cell_string();
    test_shared_formula_with_string();
    test_shared_formula_without_string();
    test_cell_formula();
    test_cell_array_formula();

    return EXIT_SUCCESS;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
