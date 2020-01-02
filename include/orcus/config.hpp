/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_CONFIG_HPP
#define INCLUDED_ORCUS_CONFIG_HPP

#include "orcus/env.hpp"
#include "orcus/types.hpp"

#include <string>

namespace orcus {

struct ORCUS_DLLPUBLIC config
{
    format_t input_format;

    /**
     * configuration settings specific to the CSV format. This struct must be
     * POD.
     */
    struct csv_config
    {
        /** Number of header rows to repeat in case of split. */
        size_t header_row_size;

        /**
         * Whether or not to split oversized source data into multiple sheets
         * in case it spills over.
         */
        bool split_to_multiple_sheets;
    };

    /**
     * Enable or disable runtime debug output to stdout or stderr.
     */
    uint16_t debug;

    /**
     * Enable or disable re-calculation of formula cells on document load.
     */
    bool recalc_formula_cells;

    /**
     * Control whether or not to perform strict check of the xml structure of
     * a stream being parsed.  When enabled, it throws an xml_structure_error
     * exception when an incorrect xml structure is detected.
     */
    bool structure_check;

    union
    {
        csv_config csv;

        // TODO : add config for other formats as needed.
    };

    config(format_t input_format);
};

struct ORCUS_DLLPUBLIC json_config
{
    /**
     * Path of the JSON file being parsed, in case the JSON string originates
     * from a file.  This parameter is required if external JSON files need to
     * be resolved.  Otherwise it's optional.
     */
    std::string input_path;

    /**
     * Path of the file to which output is written to.  Used only from the
     * orcus-json command line tool.
     */
    std::string output_path;

    /**
     * Output format type.  Used only from the orcus-json command line tool.
     */
    dump_format_t output_format;

    /**
     * Control whether or not to preserve the order of object's child
     * name/value pairs.  By definition, JSON's object is an unordered set of
     * name/value pairs, but in some cases preserving the original order may
     * be desirable.
     */
    bool preserve_object_order;

    /**
     * Control whether or not to resolve JSON references to external files.
     */
    bool resolve_references;

    /**
     * When true, the document tree should allocate memory and hold copies of
     * string values in the tree.  When false, no extra memory is allocated
     * for string values in the tree and the string values simply point to the
     * original json string stream.
     *
     * In other words, when this option is set to false, the caller must
     * ensure that the json string stream instance stays alive for the entire
     * life cycle of the document tree.
     */
    bool persistent_string_values;

    json_config();
    ~json_config();
};

struct ORCUS_DLLPUBLIC yaml_config
{
    enum class output_format_type { none, yaml, json };

    std::string input_path;
    std::string output_path;

    output_format_type output_format;

    yaml_config();
    ~yaml_config();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
