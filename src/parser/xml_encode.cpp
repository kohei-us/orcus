/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/xml_encode.hpp"

namespace orcus {

void write_content_encoded(std::ostream& os, std::string_view val, xml_encode_context_t cxt)
{
    const char* p0 = nullptr;

    auto flush = [&os, &p0](const char* p)
    {
        if (p0)
        {
            os.write(p0, p - p0);
            p0 = nullptr;
        }
    };

    for (const char* p = val.data(), *end = p + val.size(); p != end; ++p)
    {
        if (!p0)
            p0 = p;

        switch (*p)
        {
            case '&':
                flush(p);
                os << "&amp;";
                break;
            case '<':
                flush(p);
                os << "&lt;";
                break;
            case '>':
                flush(p);
                os << "&gt;";
                break;
            case '"':
                if (cxt == xml_encode_context_t::attr_double_quoted)
                {
                    flush(p);
                    os << "&quot;";
                }
                break;
            case '\'':
                if (cxt == xml_encode_context_t::attr_single_quoted)
                {
                    flush(p);
                    os << "&apos;";
                }
                break;
        }
    }

    flush(val.data() + val.size());
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
