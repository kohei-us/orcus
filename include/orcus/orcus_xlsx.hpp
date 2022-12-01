/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_ORCUS_XLSX_HPP
#define INCLUDED_ORCUS_ORCUS_XLSX_HPP

#include "interface.hpp"

#include <memory>

namespace orcus {

namespace spreadsheet { namespace iface { class import_factory; }}

struct xlsx_rel_sheet_info;
struct xlsx_rel_table_info;
struct xlsx_rel_pivot_cache_info;
struct xlsx_rel_pivot_cache_record_info;
struct orcus_xlsx_impl;
class xlsx_opc_handler;

class ORCUS_DLLPUBLIC orcus_xlsx : public iface::import_filter
{
    friend class xlsx_opc_handler;
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    orcus_xlsx(spreadsheet::iface::import_factory* factory);
    ~orcus_xlsx();

    orcus_xlsx(const orcus_xlsx&) = delete;
    orcus_xlsx& operator= (const orcus_xlsx&) = delete;

    static bool detect(const unsigned char* blob, size_t size);

    virtual void read_file(std::string_view filepath) override;
    virtual void read_stream(std::string_view stream) override;

    virtual std::string_view get_name() const override;

private:

    void set_formulas_to_doc();

    void read_workbook(const std::string& dir_path, const std::string& file_name);

    /**
     * Parse a sheet xml part that contains data stored in a single sheet.
     */
    void read_sheet(const std::string& dir_path, const std::string& file_name, xlsx_rel_sheet_info* data);

    /**
     * Parse sharedStrings.xml part that contains a list of strings referenced
     * in the document.
     */
    void read_shared_strings(const std::string& dir_path, const std::string& file_name);

    void read_styles(const std::string& dir_path, const std::string& file_name);

    void read_table(const std::string& dir_path, const std::string& file_name, xlsx_rel_table_info* data);

    void read_pivot_cache_def(
        const std::string& dir_path, const std::string& file_name,
        const xlsx_rel_pivot_cache_info* data);

    void read_pivot_cache_rec(
        const std::string& dir_path, const std::string& file_name,
        const xlsx_rel_pivot_cache_record_info* data);

    void read_pivot_table(const std::string& dir_path, const std::string& file_name);

    void read_rev_headers(const std::string& dir_path, const std::string& file_name);

    void read_rev_log(const std::string& dir_path, const std::string& file_name);

    void read_drawing(const std::string& dir_path, const std::string& file_name);
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
