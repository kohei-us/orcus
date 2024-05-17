/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ooxml_global.hpp"
#include "ooxml_types.hpp"
#include "ooxml_token_constants.hpp"
#include "ooxml_namespace_types.hpp"
#include "xml_context_base.hpp"

#include <iostream>
#include <sstream>

namespace orcus {

void print_opc_rel::operator() (const opc_rel_t& v) const
{
    std::cout << v.rid << ": " << v.target << " (" << v.type << ")" << std::endl;
}

std::string resolve_file_path(std::string_view dir_path, std::string_view file_name)
{
    if (dir_path.empty())
        return std::string{file_name};

    const char* p = dir_path.data();
    const char* p_end = p + dir_path.size();

    bool has_root = *p == '/';
    if (has_root)
        ++p;

    std::vector<std::string_view> dir_stack;
    const char* p_head = nullptr;
    for (; p != p_end; ++p)
    {
        if (*p == '/')
        {
            if (!p_head)
                // invalid directory path.
                return std::string{file_name};

            size_t len = p - p_head;
            std::string_view dir(p_head, len);
            if (dir == "..")
            {
                if (dir_stack.empty())
                    // invalid directory path.
                    return std::string{file_name};

                dir_stack.pop_back();
            }
            else
                dir_stack.push_back(dir);

            p_head = nullptr;
        }
        else if (p_head)
        {
            // Do nothing.
        }
        else
            p_head = p;
    }

    if (p_head)
    {
        // directory path must end with '/'.  This one doesn't.
        return std::string{file_name};
    }

    std::ostringstream full_path;
    if (has_root)
        full_path << '/';

    for (auto dir : dir_stack)
        full_path << dir << '/';

    full_path << file_name;

    return full_path.str();
}

void init_ooxml_context(xml_context_base& cxt)
{
    cxt.set_always_allowed_elements({
        { NS_mc, XML_Choice },
        { NS_mc, XML_Fallback },
    });
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
