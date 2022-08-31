/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ODF_NUMBER_FORMATTING_CONTEXT_HPP
#define ODF_NUMBER_FORMATTING_CONTEXT_HPP

#include "xml_context_base.hpp"
#include "odf_styles.hpp"

#include "orcus/string_pool.hpp"

namespace orcus {

namespace spreadsheet { namespace iface {
    class import_styles;
}}

class date_style_context : public xml_context_base
{
public:
    date_style_context(session_context& session_cxt, const tokens& tk);

    void start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs) override;
    bool end_element(xmlns_id_t ns, xml_token_t name) override;
    void characters(std::string_view str, bool transient) override;

    void reset();

    std::unique_ptr<odf_number_format> pop_style();

private:
    void start_element_date_style(const std::vector<xml_token_attr_t>& attrs);
    void start_element_month(const std::vector<xml_token_attr_t>& attrs);
    void start_element_day(const std::vector<xml_token_attr_t>& attrs);
    void start_element_year(const std::vector<xml_token_attr_t>& attrs);

private:
    std::unique_ptr<odf_number_format> m_current_style;
    std::ostringstream m_text_stream;
};

class time_style_context : public xml_context_base
{
public:
    time_style_context(session_context& session_cxt, const tokens& tk);

    void start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs) override;
    bool end_element(xmlns_id_t ns, xml_token_t name) override;
    void characters(std::string_view str, bool transient) override;

    void reset();

    std::unique_ptr<odf_number_format> pop_style();

private:
    void start_element_time_style(const std::vector<xml_token_attr_t>& attrs);
    void start_element_seconds(const std::vector<xml_token_attr_t>& attrs);

private:
    std::unique_ptr<odf_number_format> m_current_style;
    std::ostringstream m_text_stream;
};

class percentage_style_context : public xml_context_base
{
public:
    percentage_style_context(session_context& session_cxt, const tokens& tk);

    void start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs) override;
    bool end_element(xmlns_id_t ns, xml_token_t name) override;
    void characters(std::string_view str, bool transient) override;

    void reset();

    std::unique_ptr<odf_number_format> pop_style();

private:
    std::unique_ptr<odf_number_format> m_current_style;
    std::ostringstream m_text_stream;
};

class boolean_style_context : public xml_context_base
{
public:
    boolean_style_context(session_context& session_cxt, const tokens& tk);

    void start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs) override;
    bool end_element(xmlns_id_t ns, xml_token_t name) override;

    void reset();

    std::unique_ptr<odf_number_format> pop_style();

private:
    std::unique_ptr<odf_number_format> m_current_style;
};

class text_style_context : public xml_context_base
{
public:
    text_style_context(session_context& session_cxt, const tokens& tk);

    void start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs) override;
    bool end_element(xmlns_id_t ns, xml_token_t name) override;

    void reset();

    std::unique_ptr<odf_number_format> pop_style();

private:
    std::unique_ptr<odf_number_format> m_current_style;
};

/**
 * Context for <number:number-style> scope.
 */
class number_style_context : public xml_context_base
{
public:
    number_style_context(session_context& session_cxt, const tokens& tk);

    void start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs) override;
    bool end_element(xmlns_id_t ns, xml_token_t name) override;
    void characters(std::string_view str, bool transient) override;

    void reset();

    std::unique_ptr<odf_number_format> pop_style();

private:
    void start_element_fraction(const std::vector<xml_token_attr_t>& attrs);
    void start_element_number_style(const std::vector<xml_token_attr_t>& attrs);
    void start_element_scientific_number(const std::vector<xml_token_attr_t>& attrs);

private:
    std::unique_ptr<odf_number_format> m_current_style;
    std::ostringstream m_text_stream;

    std::string_view m_country_code; // TODO: handle this
    std::string_view m_language; // TODO: handle this
};

/**
 * Context for <number:currency-style> element scope.
 */
class currency_style_context : public xml_context_base
{
public:
    currency_style_context(session_context& session_cxt, const tokens& tk);

    void start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs) override;
    bool end_element(xmlns_id_t ns, xml_token_t name) override;
    void characters(std::string_view str, bool transient) override;

    void reset();

    std::unique_ptr<odf_number_format> pop_style();

private:
    void start_element_currency_style(const std::vector<xml_token_attr_t>& attrs);

private:
    std::unique_ptr<odf_number_format> m_current_style;
    std::ostringstream m_text_stream;

    std::string_view m_country_code; // TODO: handle this
    std::string_view m_language; // TODO: handle this
};

} // namespace orcus

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
