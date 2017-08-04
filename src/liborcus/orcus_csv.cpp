/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_csv.hpp"

#include "orcus/csv_parser.hpp"
#include "orcus/pstring.hpp"
#include "orcus/global.hpp"
#include "orcus/stream.hpp"
#include "orcus/spreadsheet/import_interface.hpp"

#include <cstring>
#include <iostream>

using namespace std;

namespace orcus {

namespace {

constexpr const char* base_sheet_name = "data";

class csv_handler
{
public:
    csv_handler(spreadsheet::iface::import_factory& factory) :
        m_factory(factory), mp_sheet(nullptr), m_sheet(0), m_row(0), m_col(0) {}

    void begin_parse()
    {
        std::string sheet_name = get_sheet_name();
        mp_sheet = m_factory.append_sheet(m_sheet++, sheet_name.data(), sheet_name.size());
    }

    void end_parse() {}
    void begin_row() {}

    void end_row()
    {
        ++m_row;
        m_col = 0;

        // Check to see if the next row is beyond the max row of the current
        // sheet, and if so, append a new sheet and reset the current row to
        // 0.

        if (m_row >= mp_sheet->get_sheet_size().rows)
        {
            // The next row will be outside the boundary of the current sheet.
            std::string sheet_name = get_sheet_name();
            mp_sheet = m_factory.append_sheet(m_sheet++, sheet_name.data(), sheet_name.size());
            m_row = 0;
        }
    }

    void cell(const char* p, size_t n)
    {
        mp_sheet->set_auto(m_row, m_col, p, n);
        ++m_col;
    }

private:
    std::string get_sheet_name() const
    {
        if (!m_sheet)
            // First sheet has no suffix.
            return base_sheet_name;

        // Add a suffix to keep the sheet name unique.
        std::ostringstream os;
        os << base_sheet_name << '_' << m_sheet;
        return os.str();
    }

private:
    spreadsheet::iface::import_factory& m_factory;
    spreadsheet::iface::import_sheet* mp_sheet;
    spreadsheet::sheet_t m_sheet;
    spreadsheet::row_t m_row;
    spreadsheet::col_t m_col;
};

}

orcus_csv::orcus_csv(spreadsheet::iface::import_factory* factory) :
    iface::import_filter(format_t::csv), mp_factory(factory) {}

void orcus_csv::read_file(const string& filepath)
{
    string strm = load_file_content(filepath.c_str());
    parse(&strm[0], strm.size());

    mp_factory->finalize();
}

void orcus_csv::read_stream(const char* content, size_t len)
{
    if (!content)
        return;

    parse(content, len);

    mp_factory->finalize();
}

const char* orcus_csv::get_name() const
{
    static const char* name = "csv";
    return name;
}

void orcus_csv::parse(const char* content, size_t len)
{
    if (!len)
        return;

    csv_handler handler(*mp_factory);
    csv::parser_config config;
    config.delimiters.push_back(',');
    config.text_qualifier = '"';
    csv_parser<csv_handler> parser(content, len, handler, config);
    try
    {
        parser.parse();
    }
    catch (const csv::parse_error& e)
    {
        cout << "parse failed: " << e.what() << endl;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
